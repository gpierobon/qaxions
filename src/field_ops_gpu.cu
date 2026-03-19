#ifdef USE_GPU

#include "field.h"
#include "profiler.h"
#include "gpu/cuda_utils.h"

// ----------------------------------------------------------------------------
//   Kick - GPU
// ----------------------------------------------------------------------------
__global__ void kick_kernel(cufftDoubleComplex* psi,
                            const double*       V,
                            size_t              sites,
                            double              fac)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= sites) return;

    double phase  = fac * V[idx];
    double re     = psi[idx].x;
    double im     = psi[idx].y;
    double cos_p, sin_p;
    sincos(phase, &sin_p, &cos_p);   // fused sincos

    psi[idx].x = re * cos_p - im * sin_p;
    psi[idx].y = re * sin_p + im * cos_p;
}


void Field::kick_gpu(double dt)
{
    const double fac = -dt;
    int grid = (sites_ + CUDA_BLOCK - 1) / CUDA_BLOCK;
    kick_kernel<<<grid, CUDA_BLOCK>>>(d_psi_, d_V_, sites_, fac);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

// ----------------------------------------------------------------------------
//   Drift_k2 - GPU
// ----------------------------------------------------------------------------
__global__ void drift_kernel(cufftDoubleComplex* psi,
                             size_t              sites,
                             double              fac,
                             double              dk,
                             int                 N,
                             int                 dim)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= sites) return;

    int hN = N / 2;
    double kx = 0.0, ky = 0.0, kz = 0.0;

    if (dim == 3)
    {
        int iz = idx % N;
        int iy = (idx / N) % N;
        int ix = idx / (N * N);

        kx = ((ix <= hN) ? ix : ix - N) * dk;
        ky = ((iy <= hN) ? iy : iy - N) * dk;
        kz = ((iz <= hN) ? iz : iz - N) * dk;
    }
    else
    {
        int iy = idx / N;
        int ix = idx % N;

        kx = ((ix <= hN) ? ix : ix - N) * dk;
        ky = ((iy <= hN) ? iy : iy - N) * dk;
    }

    double k2    = kx*kx + ky*ky + kz*kz;
    double phase = fac * k2;
    double re    = psi[idx].x;
    double im    = psi[idx].y;
    double cos_p, sin_p;
    sincos(phase, &sin_p, &cos_p);

    psi[idx].x = (re * cos_p - im * sin_p) / sites;
    psi[idx].y = (re * sin_p + im * cos_p) / sites;
}



void Field::drift_k2_gpu(double dt)
{
    const double fac = -0.5 * dt;
    const double dk  = 2.0 * M_PI / Lbox_;

    int grid = (sites_ + CUDA_BLOCK - 1) / CUDA_BLOCK;
    drift_kernel<<<grid, CUDA_BLOCK>>>(d_psi_, sites_, fac, dk, N_, dim_);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}


// ----------------------------------------------------------------------------
//   Density contrast 
// ----------------------------------------------------------------------------
__global__ void density_kernel(const cufftDoubleComplex* psi,
                               double*                   V,
                               size_t                    sites,
                               double                    pref,
                               double                    rho_mean)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= sites) return;

    double re  = psi[idx].x;
    double im  = psi[idx].y;
    V[idx]     = pref * (re*re + im*im - rho_mean);
}

// ----------------------------------------------------------------------------
//   Poisson solve in k-space
// ----------------------------------------------------------------------------
__global__ void poisson_kernel(cufftDoubleComplex* Vhat,
                               size_t              ksites,
                               double              twopi,
                               double              vol,
                               int                 N,
                               int                 dim)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= ksites) return;

    int hN  = N / 2;
    int hN1 = N / 2 + 1;

    double kx = 0.0, ky = 0.0, kz = 0.0;

    if (dim == 3)
    {
        int ix = idx / (N * hN1);
        int iy = (idx / hN1) % N;
        int iz = idx % hN1;          // ix runs 0..hN only (r2c half-array)

        kx = ((ix <= hN) ? ix : ix - N) * twopi;
        ky = ((iy <= hN) ? iy : iy - N) * twopi;
        kz = iz * twopi;             // iz is always 0..hN1-1, no folding needed
    }
    else
    {
        int iy = idx / hN1;
        int ix = idx % hN1;          // ix runs 0..hN only

        kx = ix * twopi;
        ky = ((iy <= hN) ? iy : iy - N) * twopi;
    }

    double k2  = kx*kx + ky*ky + kz*kz;
    k2         = k2 + (k2 == 0.0);  // avoid division by zero at DC mode
    double fac = -1.0 / (k2 * vol);

    Vhat[idx].x *= fac;
    Vhat[idx].y *= fac;
}

// ----------------------------------------------------------------------------
//   Vmax reduction — two-pass: per-block max then host finalises
// ----------------------------------------------------------------------------
__global__ void vmax_kernel(const double* V,
                            double*       block_max,
                            size_t        sites)
{
    extern __shared__ double sdata[];

    size_t idx  = blockIdx.x * blockDim.x + threadIdx.x;
    size_t tid  = threadIdx.x;

    sdata[tid] = (idx < sites) ? fabs(V[idx]) : 0.0;
    __syncthreads();

    // Tree reduction within block
    for (size_t s = blockDim.x / 2; s > 0; s >>= 1)
    {
        if (tid < s)
            sdata[tid] = fmax(sdata[tid], sdata[tid + s]);
        __syncthreads();
    }

    if (tid == 0) block_max[blockIdx.x] = sdata[0];
}

// ----------------------------------------------------------------------------
//   updatePotential_gpu
// ----------------------------------------------------------------------------
void Field::updatePotential_gpu()
{
    PROFILE(POISSON);
    //{ toDevice(); }
    const double pref  = norm_ * a_;
    const double twopi = 2.0 * M_PI / Lbox_;
    const double vol   = (dim_ == 3) ? N_*N_*N_ : N_*N_;

    int grid_sites  = (sites_  + CUDA_BLOCK - 1) / CUDA_BLOCK;
    int grid_ksites = (ksites_ + CUDA_BLOCK - 1) / CUDA_BLOCK;

    // ── density contrast ─────────────────────────────────────────────────────
    density_kernel<<<grid_sites, CUDA_BLOCK>>>(d_psi_, d_V_,
                                               sites_, pref, rho_mean_);
    CUDA_CHECK(cudaGetLastError());

    fft_forward_r2c();

    poisson_kernel<<<grid_ksites, CUDA_BLOCK>>>(d_Vhat_, ksites_,
                                                twopi, vol, N_, dim_);
    CUDA_CHECK(cudaGetLastError());

    fft_backward_c2r();

    // ── Vmax reduction ────────────────────────────────────────────────────────
    int n_blocks = grid_sites;
    double* d_block_max = nullptr;
    CUDA_CHECK(cudaMalloc(&d_block_max, n_blocks * sizeof(double)));

    vmax_kernel<<<n_blocks, CUDA_BLOCK, CUDA_BLOCK * sizeof(double)>>>(
        d_V_, d_block_max, sites_);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    // Finalise on host — n_blocks is small enough that this is negligible
    std::vector<double> h_block_max(n_blocks);
    CUDA_CHECK(cudaMemcpy(h_block_max.data(), d_block_max,
                          n_blocks * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_block_max));

    Vmax_ = 0.0;
    for (int i = 0; i < n_blocks; ++i)
        if (h_block_max[i] > Vmax_) Vmax_ = h_block_max[i];
}

#endif

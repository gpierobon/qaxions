#ifdef USE_GPU

#include <stdexcept>
#include <cstdio>
#include <cuda_runtime.h>
#include <cufft.h>
#include <fftw3.h> 
#include "../gpu/cuda_utils.h"

#include "fft.h"

class CUFFTBackend : public FFTBackend
{
    int    dim_;
    size_t N_;
    size_t sites_;
    size_t ksites_;

    cufftHandle plan_fwd_c2c_ = 0;
    cufftHandle plan_bwd_c2c_ = 0;
    cufftHandle plan_fwd_r2c_ = 0;
    cufftHandle plan_bwd_c2r_ = 0;

public:
    CUFFTBackend(int dim, size_t N, bool verbose)
        : dim_(dim), N_(N)
    {
        if (dim_ != 2 && dim_ != 3)
            throw std::invalid_argument("Dimension must be 2 or 3");

        sites_  = (dim_ == 3) ? N_*N_*N_       : N_*N_;
        ksites_ = (dim_ == 3) ? N_*N_*(N_/2+1) : N_*(N_/2+1);

        int n = static_cast<int>(N_);

        if (dim_ == 3)
        {
            CUFFT_CHECK(cufftPlan3d(&plan_fwd_c2c_, n, n, n, CUFFT_Z2Z));
            CUFFT_CHECK(cufftPlan3d(&plan_fwd_r2c_, n, n, n, CUFFT_D2Z));
            CUFFT_CHECK(cufftPlan3d(&plan_bwd_c2r_, n, n, n, CUFFT_Z2D));
            CUFFT_CHECK(cufftPlan3d(&plan_bwd_c2c_, n, n, n, CUFFT_Z2Z));
        }
        else
        {
            CUFFT_CHECK(cufftPlan2d(&plan_fwd_c2c_, n, n, CUFFT_Z2Z));
            CUFFT_CHECK(cufftPlan2d(&plan_fwd_r2c_, n, n, CUFFT_D2Z));
            CUFFT_CHECK(cufftPlan2d(&plan_bwd_c2r_, n, n, CUFFT_Z2D));
            CUFFT_CHECK(cufftPlan2d(&plan_bwd_c2c_, n, n, CUFFT_Z2Z));
        }

        if (verbose)
            printf("[cuFFT] Plans created  dim=%d  N=%zu  sites=%zu  ksites=%zu\n",
                   dim_, N_, sites_, ksites_);
    }

    ~CUFFTBackend() override
    {
        cufftDestroy(plan_fwd_c2c_);
        cufftDestroy(plan_bwd_c2c_);
        cufftDestroy(plan_fwd_r2c_);
        cufftDestroy(plan_bwd_c2r_);
    }

    // ── interface implementation ─────────────────────────────────────────────
    // The base class uses fftw_complex*/double* in its signatures.
    // We cast to the binary-compatible cuFFT types inside each call.
    void forward_c2c(fftw_complex* data) override
    {
        CUFFT_CHECK(cufftExecZ2Z(plan_fwd_c2c_,
                                 reinterpret_cast<cufftDoubleComplex*>(data),
                                 reinterpret_cast<cufftDoubleComplex*>(data),
                                 CUFFT_FORWARD));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void backward_c2c(fftw_complex* data) override
    {
        CUFFT_CHECK(cufftExecZ2Z(plan_bwd_c2c_,
                                 reinterpret_cast<cufftDoubleComplex*>(data),
                                 reinterpret_cast<cufftDoubleComplex*>(data),
                                 CUFFT_INVERSE));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void forward_r2c(double* in_real, fftw_complex* out_complex) override
    {
        CUFFT_CHECK(cufftExecD2Z(plan_fwd_r2c_,
                                 reinterpret_cast<cufftDoubleReal*>(in_real),
                                 reinterpret_cast<cufftDoubleComplex*>(out_complex)));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void backward_c2r(fftw_complex* in_complex, double* out_real) override
    {
        CUFFT_CHECK(cufftExecZ2D(plan_bwd_c2r_,
                                 reinterpret_cast<cufftDoubleComplex*>(in_complex),
                                 reinterpret_cast<cufftDoubleReal*>(out_real)));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    size_t    sites()     const override { return sites_;  }
    size_t    ksites()    const override { return ksites_; }
    ptrdiff_t global_N()  const override { return static_cast<ptrdiff_t>(N_); }
};

std::unique_ptr<FFTBackend> makeCUFFTBackend(int dim, size_t N, bool verbose)
{
    return std::make_unique<CUFFTBackend>(dim, N, verbose);
}

#endif // USE_GPU

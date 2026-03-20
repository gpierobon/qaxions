#include "../field.h"
#include "../profiler.h"
#include "cuda_utils.h"


void Field::allocGPU()
{
    CUDA_CHECK(cudaMalloc(&d_psi_,  sites_  * sizeof(CuComplex)));
    CUDA_CHECK(cudaMalloc(&d_Vhat_, ksites_ * sizeof(CuComplex)));
    CUDA_CHECK(cudaMalloc(&d_V_,    sites_  * sizeof(CuReal)));
    if (verb_)
    {
        std::cout << "[GPU] Device buffers allocated  ("
                  << ( sites_  * sizeof(CuComplex) * 2
                     + ksites_ * sizeof(CuComplex)) / (1 << 20)
                  << " MB)\n";
    }
}

void Field::freeGPU()
{
    if (d_psi_)  { CUDA_CHECK(cudaFree(d_psi_));  d_psi_  = nullptr; }
    if (d_V_)    { CUDA_CHECK(cudaFree(d_V_));    d_V_    = nullptr; }
    if (d_Vhat_) { CUDA_CHECK(cudaFree(d_Vhat_)); d_Vhat_ = nullptr; }
}


void Field::toDevice()
{
    PROFILE(GPU_H2D);

    gpu_active_ = true;

    if (verb_)
        std::cout << "[GPU]: Host to device transfer ..." << std::endl;

    CUDA_CHECK(cudaMemcpy(d_psi_,  psi_, 
               sites_ * sizeof(CuComplex), 
               cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_Vhat_, Vhat_,
               ksites_ * sizeof(CuComplex), 
               cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_V_, V_, 
               sites_ * sizeof(CuReal), 
               cudaMemcpyHostToDevice));

    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

void Field::toHost()
{
    PROFILE(GPU_D2H);
    
    if (verb_)
        std::cout << "[GPU]: Device to host transfer ..." << std::endl;

    CUDA_CHECK(cudaMemcpy(psi_,  d_psi_,
               sites_ * sizeof(CuComplex), 
               cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(Vhat_,  d_Vhat_, 
               ksites_ * sizeof(CuComplex), 
               cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(V_, d_V_, 
               sites_ * sizeof(CuReal), 
               cudaMemcpyDeviceToHost));

    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

void Field::syncVhatToHost()
{
    if (verb_)
        std::cout << "[GPU]: Sync Vhat to host ..." << std::endl;

    CUDA_CHECK(cudaMemcpy(Vhat_, d_Vhat_,
                          ksites_ * sizeof(CuComplex),
                          cudaMemcpyDeviceToHost));

    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

void Field::syncVToDevice()
{
    if (verb_)
        std::cout << "[GPU]: Sync V to device ..." << std::endl;

    CUDA_CHECK(cudaMemcpy(d_V_, V_,
                          sites_ * sizeof(CuReal),
                          cudaMemcpyHostToDevice));

    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

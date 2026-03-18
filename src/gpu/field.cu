#include "../field.h"
#include "../profiler.h"


void Field::allocGPU()
{
    cudaMalloc(&d_psi_,  sites_ * sizeof(cufftDoubleComplex));
    cudaMalloc(&d_Vhat_, sites_ * sizeof(cufftDoubleComplex));
    cudaMalloc(&d_V_,  sites_ * sizeof(cufftDoubleReal));
    if (verb_)
    {
        std::cout << "[GPU] Device buffers allocated  ("
                  << ( sites_  * sizeof(cufftDoubleComplex) * 2
                     + ksites_ * sizeof(cufftDoubleComplex)) / (1 << 20)
                  << " MB)\n";
    }
}

void Field::freeGPU()
{
    if (d_psi_)  { cudaFree(d_psi_);  d_psi_  = nullptr; }
    if (d_V_)    { cudaFree(d_V_);    d_V_    = nullptr; }
    if (d_Vhat_) { cudaFree(d_Vhat_); d_Vhat_ = nullptr; }
}


void Field::toDevice()
{
    PROFILE(GPU_H2D);
    if (verb_)
        std::cout << "[GPU]: Host to device transfer ..." << std::endl;
    cudaMemcpy(d_psi_,  psi_,  sites_ * sizeof(cufftDoubleComplex), 
               cudaMemcpyHostToDevice);
    cudaMemcpy(d_V_, V_, sites_ * sizeof(cufftDoubleReal), 
               cudaMemcpyHostToDevice);
    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

void Field::toHost()
{
    PROFILE(GPU_D2H);
    if (verb_)
        std::cout << "[GPU]: Device to host transfer ..." << std::endl;
    cudaMemcpy(psi_,  d_psi_,  sites_ * sizeof(cufftDoubleComplex), 
               cudaMemcpyDeviceToHost);
    cudaMemcpy(V_, d_V_, sites_ * sizeof(cufftDoubleReal), 
               cudaMemcpyDeviceToHost);
    if (verb_)
        std::cout << "[GPU]: done!" << std::endl;
}

void Field::syncVhatToHost()
{
#ifdef USE_GPU
    cudaMemcpy(Vhat_, d_Vhat_, ksites_ * sizeof(cufftDoubleComplex),
               cudaMemcpyDeviceToHost);
#endif
}

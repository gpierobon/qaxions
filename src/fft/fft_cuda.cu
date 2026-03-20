#ifdef USE_GPU

#include <stdexcept>
#include <cstdio>
#include <cuda_runtime.h>
#include <cufft.h>
#include <fftw3.h> 
#include "../gpu/cuda_utils.h"
#include "../types.h"

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
            CUFFT_CHECK(cufftPlan3d(&plan_fwd_c2c_, n, n, n, CUFFT_C2C_TYPE));
            CUFFT_CHECK(cufftPlan3d(&plan_fwd_r2c_, n, n, n, CUFFT_R2C_TYPE));
            CUFFT_CHECK(cufftPlan3d(&plan_bwd_c2r_, n, n, n, CUFFT_C2R_TYPE));
            CUFFT_CHECK(cufftPlan3d(&plan_bwd_c2c_, n, n, n, CUFFT_C2C_TYPE));
        }
        else
        {
            CUFFT_CHECK(cufftPlan2d(&plan_fwd_c2c_, n, n, CUFFT_C2C_TYPE));
            CUFFT_CHECK(cufftPlan2d(&plan_fwd_r2c_, n, n, CUFFT_R2C_TYPE));
            CUFFT_CHECK(cufftPlan2d(&plan_bwd_c2r_, n, n, CUFFT_C2R_TYPE));
            CUFFT_CHECK(cufftPlan2d(&plan_bwd_c2c_, n, n, CUFFT_C2C_TYPE));
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

    
    void forward_c2c(Complex* data) override
    {
        CUFFT_CHECK(CUFFT_EXEC_C2C(plan_fwd_c2c_,
                                   reinterpret_cast<CuComplex*>(data),
                                   reinterpret_cast<CuComplex*>(data),
                                   CUFFT_FORWARD));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void backward_c2c(Complex* data) override
    {
        CUFFT_CHECK(CUFFT_EXEC_C2C(plan_bwd_c2c_,
                                   reinterpret_cast<CuComplex*>(data),
                                   reinterpret_cast<CuComplex*>(data),
                                   CUFFT_INVERSE));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void forward_r2c(Real* in_real, Complex* out_complex) override
    {
        CUFFT_CHECK(CUFFT_EXEC_R2C(plan_fwd_r2c_,
                                   reinterpret_cast<CuReal*>(in_real),
                                   reinterpret_cast<CuComplex*>(out_complex)));
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void backward_c2r(Complex* in_complex, Real* out_real) override
    {
        CUFFT_CHECK(CUFFT_EXEC_C2R(plan_bwd_c2r_,
                                   reinterpret_cast<CuComplex*>(in_complex),
                                   reinterpret_cast<CuReal*>(out_real)));
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

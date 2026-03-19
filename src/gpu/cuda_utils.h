#pragma once

#ifdef USE_GPU

#include <cuda_runtime.h>
#include <cufft.h>
#include <stdexcept>
#include <cstdio>

static constexpr int CUDA_BLOCK = 256;

#define CUDA_CHECK(call)                                                      \
    do {                                                                      \
        cudaError_t _e = (call);                                              \
        if (_e != cudaSuccess) {                                              \
            fprintf(stderr, "CUDA error %s:%d  %s\n",                         \
                    __FILE__, __LINE__, cudaGetErrorString(_e));              \
            throw std::runtime_error(cudaGetErrorString(_e));                 \
        }                                                                     \
    } while (0)

#define CUFFT_CHECK(call)                                                     \
    do {                                                                      \
        cufftResult _r = (call);                                              \
        if (_r != CUFFT_SUCCESS) {                                            \
            fprintf(stderr, "cuFFT error %s:%d  code=%d\n",                   \
                    __FILE__, __LINE__, (int)_r);                             \
            throw std::runtime_error("cuFFT error");                          \
        }                                                                     \
    } while (0)


#endif

#ifndef FFT_GPU_H
#define FFT_GPU_H

#ifdef USE_GPU
#include "fft.h"
#include <memory>
std::unique_ptr<FFTBackend> makeCUFFTBackend(int dim, size_t N, bool verbose);
#endif

#endif

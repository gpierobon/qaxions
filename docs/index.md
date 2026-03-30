---
layout: default
title: Home
nav_order: 1
---

# qaxions

`qaxions` is a C++ project to solve the Schrödinger-Poisson system of equations on a 2D/3D lattice.

[View it on GitLab][gitlab repo]{: .btn} 

## Overview

- Static Universe simulations to study the soliton interaction via gravity
- Axion star/minicluster formation around matter-radiation equality
- Interface with the [jaxions][jaxions repo] code

## Obtain the code

To download the source code from the public repository use:

```
git clone https://gitlab.com/gpierobon/qaxions.git
```

```
git clone https://github.com/gpierobon/qaxions.git
```

## Features

- High-performance C++ core
- OpenMP parallelism
- HDF5 and FFTW support
- GPU acceleration with CUDA and cuFFT
- Python bindings via `pybind11`
- Linux and macOS support
- Visualisation scripts

## Repository layout

```text
.
├── src/              # C++ sources
│   │ 
│   ├── core/               ← types.h, enum.h, profiler.h
│   ├── field/              ← field.h, field.cxx, field_ops ...
│   ├── fft/                ← fft.h, fft_cuda.cu, fft_cuda.h
│   ├── gpu/                ← cuda_utils.h, field_gpu.cu,
│   ├── ic/                 ← ic.h, ic.cxx, ...
│   ├── io/                 ← io.h, io.cxx
│   ├── meas/               ← meas.h, meas.cxx
│   ├── spectrum/           ← spectrum.h, spectrum.cxx
│   ├── propagator/         ← propagator.h, propagator.cxx
│   ├── examples/           ← pybind examples
│   └── main.cxx            ← main program
│ 
├── pyqaxions/        # Python package (generated at build time for the tests)
├── scripts/          # Visualisation scripts (bash + python)
├── tests/            # Testing scripts from pybind11
├── build/            # Build artifacts
├── bin/              # Binary files for plotting
├── output/           # Default directory with output snapshots/spectra
├── Makefile
└── README.md
```


[gitlab repo]: https://gitlab.com/gpierobon/qaxions.git
[github repo]: https://github.com/gpierobon/qaxions.git
[jaxions repo]: https://github.com/veintemillas/jaxions.git

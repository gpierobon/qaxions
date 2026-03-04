---
layout: default
title: Home
nav_order: 1
---

# qaxions

`qaxions` is a C++ project to solve the Schrödinger-Poisson system of equations on a 2D/3D lattice.

## Features

- High-performance C++ core
- Python bindings via `pybind11`
- OpenMP parallelism
- HDF5 and FFTW support
- Linux and macOS support

## Repository layout

```text
.
├── src/                 # C++ sources
│   ├── examples/        # pybind11 modules for tests (.cxx)
│   └── ...
├── pyqaxions/           # Python package (generated at build time for the tests)
├── tests/               # Testing scripts from pybind11
├── build/               # Build artifacts
├── bin/                 # Binary files for plotting
├── Makefile
└── README.md
```

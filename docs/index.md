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
- Fuzzy dark matter cosmological simulations 
- Interface with [jaxions][jaxions repo] 

Details on the physics of $$ \texttt{jaxions} $$ are found [here]({% link install.md %}).

## Obtain the code

To download the source code from the public repository use:

```
git clone https://gitlab.com/gpierobon/jaxions.git
```

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



[gitlab repo]: https://gitlab.com/gpierobon/qaxions.git
[github repo]: https://github.com/gpierobon/qaxions.git
[jaxions repo]: https://github.com/veintemillas/jaxions.git

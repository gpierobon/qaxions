---
layout: default
title: Installation
nav_order: 2
---

# Installation

## System requirements

- C++ compiler with C++17 support
- `make`
- Python >= 3.9
- `pybind11`

## Required libraries

- OpenMP
- HDF5
- FFTW

## Installing system dependencies

### macOS (Homebrew)

```bash
brew install libomp hdf5 fftw pybind11
```

### Linux (Ubuntu / Debian)

```bash
sudo apt install build-essential libomp-dev \
  libhdf5-dev libfftw3-dev pybind11-dev
```

## Python environment

It is strongly recommended to use a virtual environment:

```bash
python3 -m venv venv
source venv/bin/activate
pip install pybind11 pytest numpy h5py
```

---
layout: default
title: Installation
nav_order: 2
---

# Installation
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## System Requirements

| Requirement | Version |
|-------------|---------|
| C++ compiler | C++17 or later |
| `make` | any |
| Python | >= 3.9 |
| `pybind11` | any |

---

## Required Libraries

### FFTW3

The [Fastest Fourier Transform in the West](http://www.fftw.org/) must be built
with the following `./configure` options:

| Flag | Purpose |
|------|---------|
| `--enable-single` | Single-precision support |
| `--enable-openmp` | Multithreaded (shared memory) parallelism |
| `--enable-avx` / `--enable-avx2` / `--enable-avx512` | Vector (SIMD) operations |

### HDF5

The [Hierarchical Data Format](https://www.hdfgroup.org/solutions/hdf5/)
library, version **1.10.2 or later**, is required for data I/O.

---

## Installing Dependencies

### macOS (Homebrew)

Install Homebrew if not already present:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then install all dependencies in one step:

```bash
brew install libomp hdf5 fftw pybind11
```

### Linux (Ubuntu / Debian)

```bash
sudo apt install build-essential libomp-dev \
  libhdf5-dev libfftw3-dev pybind11-dev
```

### Linux (HPC Environment)

Most HPC systems provide these libraries as modules. Load them with:

```bash
module load <module_name>
```

To verify that a module was built with the required configuration options:

```bash
module show <module_name>
```

Then inspect the configure flags at the path shown.

### Building from Source

Use this approach if pre-built packages are unavailable or lack the required
configure options.

#### FFTW3

```bash
FFTW_VERSION="3.3.10"
FFTW_URL="http://www.fftw.org/fftw-${FFTW_VERSION}.tar.gz"
FFTW_SRC_DIR="/tmp/fftw-${FFTW_VERSION}"

wget -O /tmp/fftw-${FFTW_VERSION}.tar.gz ${FFTW_URL}
tar -xzvf /tmp/fftw-${FFTW_VERSION}.tar.gz -C /tmp
cd ${FFTW_SRC_DIR}

./configure --prefix=</path/to/install> \
            --enable-shared \
            --enable-threads \
            --enable-openmp \
            --enable-single   # omit for double precision only
make
make install
```

#### HDF5

```bash
HDF5_VERSION="1.12.1"
HDF5_URL="https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-${HDF5_VERSION%.*}/hdf5-${HDF5_VERSION}/src/hdf5-${HDF5_VERSION}.tar.gz"
HDF5_SRC_DIR="/tmp/hdf5-${HDF5_VERSION}"

wget -O /tmp/hdf5-${HDF5_VERSION}.tar.gz ${HDF5_URL}
tar -xzvf /tmp/hdf5-${HDF5_VERSION}.tar.gz -C /tmp
cd ${HDF5_SRC_DIR}

./configure --prefix=</path/to/install>
make
make install
```

---

## Python Environment

A virtual environment is strongly recommended for running tests and
visualisation scripts.

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

> **Note:** If `pybind11` is not found, the code will build successfully but
> without the Python-backed I/O, initial condition, and time-step tests. To
> enable those tests, ensure pybind11 is installed and build with
> `make WITH_PYTHON=1`.


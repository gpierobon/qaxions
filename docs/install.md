---
layout: default
title: Installation
nav_order: 2
---

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}
{: .highlight}


# Installation

## System requirements

- C++ compiler with C++17 support
- `make`
- Python >= 3.9
- `pybind11`

## Required libraries

- OpenMP
- Fastest Fourier Transform in the West ([FFTW3](http://www.fftw.org/)). To make us of the hybrid parallelisation it **needs to be installed** with the following options in the `./configure` step:
  -  `--enable-single` to allow for single precision
  <!-- -  `--enable-mpi` for distributed resources (MPI processes) -->
  -  `--enable-openmp` for shared resources (multithreading)
  -  `--enable-avx`, `--enable-avx2` or `--enable-avx512` to exploit vector operations
- Hierarchical Data Format ([HDF5](https://www.hdfgroup.org/solutions/hdf5/), version 1.10.2 or later). <!-- The `--enable-parallel` option **needs to be included** in the configuration to allow for parallel compression of large files. -->


## Installing system dependencies

### macOS (Homebrew)

To install brew:

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

```bash
brew install libomp hdf5 fftw pybind11
```

### Linux (Ubuntu / Debian)

```bash
sudo apt install build-essential libomp-dev \
  libhdf5-dev libfftw3-dev pybind11-dev
```

### Linux (HPC environment)

Most of these libraries  are typically available in most HPC environments, where they can be simply loaded by the command `module load <module_name>` etc. To make sure the parallel options are present type `module show <module_name>` and then check the configurations settings from the specified path.

### Building from source

To build the HDF5 libraries:

```
FFTW_VERSION="3.3.10"
FFTW_URL="http://www.fftw.org/fftw-${FFTW_VERSION}.tar.gz"
FFTW_SRC_DIR="/tmp/fftw-${FFTW_VERSION}"

wget -O /tmp/fftw-${FFTW_VERSION}.tar.gz ${FFTW_URL}
tar -xzvf /tmp/fftw-${FFTW_VERSION}.tar.gz -C /tmp

cd ${FFTW_SRC_DIR}
./configure --prefix=</path/to/install> --enable-shared --enable-threads --enable-openmp --enable-mpi (--enable single)
make
make install
```

To build the HDF5 libraries:

```
HDF5_VERSION="1.12.1"
HDF5_URL="https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-${HDF5_VERSION%.*}/hdf5-${HDF5_VERSION}/src/hdf5-${HDF5_VERSION}.tar.gz"
HDF5_SRC_DIR="/tmp/hdf5-${HDF5_VERSION}"

wget -O /tmp/hdf5-${HDF5_VERSION}.tar.gz ${HDF5_URL}
tar -xzvf /tmp/hdf5-${HDF5_VERSION}.tar.gz -C /tmp

cd ${HDF5_SRC_DIR}
./configure --prefix=</path/to/install> (--enable-parallel)
make
make install
```


## Python environment

It is strongly recommended to use a virtual environment to build tests and for visualisation. If `pybind` is not found, the code will build without the I/O, initial condition and time step tests against python. To enable the tests run the following:

```bash
python3 -m venv venv
source venv/bin/activate
pip install pybind11 pytest numpy h5py
```


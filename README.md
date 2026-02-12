# qaxions

`qaxions` is a C++ project to solve the Schroedinger-Poisson system of equations on a 2D/3D lattice.

---

## Features

- High-performance C++ core
- Python bindings via `pybind11`
- OpenMP parallelism
- HDF5 and FFTW support
- Linux and macOS support

---

## Repository layout

```text
.
├── src/                 # C++ sources
│   ├── examples/        # pybind11 modules fort tests (.cxx)
│   └── ...
├── pyqaxions/           # Python package (generated at build time for the tests)
├── tests/               # Testing scripts from pybind11
├── build/               # Build artifacts
├── bin/                 # Binary files for plotting
├── Makefile
└── README.md
```

## System requirements
### General

* C++ compiler with C++17 support
* make
* Python >= 3.9
* pybind11

### Required libraries

* OpenMP
* HDF5
* FFTW


## Installing system dependencies
### macOS (Homebrew)
```
brew install libomp hdf5 fftw pybind11
```

### Linux (example: Ubuntu / Debian)

```
  sudo apt install build-essential libomp-dev \
  libhdf5-dev libfftw3-dev pybind11-dev
```


## Python environment

It is strongly recommended to use a virtual environment:

```
python3 -m venv venv
source venv/bin/activate
pip install pybind11 pytest numpy h5py
```

## Building and testing

From the repository root, run:
```
make
pytest -v tests/
```

## Running the code
```
./qaxions
```

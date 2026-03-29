---
layout: default
title: Building & Testing
nav_order: 3
---

# Building

Once all the dependencies are installed and ready to be found, to build the code
simply run from the root directory:

```bash
make
```

This will compile the `qaxions` executable along with any Python bindings (if
enabled) and install the `bin/qaxi` helper script.

---

## Build Options

The Makefile exposes several flags that can be passed at the command line:

| Option | Default | Description |
|--------|---------|-------------|
| `WITH_DOUBLE` | `0` | Enable double-precision floating point (single by default) |
| `WITH_PYTHON` | `0` | Build pybind11 Python extension modules |
| `WITH_GADI` | `0` | Use NCI Gadi (HPC) library paths |
| `WITH_GPU` | `0` | Enable GPU acceleration via CUDA/cuFFT |
| `GPU_ARCH` | `sm_60` | Target CUDA GPU architecture |

### Double Precision

By default the code is built in single precision. To enable double precision:

```bash
make WITH_DOUBLE=1
```

### Python Bindings

Python bindings are built via [pybind11](https://github.com/pybind/pybind11).
They are disabled by default. To enable them:

```bash
make WITH_PYTHON=1
```

If pybind11 is not found on the system, the build will automatically fall back
to `WITH_PYTHON=0` with a warning. The extension modules are placed in the
`pyqaxions/` package directory.

> **Note:** Python bindings are automatically disabled when `WITH_GPU=1` is set, however this might change in the future.

### GPU Acceleration

GPU support requires CUDA and cuFFT. To enable it:

```bash
make WITH_GPU=1
```

The default target architecture is `sm_60`. To target a different GPU, set
`GPU_ARCH` accordingly. Common values:

| Value | Architecture |
|-------|-------------|
| `sm_60` | Pascal (default) |
| `sm_70` | Volta (e.g. `gpuvolta` queue on Gadi) |
| `sm_90` | Hopper (e.g. H200) |

Example:

```bash
make WITH_GPU=1 GPU_ARCH=sm_70
```

### Building on NCI Gadi (HPC)

For builds on the NCI Gadi supercomputer, use the `WITH_GADI` flag, which
selects the correct module paths for HDF5 and FFTW:

```bash
make WITH_GADI=1
```

---

## Platform Detection

The Makefile auto-detects the platform at build time and adjusts compiler and
linker flags accordingly:

| Platform | Detected when |
|----------|--------------|
| `mac` | `uname -s` returns `Darwin` |
| `gadi` | `WITH_GADI=1` is set on Linux |
| `linux` | All other Linux systems |

macOS builds use Homebrew-managed paths for OpenMP (`libomp`), HDF5, and FFTW.
Linux builds expect libraries under `/usr/local/hdf5_serial` and
`/usr/local/fftw3` by default.

---

## Combining Options

Options can be freely combined. For example, to build a double-precision GPU
binary targeting Volta GPUs:

```bash
make WITH_GPU=1 WITH_DOUBLE=1 GPU_ARCH=sm_70
```

---

# Testing

Run the full test suite with:

```bash
pytest -v tests/
```

### Python Module Tests

Python-binding tests require that the `pyqaxions` extension modules have been
built first (`make WITH_PYTHON=1`). Make sure the `pyqaxions/` directory is on
your Python path, or run pytest from the project root where it is automatically
discoverable.

```bash
make WITH_PYTHON=1
pytest -v tests/
```

### Running a Single Test File

```bash
pytest -v tests/test_<module>.py
```


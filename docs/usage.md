---
layout: default
title: Running the code
nav_order: 4
---

# Usage

## Running the Code

```bash
./qaxions [OPTIONS]
```

---

## Examples

Minimal run with all defaults:
```bash
./qaxions
```

Spectrum initial conditions and expanding cosmology:
```bash
./qaxions --N 256 --steps 1000 --nmeas 30 --meas 1 --ic spectrum --cosmo 1 
```

---

## Command-Line Options

### Grid & Simulation

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `--N <int>` | integer | `64` | Number of grid points per dimension |
| `--dim <int>` | integer | `3` | Spatial dimension (must be 2 or 3) |
| `--L <float>` | float | `1.0` | Physical box size |
| `--dt <float>` | float | `0.0001` | Time step size |
| `--steps <int>` | integer | `10` | Number of simulation steps |
| `--nmeas <int>` | integer | `20` | Number of steps between measurements |

### Initial Conditions & Physics

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `--ai <float>` | float | `0.1` | Initial scale factor |
| `--norm <float>` | float | `4000` | Field normalisation |
| `--seed <int>` | integer | `9` | Random seed |
| `--ic <string>` | string | `solitons` | Initial condition type: `solitons` or `spectrum` |
| `--cosmo <int>` | integer | `0` | Cosmology type: `0` = static, `1` = expanding |

### Output & Measurements

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `--dir <path>` | string | `output` | Output directory |
| `--meas <int>` | integer | `0` (NONE) | Measurement flags (bitmask, see below) |
| `--verb` | boolean flag | `false` | Enable verbose output |
| `--readj` | boolean flag | `false` | Enable re-adjustment |

### Performance

| Flag | Type | Default | Description |
|------|------|---------|-------------|
| `--nthr <int>` | integer | `1` | Number of OpenMP threads |
| `--fft <0–3>` | integer | `0` | FFTW plan type  |

#### `--fft` — FFTW Plan Type

| Value | Description |
|-------|-------------|
| `0` | `ESTIMATE` (default, fastest to plan) |
| `1` | `MEASURE` |
| `2` | `PATIENT` |
| `3` | `EXHAUSTIVE` (slowest to plan, fastest to execute) |

Higher values spend more time finding an optimal FFT plan, which can pay off for
long runs on fixed grid sizes.

---


## Getting Help

```bash
./qaxions --help
```


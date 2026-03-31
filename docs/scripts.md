---
layout: default
title: Visualisation
nav_order: 6
---

# Visualisation

A set of Python scripts is provided for live-playback visualisation of
simulation output. They are driven by the `scripts/qaxi` shell wrapper, which
selects the correct plot script and forwards arguments.

<video width="100%" controls>
  <source src="{{ site.baseurl }}/assets/videos/qaxi_dens.mp4" type="video/mp4">
</video>


---

## Quick Start

```bash
./bin/qaxi [TYPE] [DATA_DIR] [PAUSE]
```


| Argument | Default | Description |
|----------|---------|-------------|
| `TYPE` | `dens` | Plot type: `dens` (density field) or `spec` (power spectrum) |
| `DATA_DIR` | `output` | Directory containing simulation output files |
| `PAUSE` | `0.1` | Pause between frames in seconds (controls playback speed) |

---

## Plot Types

### `dens` — Density Field

```bash
./bin/qaxi dens output 0.1
```

Reads `field_*.h5` files from `DATA_DIR` and animates the projected density
field frame by frame.

**Requires** the field output to be enabled at run time:
```bash
./qaxions --meas 8
```

**What is plotted:**

For 3D simulations, the density field is averaged along the first axis to
produce a 2D projection. The plotted quantity is the log-density contrast:

$$\log_{10}\!\left(\rho / \langle\rho\rangle\right)$$


**HDF5 file layout expected:**

| Path | Description |
|------|-------------|
| `/rho/data` | Density array |
| `Header` attrs `N`, `dim`, `a` | Grid size, dimension, scale factor |

---

### `spec` — Power Spectrum

```bash
./bin/qaxi spec output 0.05
```

Reads `spec_*.txt` files from `DATA_DIR` and animates the dimensionless power
spectrum $\Delta^2_k$ as a function of wavenumber $k$.

**Requires** spectrum output to be enabled at run time:
```bash
./qaxions --meas 1
```

**What is plotted:**

The dimensionless power spectrum, defined as:

$$
\Delta^2_k = \frac{k^2 \, P(k)}{2\pi^2}
$$

Both axes are logarithmic. The initial condition spectrum is overplotted as a
fixed black reference line. The current spectrum is shown in blue and the title
tracks the scale factor $a$.

**Text file format expected:**

Each `spec_*.txt` file must have a header comment of the form:
```
# a: <value>
```
followed by three whitespace-separated columns: 
- `k`  Wavenumber 
- `P`  Power spectrum 
- `counts` Bin counts 

---

## Dependencies

The visualisation scripts require the following Python packages:

```bash
pip install h5py numpy matplotlib cmasher
```

A custom Matplotlib style sheet is expected at `scripts/sty.mplstyle`. Without
it the scripts will raise a warning but will still run using the default style.

---

## Tips

**Slow down or speed up playback** by adjusting the `PAUSE` argument:

```bash
./bin/qaxi dens output 0.5   # slower
./bin/qaxi dens output 0.01  # faster
```

**Replay after the run** — the scripts work equally well on completed runs as
on live output, since they simply iterate over sorted file lists.

**Enabling both outputs** simultaneously requires combining the `--meas`
flags. See the  [Usage]({% link usage.md %}) page for the full list of measurement flags.





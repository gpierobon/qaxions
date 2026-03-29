---
layout: default
title: Benchmarks
nav_order: 6
---


# Benchmarks

Performance benchmarks for parallel scaling on [**Gadi** — NCI's supercomputer](https://nci.org.au/our-systems/hpc-systems).

---

## System

| Property | Value |
|---|---|
| **System** | Gadi (NCI) |
| **CPU** |  Intel Xeon Platinum 8274 (Cascade Lake) 3.2 GHz  |
| **Cores per node** | 48 (2 × 24-core sockets) |
| **Compiler** | gcc 14.1.0 |


---

## Strong Scaling

A fixed problem size of $256^3$ is distributed across an increasing number of threads.
Runtime is wall-clock time in seconds.

### Results

| Threads | Runtime (s) | Speedup | Efficiency (%) |
|:---:|:---:|:---:|:---:|
| 1 | 253.6 | 1.00 | 100 |
| 2 | 123.9 | 2.00 | 100 |
| 4 | 64.3 | 3.94 | 98.6 |
| 8 | 32.8 | 7.73 | 96.7 |
| 16 | 16.8 | 15.11 | 94.4 |
| 32 | 8.7 | 29.12 | 91.1 |
| 48 | 6.5 | 38.99 | 81.2 |

> **Speedup** = T₁ / Tₙ &nbsp;·&nbsp; **Efficiency** = Speedup / N × 100

![Strong scaling speedup](assets/img/gadi_256_strong.png)

The dashed line shows ideal linear speedup. Deviation from ideal is expected due to
communication overhead and non-parallelisable portions of the code (Amdahl's Law).

---

## Weak Scaling

Problem size grows proportionally with thread count, so an ideal implementation
maintains constant runtime.

### Results

| Threads | Problem size | Runtime (s) |
|:---:|:---:|:---:|
| 1 | 128 | — |
| 2 | 256 | — |
| 4 | 512 | — |
| 8 | 1024 | — |
| 16 | 2048 | — |

<!--- ![Weak scaling runtime](weak_scaling.png) --->

---

## Conclusions

- **Strong scaling** is efficient up to $\mathcal{O}(20)$ threads, beyond which communication overhead
  begins to dominate.
- ...



<!--- ## Reproducing These Results

```bash

for nthr in 1 2 4 8 16 32 48; do
    ...
done

# Collect results
bash collect_timings.sh
```

See [`collect_timings.sh`](collect_timings.sh) for the timing extraction script
and [`plot_speedup.py`](plot_speedup.py) for the plotting code.
--->

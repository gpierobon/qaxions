---
layout: default
title: Physics
nav_order: 5
---


# The Schrödinger–Poisson System
{: .no_toc }


## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Static Universe

In the non-relativistic, non-expanding limit  the **Schrödinger–Poisson (SP)** system can be written as:

$$i\hbar\,\partial_t\psi = -\frac{\hbar^2}{2m_a}\nabla^2\psi + m\,\Phi\,\psi \tag{1}$$

$$\nabla^2\Phi = 4\pi G\!\left(m|\psi|^2 - \bar{\rho}\right) \tag{2}$$

where $\psi(\mathbf{x},t)$ is the complex wavefunction, $\rho = m|\psi|^2$ is
the mass density, $\Phi$ is the Newtonian gravitational potential, 
$m$ is the boson mass, $G$ is Newton's constant.


<!---$$E_\mathrm{kin} = \frac{\hbar^2}{2m_a}\int|\nabla\psi|^2\,d^3x, \qquad E_\mathrm{grav} = \frac{m_a}{2}\int|\psi|^2\Phi\,d^3x.$$ --->

---

### The Unit System

The SP system contains four dimensional quantities: $\hbar$, $m$, $G$, $\bar{\rho}$.
There are three independent physical dimensions, length $[\mathrm{L}]$, 
time $[\mathrm{T}]$ and mass $[\mathrm{M}]$, so we may freely set **three** combinations to unity. 
The remaining combination is a single dimensionless number that controls all the physics.

The relevant dimensional groups are:

$$[\hbar/m] = \mathrm{L}^2\,\mathrm{T}^{-1}, \qquad [G\bar\rho]^{1/2} = \mathrm{T}^{-1}, \qquad [\bar\rho] = \mathrm{M}\,\mathrm{L}^{-3}.$$

These three span all dimensions. The natural choice of code units is:

$$\boxed{\hbar/m=1, \quad L_{\rm box}=1, \quad \bar{\rho}=1}$$

---

### The Dimensionless System

With these three choices the SP equations become:

$$i\,\partial_{t}\tilde\psi = -\tfrac{1}{2}\nabla^2\psi + V\,\psi \tag{3}$$

$$\nabla^2{V} = 4\pi\alpha\!\left(|\psi|^2 - 1\right) \tag{4}$$

with the dimensionless variables $\psi\to\psi(m/\rho)^{1/2}$ and $V\to VmT/\hbar$.

The **dimensionless control parameter** is:

$$\boxed{\alpha \;\equiv\; \frac{G\,m^2\,\bar{\rho}\,L_\mathrm{box}^4}{\hbar^2}} \tag{5}$$

which also measures how many quantum Jeans lengths can fit in the simulation box:

$$\alpha = \left(\frac{L_\mathrm{box}}{\ell_J}\right)^4, \qquad \ell_J = \left(\frac{\hbar^2}{G m_a^2 \bar\rho}\right)^{1/4}.$$

Then, the split-step leapfrog integrates (3)–(4) via a **drift** $e^{-i\Delta\tilde{t}\,\tilde{k}^2/2}$ 
in Fourier space and a **kick** $e^{-i\Delta\tilde{t}\,\tilde{V}/2}$ in real space.

We note that for  $\alpha \lesssim 1$, quantum pressure dominates and density perturbations cannot grow, 
while in the $\alpha \gg 1$ case, gravity dominates. In such case, 
many Jeans-unstable modes fit in the box and the collapse produces solitons, filaments and halos.

---

### Recovering Physical Quantities

The dimensionless simulation produces patterns of $|\psi|^2$ on a unit box.
Translating these back to physical units requires supplying two independent 
external quantities, along with a choice of $\alpha$ in the simulation. 

The constraint that must always be satisfied is:

$$\alpha = \frac{G\,m^2\,\bar{\rho}_\mathrm{phys}\,\ell_\mathrm{phys}^4}{\hbar^2} \tag{6}$$

This is one equation relating four unknowns $(m,\,\bar\rho_\mathrm{phys},\,\ell_\mathrm{phys},\,G)$.
Since $G$ is a physical constant, we have **two free choices** among $\{m,\,\bar\rho_\mathrm{phys},\,\ell_\mathrm{phys}\}$; fixing any two determines the third via (6).

The three natural strategies are:

##### Option A: Fix $m_a$ and $\bar\rho_\mathrm{phys}$ 

The boson mass $m$ and cosmological mean density $\bar\rho_\mathrm{phys} = 3H_0^2\Omega_{m0}/(8\pi G)$ are both specified; the physical box size and timescale follow from (6):

$$\ell_\mathrm{phys} = \left(\frac{\alpha\,\hbar^2}{G\,m_a^2\,\bar{\rho}_\mathrm{phys}}\right)^{1/4} = \alpha^{1/4}\,\ell_J,$$

$$t_\mathrm{phys} = \frac{m \ell^2_{\rm phys}}{\hbar} = \alpha^{1/2}\,t_J.$$

##### Option B: Fix $m_a$ and $\ell_\mathrm{phys}$ 

Choose a physical box size (e.g. $\ell_\mathrm{phys} = 1\,\mathrm{Mpc}$) and a boson mass. The mean density required for self-consistency is:

$$\bar{\rho}_\mathrm{phys} = \frac{\alpha\,\hbar^2}{G\,m_a^2\,\ell_\mathrm{phys}^4}.$$

This is useful when studying a specific object (a dwarf galaxy, a halo of known size) where $\ell_\mathrm{phys}$ is observationally motivated.

##### Option C: Fix $\ell_\mathrm{phys}$ and $\bar\rho_\mathrm{phys}$ 

$$m_a = \hbar\left(\frac{\alpha}{G\,\bar{\rho}_\mathrm{phys}\,\ell_\mathrm{phys}^4}\right)^{1/2}.$$

This tells you which boson mass would produce the observed structure on a given scale at a given mean density.
Useful for inference.

---

### Quantities in Code Units

Once strategy A, B, or C is chosen, all other physical scales follow:

| Quantity | Code value | Physical formula |
|----------|-----------|-----------------|
| Quantum Jeans length | $\tilde\ell_J = \alpha^{-1/4}$ | $\ell_J = (\hbar^2/G m_a^2\bar\rho)^{1/4}$ |
| Jeans (free-fall) time | $\tilde t_J = (4\pi\alpha)^{-1/2}$ | $t_J = (4\pi G\bar\rho)^{-1/2}$ |
| Jeans mass | $\tilde M_J = \alpha^{-3/4}$ | $M_J = \bar\rho\,\ell_J^3$ |
| Time unit | 1 | $t_0 = m_a\,\ell_\mathrm{phys}^2/\hbar$ |
| Velocity unit | 1 | $v_0 = \ell_\mathrm{phys}/t_0 = \hbar/(m_a\,\ell_\mathrm{phys})$ |
| Gravitational constant | $\alpha$ | $G = \alpha\,\hbar^2/(m_a^2\bar\rho\,\ell_\mathrm{phys}^4)$ |

---

## Expanding Universe


import os, sys
import numpy as np
import pytest

root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(root_dir)

import pyqaxions.fieldtest as field
import helper as h


@pytest.mark.parametrize("n", [2,3])
@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("dt", [0.05, 0.1])
@pytest.mark.parametrize("idx", [1, 9])
def test_kick(n, N, dt, idx):
    norm = 1.0

    # C++ kick
    psi_c = field.test_kick(n, N, dt, idx)

    # python kick
    if n == 2:
        rho = h.get_solitons(N)
        psi_py = rho.astype(complex)
        k2 = h.get_k2(N)
    elif n == 3:
        rho = h.get_solitons_3D(N)
        psi_py = rho.astype(complex).reshape(N, N, N).transpose(2, 1, 0)
        k2 = h.get_k2_3D(N)

    Vhat = -np.fft.fftn(4.0 * np.pi * norm * (np.abs(psi_py)**2 - 1.0)) / (k2 + (k2 == 0))
    V = np.real(np.fft.ifftn(Vhat))
    psi_py *= np.exp(-1.0j * dt * V)

    expected = psi_py.flat[idx].real
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)


@pytest.mark.parametrize("n", [2,3])
@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("dt", [0.05, 0.1])
@pytest.mark.parametrize("idx", [1, 9])
def test_drift(n, N, dt, idx):
    norm = 1.0

    # C++ drift 
    psi_c = field.test_drift(n, N, dt, idx)

    # python drift
    if n == 2:
        rho = h.get_solitons(N)
        psi_py = rho.astype(complex)
        k2 = h.get_k2(N)
    elif n == 3:
        rho = h.get_solitons_3D(N)
        psi_py = rho.astype(complex).reshape(N, N, N).transpose(2, 1, 0)
        k2 = h.get_k2_3D(N)

    psihat = np.fft.fftn(psi_py)
    psihat = np.exp(dt * (-1.0j * k2 * 0.5)) * psihat
    psi_py = np.fft.ifftn(psihat)

    expected = psi_py.flat[idx].real
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)


@pytest.mark.parametrize("n", [2,3])
@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("idx", [1, 9])
def test_potential(n, N, idx):
    norm = 1.0

    # C++ kick
    V_c = field.test_potential(n, N, idx)

    # python kick
    if n == 2:
        rho = h.get_solitons(N)
        psi_py = rho.astype(complex)
        k2 = h.get_k2(N)
    elif n == 3:
        rho = h.get_solitons_3D(N)
        psi_py = rho.astype(complex).reshape(N, N, N).transpose(2, 1, 0)
        k2 = h.get_k2_3D(N)

    Vhat = -np.fft.fftn(4.0 * np.pi * norm * (np.abs(psi_py)**2 - 1.0)) / (k2 + (k2 == 0))
    V_py = np.real(np.fft.ifftn(Vhat))

    expected = V_py.flat[idx].real
    assert V_c == pytest.approx(expected, abs=1e-5, rel=1e-4)


@pytest.mark.parametrize("n", [2,3])
@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("dt", [0.05, 0.1])
@pytest.mark.parametrize("idx", [1, 9])
def test_kick_drift_step(n, N, dt, idx):
    norm = 1.0

    # C++ kick
    psi_c = field.test_kick_drift_step(n, N, dt, idx)

    # python kick
    if n == 2:
        rho = h.get_solitons(N)
        psi_py = rho.astype(complex)
        k2 = h.get_k2(N)
    elif n == 3:
        rho = h.get_solitons_3D(N)
        psi_py = rho.astype(complex).reshape(N, N, N).transpose(2, 1, 0)
        k2 = h.get_k2_3D(N)

    Vhat = -np.fft.fftn(4.0 * np.pi * norm * (np.abs(psi_py)**2 - 1.0)) / (k2 + (k2 == 0))
    V = np.real(np.fft.ifftn(Vhat))

    psi = np.exp(-1.0j * dt / 2.0 * V) * psi_py
    psihat = np.fft.fftn(psi)
    psihat = np.exp(dt * (-1.0j * k2 / 2.0)) * psihat
    psi = np.fft.ifftn(psihat)

    Vhat = -np.fft.fftn(4.0 * np.pi * norm * (np.abs(psi) ** 2 - 1.0)) / (
        k2 + (k2 == 0)
    )
    V = np.real(np.fft.ifftn(Vhat))
    psi_py = np.exp(-1.0j * dt / 2.0 * V) * psi

    expected = psi_py.flat[idx].real
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)



import os, sys
import numpy as np
import pytest

root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(root_dir)

import pyqaxions.ictest as ic
import helper as h


@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("idx", [1, 9])
def test_ic_solitons(N, idx):
    norm = 1.0

    # C++ kick
    psi_c = ic.test_solitons(N, idx)
    print(psi_c)

    # python kick
    rho = h.get_solitons(N)
    expected = rho.flat[idx]
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)

@pytest.mark.parametrize("N", [4, 8])
@pytest.mark.parametrize("idx", [1, 9])
def test_ic_solitons_3D(N, idx):
    norm = 1.0

    # C++ kick
    psi_c = ic.test_solitons_3D(N, idx)

    # python kick
    rho = h.get_solitons_3D(N)
    expected = rho.flat[idx]
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)



import os, sys
import numpy as np
import h5py as h5
import pytest

root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(root_dir)

import pyqaxions.h5test as h

@pytest.mark.parametrize("idx", [0, 1, 2, 3])
def test_read_write(idx):

    max_err = h.test_read_write(idx, "test_rw.h5")
    assert max_err < 1e-10, f"Round-trip error too large at idx={idx}: {max_err:.2e}"

@pytest.mark.parametrize("idx", [1, 9])
def test_read_jaxions(idx):

    # C++ read 
    psi_c = h.test_read_jaxions(idx)

    # python read 
    with h5.File('jaxions_2D.hdf5', 'r') as f:
        m_data = f['m'][:]

    expected = m_data.flat[idx]
    assert psi_c == pytest.approx(expected, abs=1e-5, rel=1e-4)

import os, glob
import sys
import h5py as h5
import numpy as np
import matplotlib.pyplot as plt
import cmasher as cmr
plt.style.use('scripts/sty.mplstyle')

data_dir = sys.argv[1]
speed    = float(sys.argv[2])

# Check for meas files with projection first
meas_pattern  = os.path.join(data_dir, "meas_*.h5")
field_pattern = os.path.join(data_dir, "field_*.h5")
meas_files  = sorted(glob.glob(meas_pattern))
field_files = sorted(glob.glob(field_pattern))

use_meas = False
if len(meas_files) > 0:
    # Check if first file has projection data
    with h5.File(meas_files[0], "r") as f:
        if 'P/data' in f:
            use_meas = True

if use_meas:
    files = meas_files
    print(f"Found {len(files)} meas files with projection data")
elif len(field_files) > 0:
    files = field_files
    print(f"Found {len(files)} field files (will project along z)")
else:
    print("No field or meas files found!")
    exit(0)

plt.ion()
fig, ax = plt.subplots(figsize=(8, 8))
im = None

for fname in files:
    with h5.File(fname, "r") as f:
        N   = int(f['Header'].attrs['N'])
        dim = int(f['Header'].attrs['dim'])
        a   = float(f['Header'].attrs['a'])

        if use_meas:
            rho = np.array(f['P/data']).reshape(N, N)
        else:
            density = np.array(f['/rho/data'])
            me  = np.mean(density)
            rho = density / me
            if dim == 3:
                rho = np.mean(rho, axis=0)

    me = np.mean(rho)
    rho = rho / me
    logdens = np.log10(rho)

    if im is None:
        im = ax.imshow(
            logdens,
            cmap=cmr.pride,
            vmin=-1,
            vmax=2,
            origin="lower"
        )
        ax.set_xticks([])
        ax.set_yticks([])
        cbar = plt.colorbar(im, ax=ax)
        cbar.set_label(r"$\log_{10}(\rho/\bar{\rho})$")
    else:
        im.set_data(logdens)

    ax.set_title(r'$a = %.4f$' % a)
    plt.pause(speed)

plt.ioff()
plt.show()

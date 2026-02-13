import os, glob
import sys
import h5py as h5
import numpy as np
import matplotlib.pyplot as plt
import cmasher as cmr
plt.style.use('scripts/sty.mplstyle')


data_dir = sys.argv[1]
speed    = float(sys.argv[2])
pattern = os.path.join(data_dir, "field_*.h5")
files = sorted(glob.glob(pattern))
print(f"Found {len(files)} field files")

plt.ion()

fig, ax = plt.subplots(figsize=(8, 8))
im = None

for fname in files:
    with h5.File(fname, "r") as f:
        density = np.array(f['/rho/data'])
        N = int(f['Header'].attrs['N'])
        dim = int(f['Header'].attrs['dim'])
        time = float(f['Header'].attrs['time'])

    me = np.mean(density)
    rho = density / me
    if dim == 3:
        rho = np.mean(rho, axis=0)
        #rho = np.mean(rho[N//2-N//16:N//2+N//16], axis=0)

    logdens = np.log10(rho)

    if im is None:
        im = ax.imshow(
            logdens,
            cmap=cmr.pride,
            vmin=-0.5,
            vmax=2,
            origin="lower"
        )
        ax.set_xticks([])
        ax.set_yticks([])
        cbar = plt.colorbar(im, ax=ax)
        cbar.set_label(r"$\log_{10}(\rho)$")
    else:
        im.set_data(logdens)

    ax.set_title(r'$s = %.4f$'%time)
    plt.pause(speed)   # controls playback speed

plt.ioff()
plt.show()

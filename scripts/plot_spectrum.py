import os, glob
import sys
import h5py as h5
import numpy as np
import matplotlib.pyplot as plt
import cmasher as cmr
plt.style.use('scripts/sty.mplstyle')

data_dir = sys.argv[1]
speed    = float(sys.argv[2])
pattern = os.path.join(data_dir, "spec_*.txt")
files = sorted(glob.glob(pattern))
print(f"Found {len(files)} field files")

plt.ion()

fig, ax = plt.subplots(figsize=(8, 8))
im = None

norm = 2*np.pi**2

fname0 = files[0]
k, P0, c0 = np.loadtxt(fname0, unpack=True)

line0, = ax.loglog(k, k**2*P0/norm, c='k', label='IC')

line, = ax.loglog([], [], c='C0')

ax.set_xlabel("k")
ax.set_ylabel(r"$\Delta^2_k$")
ax.set_ylim(1e-3, 15)
ax.legend()

for fname in files:
    with open(fname, 'r') as f:
        a = float(f.readline().split(':')[1])
        k, P, counts = np.loadtxt(fname, unpack=True)

    y = k**2 * P / norm

    line.set_data(k, y)
    ax.relim()
    ax.autoscale_view()
    ax.set_title(r'$a=%.3f$'%a)
    plt.pause(speed)

plt.ioff()
plt.show()

import numpy as np


def get_k2(N):
    # L=1
    klin = 2.0 * np.pi * np.arange(-N / 2, N / 2)
    kx, ky = np.meshgrid(klin, klin)
    kx = np.fft.ifftshift(kx)
    ky = np.fft.ifftshift(ky)
    return kx**2 + ky**2

def get_k2_3D(N):
    # L = 1
    klin = 2.0 * np.pi * np.arange(-N / 2, N / 2)
    kx, ky, kz = np.meshgrid(klin, klin, klin, indexing='ij')
    kx = np.fft.ifftshift(kx)
    ky = np.fft.ifftshift(ky)
    kz = np.fft.ifftshift(kz)
    return kx**2 + ky**2 + kz**2

def get_solitons(N, filename='solitons.txt'):
    solitons = []
    with open(filename, 'r') as file:
        for line in file:
            values = line.split()
            if len(values) == 5:
                amp, sigma, x, y, z = map(float, values)
                norm = 1 / (sigma**3 * 2 * np.pi)
                solitons.append((amp, sigma, x, y, z, norm))

    xlin = np.linspace(0, 1, num=N + 1)
    xlin = xlin[0:N]
    xx, yy = np.meshgrid(xlin, xlin)

    rho = np.ones_like(xx)*1.0

    for amp, sigma, x_c, y_c, z_c, norm in solitons:
        r2 = (xx - x_c) ** 2 + (yy - y_c) ** 2
        rho += (amp * np.exp(-r2 / (2 * sigma**2)) * norm)

    rhobar = np.mean(rho)
    rho /= rhobar
    psi = np.sqrt(rho)

    return psi

def get_solitons_3D(N, filename='solitons.txt'):
    solitons = []
    with open(filename, 'r') as file:
        for line in file:
            values = line.split()
            if len(values) == 5:
                amp, sigma, x, y, z = map(float, values)
                norm = 1 / (sigma**3 * 2 * np.pi)
                solitons.append((amp, sigma, x, y, z, norm))

    xlin = np.linspace(0, 1, num=N + 1)
    xlin = xlin[0:N]
    xx, yy, zz = np.meshgrid(xlin, xlin, xlin, indexing='ij')

    rho = np.ones_like(xx) * 1.0

    for amp, sigma, x_c, y_c, z_c, norm in solitons:
        r2 = ((xx - x_c)**2 +
              (yy - y_c)**2 +
              (zz - z_c)**2)
        rho += amp * np.exp(-r2 / (2 * sigma**2)) * norm

    rhobar = np.mean(rho)
    rho /= rhobar
    psi = np.sqrt(rho)

    return psi


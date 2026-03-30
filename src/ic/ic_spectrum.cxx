#include <iostream>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <complex>
#include <random>
#include <cmath>
#include <sstream>
#include <omp.h>

#include "ic_spectrum.h"
#include "../core/profiler.h"

struct PowerSpectrum
{
    std::vector<double> k;
    std::vector<double> Pk;
};

PowerSpectrum readPowerSpectrum(const std::string& filename, bool verb)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    PowerSpectrum ps;
    std::string line;

    while (std::getline(file, line))
    {
        // Skip empty lines and comments (lines starting with #)
        if (line.empty() || line[0] == '#') continue;

        double k_val, Pk_val;
        if (std::istringstream(line) >> k_val >> Pk_val)
        {
            ps.k.push_back(k_val);
            ps.Pk.push_back(Pk_val);
        }
    }
    
    if (ps.k.empty()) 
        throw std::runtime_error("No data read from file: " + filename);

    if (verb)
        std::cout << "[IC spectrum] Loaded " << ps.k.size() 
                  << " modes from " << filename << "\n";
    return ps;
}


double interpolatePk(const PowerSpectrum& ps, double k)
{
    if (k <= 0.0 || k < ps.k.front() || k > ps.k.back()) return 0.0;

    // Binary search for the bracketing interval
    size_t lo = 0, hi = ps.k.size() - 1;
    while (hi - lo > 1)
    {
        size_t mid = (lo + hi) / 2;
        (k >= ps.k[mid] ? lo : hi) = mid;
    }

    // Interpolate in log-log space
    double logk0 = std::log(ps.k[lo]),  logk1 = std::log(ps.k[hi]);
    double logP0 = std::log(ps.Pk[lo]), logP1 = std::log(ps.Pk[hi]);
    double t = (std::log(k) - logk0) / (logk1 - logk0);
    return std::exp(logP0 + t * (logP1 - logP0));
}


void SpectrumIC::apply(Field& field) const
{
    PROFILE(IC);

    const int    dim   = field.dim();
    if (dim != 3)
        throw std::runtime_error("Can't run 2D simulations with spectrum IC!");

    const int    N     = field.size();
    const double a     = field.a();
    const double L     = field.Lbox();
    const bool   verb  = field.verb();
    
    const double dk    = 2.0 * M_PI / L;
    const int    hN    = N / 2;
    double       norm  = L * L * std::sqrt(a);

    unsigned int seed = p_.seed;
    const std::string pk_file = p_.pk_file;
    
    PowerSpectrum pk;
    
    pk = readPowerSpectrum(pk_file, verb);

    if (verb)
        std::cout << "[IC spectrum] Grid loop ..." << std::endl;

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        std::mt19937_64 local_rng(seed + thread_id);
        std::normal_distribution<double> local_gauss(0.0, 1.0);
        
        #pragma omp for collapse(3) schedule(static)
        for (int iz = 0; iz < N; ++iz)
        for (int iy = 0; iy < N; ++iy)
        for (int ix = 0; ix < N; ++ix)
        {
            int nx = ix;
            int ny = (iy <= hN) ? iy : iy - N; 
            int nz = (iz <= hN) ? iz : iz - N;
            int idx = ix * N * N + iy * N + iz;
            
            Real kx = dk * nx;
            Real ky = dk * ny;
            Real kz = dk * nz;
            Real kabs = std::sqrt(kx*kx + ky*ky + kz*kz);

            double a = local_gauss(local_rng);
            double b = local_gauss(local_rng);

            double Pk = interpolatePk(pk, kabs) / norm;
            double amp = std::sqrt(Pk);

            field.psi()[idx][0] = a * amp;
            field.psi()[idx][1] = b * amp;
         }
    }
    
    if (verb)
        std::cout << "[IC spectrum] Inverse FFT ..." << std::endl;
    
#ifdef USE_GPU
    field.toDevice();
#endif
    field.fft_backward_c2c();
#ifdef USE_GPU
    field.toHost();
#endif
    
    if (verb)
        std::cout << "[IC spectrum] done! " << std::endl;
}


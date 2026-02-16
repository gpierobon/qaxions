#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include "solitons.h"
#include "../profiler.h"

struct Soliton
{
    double amp;
    double sigma;
    double norm;
    double x, y, z;
};


void SolitonsIC::apply(Field& field) const
{
    PROFILE(IC);
    const int    N     = field.size();
    const int    dim   = field.dim();
    const double L     = field.Lbox();
    const size_t sites = field.sites();
    const double dx    = L / N;
    const bool   verb  = field.verb();

    std::vector<Soliton> solitons;

    std::ifstream file("solitons.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file solitons.txt !" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        Soliton sol;
        if (ss >> sol.amp >> sol.sigma >> sol.x >> sol.y >> sol.z)
        {
            sol.norm = 1 / (sol.sigma * sol.sigma * sol.sigma * 2 * M_PI);
            solitons.push_back(sol);
        }
        else
            std::cerr << "Error parsing line: " << line << std::endl;
    }
    file.close();

    if (verb)
    {
        std::cout << "[IC solitons] Read the soliton file!" << std::endl;
        std::cout << "[IC solitons] Grid loop ..." << std::endl;
    }

    double bkg = p_.sol_bkg;
    std::vector<double> rho(sites, bkg);
    double rho_sum = 0.0;

    #pragma omp parallel for reduction(+:rho_sum)
    for (size_t idx = 0; idx < sites; ++idx)
    {
        double x, y, z = 0.0;
        if (dim == 3)
        {
            int iz = idx / (N * N);
            int iy = (idx / N) % N;
            int ix = idx % N;
            x = ix * dx;
            y = iy * dx;
            z = iz * dx;
        }
        else
        { 
            int iy = idx / N;
            int ix = idx % N;
            x = ix * dx;
            y = iy * dx;
        }

        for (const auto& sol : solitons)
        {
            double dx = x - sol.x * L;
            double dy = y - sol.y * L;
            double dz = (dim == 3) ? (z - sol.z * L) : 0.0;
            double r2 = dx*dx + dy*dy + dz*dz;

            rho[idx] += sol.amp * sol.norm * std::exp(-r2 / (2 * sol.sigma * sol.sigma));
        }

        rho_sum += rho[idx];
    }

    if (verb)
        std::cout << "[IC solitons] done!" << std::endl;

    double mean_rho = rho_sum / static_cast<double>(sites);

    #pragma omp parallel for
    for (size_t idx = 0; idx < sites; ++idx)
    {
        double rho_norm = rho[idx] / mean_rho;

        field.psi()[idx][0] = std::sqrt(rho_norm);
        field.psi()[idx][1] = 0.0;
    }
}


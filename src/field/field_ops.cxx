#include "field.h"
#include "../core/profiler.h"


// ----------------------------------------------------------------------------
//   Kick - CPU
// ----------------------------------------------------------------------------
void Field::kick_cpu(double dt)
{
    const Real fac = -dt;

    #pragma omp parallel for simd//schedule(dynamic)
    for (size_t idx = 0; idx < sites_; ++idx)
    {
        Real phase = fac * V_[idx];
        Real re = psi_[idx][0];
        Real im = psi_[idx][1];
        
        Real cos_p = std::cos(phase);
        Real sin_p = std::sin(phase);
        
        psi_[idx][0] = re * cos_p - im * sin_p;
        psi_[idx][1] = re * sin_p + im * cos_p;
    }
}

//
// Example of kick kernel using explicit vectorisation, about ~20% faster 
//

//#include <hwy/highway.h>
//#include <hwy/contrib/math/math-inl.h>
//
//namespace hn = hwy::HWY_NAMESPACE;
//
//void Field::kick_cpu(double dt)
//{
//    const hn::ScalableTag<double> d;
//    const size_t N = hn::Lanes(d);
//    const double fac = -dt;
//
//    #pragma omp parallel for
//    for (size_t idx = 0; idx < sites_; idx += N)
//    {
//        auto phase = hn::Mul(hn::LoadU(d, &V_[idx]), hn::Set(d, fac));
//        auto cos_p = hn::Cos(d, phase);
//        auto sin_p = hn::Sin(d, phase);
//
//        auto re = hn::LoadU(d, &psi_[idx][0]);
//        auto im = hn::LoadU(d, &psi_[idx][1]);
//
//        hn::StoreU(hn::Sub(hn::Mul(re, cos_p), hn::Mul(im, sin_p)), d, &psi_[idx][0]);
//        hn::StoreU(hn::Add(hn::Mul(re, sin_p), hn::Mul(im, cos_p)), d, &psi_[idx][1]);
//    }
//}


// ----------------------------------------------------------------------------
//   Drift_k2 - CPU
// ----------------------------------------------------------------------------
void Field::drift_k2_cpu(double dt)
{
    const Real fac = -0.5 * dt;
    const Real dk  = 2.0 * M_PI / Lbox_;
    const int hN     = N_ / 2;

    #pragma omp parallel for
    for (size_t idx = 0; idx < sites_; ++idx)
    {
        Real kx = 0.0, ky = 0.0, kz = 0.0;

        if (dim_ == 3)
        {
            int iz = idx % N_;
            int iy = (idx / N_) % N_;
            int ix = idx / (N_ * N_);

            int nx = (ix <= hN) ? ix : ix - N_;
            int ny = (iy <= hN) ? iy : iy - N_;
            int nz = (iz <= hN) ? iz : iz - N_;

            kx = nx * dk;
            ky = ny * dk;
            kz = nz * dk;
        }
        else // 2D
        {
            int iy = idx / N_;
            int ix = idx % N_;

            int nx = (ix <= hN) ? ix : ix - N_;
            int ny = (iy <= hN) ? iy : iy - N_;

            kx = nx * dk;
            ky = ny * dk;
        }

        Real k2 = kx*kx + ky*ky + kz*kz;
        Real phase = fac * k2;

        Real re = psi_[idx][0];
        Real im = psi_[idx][1];

        Real cos_p = std::cos(phase);
        Real sin_p = std::sin(phase);

        psi_[idx][0] = (re * cos_p - im * sin_p) / sites_;
        psi_[idx][1] = (re * sin_p + im * cos_p) / sites_;
    }
}


// ----------------------------------------------------------------------------
//   Poisson - CPU
// ----------------------------------------------------------------------------
void Field::Poisson_cpu()
{
    int hN  = N_ / 2;
    int hN1 = N_ / 2 + 1;
    const Real twopi = 2.0 * M_PI / Lbox_;

    if (dim_ == 2)
    {
        Real vol = N_ * N_;

        #pragma omp parallel for collapse(2) schedule(static) default(shared)
        for (int iy = 0; iy < N_; ++iy)
        {
            for (int ix = 0; ix < hN1; ++ix)
            {
                int nx = ix;  
                int ny = (iy <= hN) ? iy : iy - N_; 
                int idx = iy * hN1 + ix;

                Real kx = twopi * nx; 
                Real ky = twopi * ny; 
                                            
                Real k2 = kx*kx + ky*ky;
                k2 = k2 + (k2 == 0.0);
                Real fac = - 1 / k2 / vol;

                Vhat_[idx][0] *= fac;
                Vhat_[idx][1] *= fac;
            }
        }
    }
    else if (dim_ == 3)
    {
        Real vol = N_ * N_ * N_;

        #pragma omp parallel for collapse(3) schedule(static) default(shared)
        for (int iz = 0; iz < N_; ++iz)
        {
            for (int iy = 0; iy < N_; ++iy)
            {
                for (int ix = 0; ix < hN1; ++ix)
                {
                    int nx = ix;
                    int ny = (iy <= hN) ? iy : iy - N_; 
                    int nz = (iz <= hN) ? iz : iz - N_;
                    int idx = iz * N_ * hN1 + iy * hN1 + ix; 
                    
                    Real kx = twopi * nx;
                    Real ky = twopi * ny;
                    Real kz = twopi * nz;
                                            
                    Real k2 = kx*kx + ky*ky + kz*kz;
                    k2 = k2 + (k2 == 0.0);
                    Real fac = - 1 / k2 / vol;

                    Vhat_[idx][0] *= fac;
                    Vhat_[idx][1] *= fac;
                }
            }
        }
    }
}


void Field::updatePotential_cpu()
{
    PROFILE(POISSON);

    const Real pref = alpha_ * a_;

    #pragma omp parallel for simd //schedule(static)
    for (size_t i=0; i < sites_; ++i) 
    {
        Real rho = psi_[i][0] * psi_[i][0] + psi_[i][1] * psi_[i][1];
        V_[i] = pref * (rho - rho_mean_);
    }
    
    if (verb_)
        std::cout << "[updatePotential] Starting 2rc forward ..." << std::endl;

    fft_forward_r2c();

    if (verb_)
    {
        std::cout << "[updatePotential] 2rc forward done!" << std::endl;
        std::cout << "[updatePotential] Starting Poisson ... " << std::endl;
    }

    Poisson_cpu();

    if (verb_)
    {
        std::cout << "[updatePotential] Poisson done!" << std::endl;
        std::cout << "[updatePotential] Starting c2r backward ... " << std::endl;
    }

    fft_backward_c2r();

    if (verb_)
        std::cout << "[updatePotential] c2r backward done!" << std::endl;

    Real local_max = 0.0;
    #pragma omp parallel for reduction(max:local_max)
    for (size_t i = 0; i < sites_; ++i)
    {
        Real absV = std::abs(V_[i]);
        if (absV > local_max) local_max = absV;
    }
    Vmax_ = local_max;  
    dsK_ = 2.0 * M_PI / Vmax_;
}



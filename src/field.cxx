#include <array>
#include <cmath>
#include <iostream>
#include <omp.h>

#include "field.h"
#include "parse.h"
#include "enum.h"
#include "utils.h"
#include "profiler.h"
#include "fft/fft.h"


void Field::init(const Params& pars)
{
    PROFILE(FIELD);

    if (psi_)  { fftw_free(psi_); psi_   = nullptr; }
    if (V_)    { fftw_free(V_);     V_   = nullptr; }
    if (Vhat_) { fftw_free(Vhat_); Vhat_ = nullptr; }

    N_      = pars.N;
    Lbox_   = pars.Lbox;
    dim_    = pars.dim;
    norm_   = pars.norm;    // Might create a header entry for this 
    nthr_   = pars.nthr;
    ds_     = pars.dt;
    nsteps_ = pars.nsteps;
    verb_   = pars.verb;

    fft_backend_ = std::make_unique<FFTWOpenMPBackend>
                   (dim_, N_, Lbox_, pars.plan, verb_, nthr_);

    sites_  = fft_backend_->sites();
    ksites_ = fft_backend_->ksites();

    psi_  = fftw_alloc_complex(sites_);
    Vhat_ = fftw_alloc_complex(ksites_);
    V_    = fftw_alloc_real(sites_);

    if (!V_ || !psi_ || !Vhat_ )
        throw std::bad_alloc();

    if (verb_)
        printMemoryUsage();
}

Field::Field()
    : N_(0), dim_(0), nthr_(0), Lbox_(0.0), ds_(0.0), sites_(0), ksites_(0),
      psi_(nullptr), Vhat_(nullptr), V_(nullptr) {}     

Field::Field(const Params& p)
    : Field()
{
    init(p);
}


Field::~Field()
{
    fft_backend_.reset();
    if (psi_)  fftw_free(psi_);
    if (V_)    fftw_free(V_);
    if (Vhat_) fftw_free(Vhat_);
    fftw_cleanup_threads();
}


void Field::kick(double dt)
{
    if (verb_)
        std::cout << "[kick] Starting loop ..." << std::endl;

    PROFILE(KICK);
    FLOP_COUNT(128, 32);  // ~128 flops/site, 32 bytes/site
    const double fac = -dt;

    #pragma omp parallel for simd//schedule(dynamic)
    for (size_t idx = 0; idx < sites_; ++idx)
    {
        double phase = fac * V_[idx];
        double re = psi_[idx][0];
        double im = psi_[idx][1];
        double cos_p = std::cos(phase);
        double sin_p = std::sin(phase);
        psi_[idx][0] = re * cos_p - im * sin_p;
        psi_[idx][1] = re * sin_p + im * cos_p;
    }
    
    if (verb_)
        std::cout << "[kick] done!" << std::endl;
}


void Field::drift(double dt)
{
    PROFILE(DRIFT);
    FLOP_COUNT(128, 32);

    const double fac = -0.5 * dt;
    const double dk  = 2.0 * M_PI / Lbox_;
    const int hN     = N_ / 2;

    if (verb_)
        std::cout << "[drift] Applying c2c forward ..." << std::endl;

    {
        PROFILE(FFT)
        FLOP_COUNT(0, 32);
        fft_backend_->forward_c2c(psi_);
    }

    if (verb_)
        std::cout << "[drift] Starting drift loop ..." << std::endl;

    #pragma omp parallel for
    for (size_t idx = 0; idx < sites_; ++idx)
    {
        double kx = 0.0, ky = 0.0, kz = 0.0;

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

        double k2 = kx*kx + ky*ky + kz*kz;
        double phase = fac * k2;

        double re = psi_[idx][0];
        double im = psi_[idx][1];

        double cos_p = std::cos(phase);
        double sin_p = std::sin(phase);

        psi_[idx][0] = (re * cos_p - im * sin_p) / sites_;
        psi_[idx][1] = (re * sin_p + im * cos_p) / sites_;
    }
    
    if (verb_)
    {
        std::cout << "[drift] done!" << std::endl;
        std::cout << "[drift] Starting c2c backward ..." << std::endl;
    }

    {
        PROFILE(FFT)
        FLOP_COUNT(0, 32);
        fft_backend_->backward_c2c(psi_);
    }

    if (verb_)
        std::cout << "[drift] done!" << std::endl;
}

void Field::computeEnergy()
{
    if (verb_)
        std::cout << "[computeEnergy] Starting loop ..." << std::endl;

    #pragma omp parallel for
    for (size_t idx = 0; idx < sites_; ++idx)
    {
        double re = psi_[idx][0];
        double im = psi_[idx][1];
        V_[idx]   = re * re + im * im;
    }
    
    if (verb_)
        std::cout << "[computeEnergy] done ..." << std::endl;

    double local_max = 0.0;
    #pragma omp parallel for simd reduction(max:local_max)
    for (size_t i = 0; i < sites_; ++i)
    {
        double absV = std::abs(V_[i]);
        if (absV > local_max) local_max = absV;
    }
    rhomax_ = local_max;  
}


void Field::Poisson()
{
    int hN  = N_ / 2;
    int hN1 = N_ / 2 + 1;
    const double twopi = 2.0 * M_PI / Lbox_;

    if (dim_ == 2)
    {
        double vol = N_ * N_;

        #pragma omp parallel for collapse(2) schedule(static) default(shared)
        for (int iy = 0; iy < N_; ++iy)
        {
            for (int ix = 0; ix < hN1; ++ix)
            {
                int nx = ix;  
                int ny = (iy <= hN) ? iy : iy - N_; 
                int idx = iy * hN1 + ix;

                double kx = twopi * nx; 
                double ky = twopi * ny; 
                                            
                double k2 = kx*kx + ky*ky;
                k2 = k2 + (k2 == 0.0);
                double fac = - 1 / k2 / vol;

                Vhat_[idx][0] *= fac;
                Vhat_[idx][1] *= fac;
            }
        }
    }
    else if (dim_ == 3)
    {
        double vol = N_ * N_ * N_;

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
                    
                    double kx = twopi * nx;
                    double ky = twopi * ny;
                    double kz = twopi * nz;
                                            
                    double k2 = kx*kx + ky*ky + kz*kz;
                    k2 = k2 + (k2 == 0.0);
                    double fac = - 1 / k2 / vol;

                    Vhat_[idx][0] *= fac;
                    Vhat_[idx][1] *= fac;
                }
            }
        }
    }
}


void Field::updatePotential()
{
    PROFILE(POISSON);

    const double pref = 4.0 * M_PI * norm_;

    #pragma omp parallel for simd //schedule(static)
    for (size_t i=0; i < sites_; ++i) 
    {
        double rho = psi_[i][0] * psi_[i][0] + psi_[i][1] * psi_[i][1];
        V_[i] = pref * (rho - 1.0);
    }
    
    if (verb_)
        std::cout << "[updatePotential] Starting 2rc forward ..." << std::endl;
    {
        PROFILE(FFT)
        FLOP_COUNT(0, 32);
        fft_backend_->forward_r2c(V_, Vhat_);
    }

    if (verb_)
    {
        std::cout << "[updatePotential] 2rc forward done!" << std::endl;
        std::cout << "[updatePotential] Starting Poisson ... " << std::endl;
    }

    Poisson();

    if (verb_)
    {
        std::cout << "[updatePotential] Poisson done!" << std::endl;
        std::cout << "[updatePotential] Starting c2r backward ... " << std::endl;
    }

    {
        PROFILE(FFT)
        FLOP_COUNT(0, 32);
        fft_backend_->backward_c2r(Vhat_, V_);
    }

    if (verb_)
        std::cout << "[updatePotential] c2r backward done!" << std::endl;

    double local_max = 0.0;
    #pragma omp parallel for simd reduction(max:local_max)
    for (size_t i = 0; i < sites_; ++i)
    {
        double absV = std::abs(V_[i]);
        if (absV > local_max) local_max = absV;
    }
    Vmax_ = local_max;  
}


void Field::updateTime()
{
    curr_ += 1;
}

void Field::propagate()
{
    kick(0.5 * ds_);
    drift(ds_);
    updatePotential();
    kick(0.5 * ds_);
}





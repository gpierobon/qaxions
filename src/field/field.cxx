#include <array>
#include <cmath>
#include <iostream>
#include <omp.h>

#include "field.h"
#include "../parse.h"
#include "../utils.h"
#include "../core/enum.h"
#include "../core/profiler.h"
#include "../fft/fft.h"

#ifdef USE_GPU
    #include "../fft/fft_cuda.h"
#endif

void Field::init(Params& pars)
{
    PROFILE(FIELD);

    if (psi_)  { FFTW_FREE(psi_); psi_   = nullptr; }
    if (V_)    { FFTW_FREE(V_);     V_   = nullptr; }
    if (Vhat_) { FFTW_FREE(Vhat_); Vhat_ = nullptr; }

    N_      = pars.N;
    Lbox_   = pars.Lbox;
    dim_    = pars.dim;
    
    nthr_   = pars.nthr;
    nsteps_ = pars.nsteps;
    verb_   = pars.verb;

    setCosmo(pars);

#ifdef USE_GPU
    fft_backend_ = makeCUFFTBackend(dim_, N_, verb_);
#else
    fft_backend_ = std::make_unique<FFTWOpenMPBackend>
                   (dim_, N_, pars.plan, verb_, nthr_);
#endif

    sites_  = fft_backend_->sites();
    ksites_ = fft_backend_->ksites();

    psi_  = FFTW_ALLOC_C(sites_);
    Vhat_ = FFTW_ALLOC_C(ksites_);
    V_    = FFTW_ALLOC_R(sites_);

    if (!V_ || !psi_ || !Vhat_ )
        throw std::bad_alloc();

    if (verb_)
        printMemoryUsage();

#ifdef USE_GPU
    freeGPU();
    allocGPU();
#endif
}

Field::Field()
    : psi_(nullptr), Vhat_(nullptr), V_(nullptr),
#ifdef USE_GPU
      d_psi_(nullptr), d_Vhat_(nullptr), d_V_(nullptr),
#endif
      Lbox_(0.0), ds_(0.0), N_(0), dim_(0), nthr_(0), sites_(0), ksites_(0),
      fft_backend_(nullptr) {}

Field::Field(Params& p)
    : Field()
{
    init(p);
}


Field::~Field()
{

#ifdef USE_GPU
    freeGPU();
#endif
    fft_backend_.reset();
    if (psi_)  fftw_free(psi_);
    if (V_)    fftw_free(V_);
    if (Vhat_) fftw_free(Vhat_);
    fftw_cleanup_threads();

}

void Field::setCosmo(Params& p)
{
    CosmoType ctype = p.cosmotype;
    switch (ctype)
    {
        case CosmoType::STATIC:
            cosmo_ = 0;
            a_ = 1.0;
            s_ = 0;
            ds_ = p.dt;
            rho_mean_ = 1.0;
            norm_ = 4 * M_PI * p.norm;
            break;

        case CosmoType::MRE:
            cosmo_ = 1;
            a_ = p.ai;
            ds_ = p.dt/a_;
            rho_mean_ = 1.5 * (2.0 * M_PI) * (2.0 * M_PI) * 1.27 * 1.27;
            norm_ = 1.0;
            //s_ = getTime(a_); // TO IMPLEMENT
            break;
    }
}

void Field::kick(double dt)
{
    PROFILE(KICK);

    if (verb_)
        std::cout << "[kick] Starting loop ..." << std::endl;

#ifdef USE_GPU
    kick_gpu(dt);
#else
    kick_cpu(dt);
#endif
    
    if (verb_)
        std::cout << "[kick] done!" << std::endl;
}

void Field::drift(double dt)
{
    PROFILE(DRIFT);
    
    if (verb_)
        std::cout << "[drift] Applying c2c forward ..." << std::endl;

    fft_forward_c2c();

    if (verb_)
        std::cout << "[drift] Starting drift loop ..." << std::endl;

#ifdef USE_GPU
    drift_k2_gpu(dt);
#else
    drift_k2_cpu(dt);
#endif

    if (verb_)
    {
        std::cout << "[drift] done!" << std::endl;
        std::cout << "[drift] Starting c2c backward ..." << std::endl;
    }

    fft_backward_c2c();

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
        Real re = psi_[idx][0];
        Real im = psi_[idx][1];
        V_[idx] = re * re + im * im;
    }
    
    if (verb_)
        std::cout << "[computeEnergy] done ..." << std::endl;

    Real local_max = 0.0;
    Real local_sum = 0.0;
    #pragma omp parallel for reduction(max:local_max) reduction(+:local_sum)
    for (size_t i = 0; i < sites_; ++i)
    {
        Real absV = std::abs(V_[i]);
        if (absV > local_max) local_max = absV;
        local_sum += absV;
    }
    Real avg = local_sum / static_cast<double>(sites_);
    rhomax_ = local_max / avg;  
}

void Field::updatePotential()
{
#ifdef USE_GPU
    updatePotential_gpu();
#else
    updatePotential_cpu();
#endif
}


// --------------------------------------
// FFT wrappers
// --------------------------------------
void Field::fft_forward_c2c()
{
    PROFILE(FFT);
#ifdef USE_GPU
    fft_backend_->forward_c2c(reinterpret_cast<Complex*>(d_psi_));
#else
    fft_backend_->forward_c2c(psi_);
#endif
}


void Field::fft_backward_c2c()
{
    PROFILE(FFT);
#ifdef USE_GPU
    fft_backend_->backward_c2c(reinterpret_cast<Complex*>(d_psi_));
#else
    fft_backend_->backward_c2c(psi_);
#endif
}


void Field::fft_forward_r2c()
{
    PROFILE(FFT);
#ifdef USE_GPU
    fft_backend_->forward_r2c(reinterpret_cast<Real*>(d_V_),
                              reinterpret_cast<Complex*>(d_Vhat_));
#else
    fft_backend_->forward_r2c(V_, Vhat_);
#endif
}


void Field::fft_backward_c2r()
{
    PROFILE(FFT);
#ifdef USE_GPU
    fft_backend_->backward_c2r(reinterpret_cast<Complex*>(d_Vhat_),
                               reinterpret_cast<Real*>(d_V_));
#else
    fft_backend_->backward_c2r(Vhat_, V_);
#endif
}


void Field::updateTime()
{
    // Get ctype??
    curr_ += 1; 

    if (cosmo_ == 1)
    {
        ds_ = 0.00025/a_;
        a_  = a_ * (1 + 1.27 * 1.27 * std::sqrt(1 + a_ ) * ds_);
    }
    
    s_ += ds_;
}





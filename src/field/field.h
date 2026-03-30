#ifndef FIELD_H
#define FIELD_H

#include <fftw3.h>
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <complex>
#include "../parse.h"
#include "../fft/fft.h"
#include "../core/types.h"


#ifdef USE_GPU
#include <cuda_runtime.h>
#include <cufft.h>
#endif

class Field 
{
    public:

        void init(Params& pars);
        void setCosmo(Params& pars);

        Field();
        explicit Field(Params& pars);
        ~Field();

        void kick(double dt);
        void kick_cpu(double dt);
        void kick_gpu(double dt);

        void drift(double dt);
        void drift_k2_cpu(double dt);
        void drift_k2_gpu(double dt);

        void Poisson_cpu();
        void Poisson_gpu();
        void updatePotential();
        void updatePotential_cpu();
        void updatePotential_gpu();

        void computeEnergy();
        std::vector<Real> computeProjection();
        void updateTime();

        const Complex* psi() const { return psi_; }   
        Complex*       psi()       { return psi_; } 
        Real getPsiIdx(int idx) const { return psi_[idx][0]; }
        Real getPsiImagIdx(int idx) const { return psi_[idx][1]; }

        const Real* V() const { return V_; }
        Real*       V()       { return V_; }
        Real getVIdx(int idx) const { return V_[idx]; }

        Complex* Vhat()     { return Vhat_; }

#ifdef USE_GPU
        CuComplex* devicePsi()  { return d_psi_;  }
        CuReal*    deviceV()    { return d_V_;    }
        CuComplex* deviceVhat() { return d_Vhat_; }
#endif
        
        FFTBackend* fftBackend() const { return fft_backend_.get(); }

        int size() const { return N_; }
        int dim()  const { return dim_; }
        double Lbox() const { return Lbox_; }

        size_t sites()  const { return sites_; }
        size_t ksites() const { return ksites_; }

        Real Vmax() const { return Vmax_; }
        Real rhomax() const { return rhomax_; }
        Real rho_mean() const { return rho_mean_; }
        
        bool   verb()  const { return verb_;}
        bool   gpu_active()  const { return gpu_active_;}
        int   curr()  const { return curr_;}
        double norm()  const { return norm_;}
        double time()  const { return s_;}
        double ds()  const { return ds_;}
        double a()  const { return a_;}
        int   nsteps()  const { return nsteps_;}
        int   cosmo()  const { return cosmo_;}
        
        void fft_forward_c2c();
        void fft_backward_c2c();
        void fft_forward_r2c();
        void fft_backward_c2r();

#ifdef USE_GPU
        void allocGPU();
        void freeGPU();
        void toDevice(); // Host to device
        void toHost();   // Device to host
        void syncVhatToHost(); // For spectrum
        void syncVToDevice(); // For spectrum
#endif


    private:
        Complex*   psi_   = nullptr;
        Complex*   Vhat_  = nullptr;
        Real*      V_     = nullptr;

#ifdef USE_GPU
        CuComplex* d_psi_  = nullptr;
        CuComplex* d_Vhat_ = nullptr;
        CuReal*    d_V_    = nullptr;
#endif
        
        double Lbox_;
        double norm_;
        double ds_;
        double s_;
        double a_;
        
        Real Vmax_ = 0.0;
        Real rhomax_ = 0.0;
        Real rho_mean_;

        int    N_;
        int    dim_;
        int    nthr_;
        int    cosmo_;
        int    curr_   = 0;
        int    nsteps_;
        bool   verb_;
        bool   gpu_active_ = false;
        size_t sites_ = 0;
        size_t ksites_ = 0;

        std::unique_ptr<FFTBackend> fft_backend_;
};


#endif

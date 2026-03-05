#ifndef FIELD_H
#define FIELD_H

#include <fftw3.h>
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <complex>
#include "parse.h"
#include "fft/fft.h"

class Field 
{
    public:

        void init(Params& pars);
        void setCosmo(Params& pars);

        Field();
        explicit Field(Params& pars);
        ~Field();

        void kick(double dt);
        void drift(double dt);
        double get_kSq(int idx); 
        void Poisson();
        void updatePotential();
        void computeEnergy();
        void updateTime();
        void half_kick();
        void full_kick();
        void drift_update();
        void propagate();

        const fftw_complex* psi() const { return psi_; }   
        fftw_complex*       psi()       { return psi_; } 
        double getPsiIdx(int idx) const { return psi_[idx][0]; }
        double getPsiImagIdx(int idx) const { return psi_[idx][1]; }

        const double* V() const { return V_; }
        double*       V()       { return V_; }
        double getVIdx(int idx) const { return V_[idx]; }

        fftw_complex* Vhat()     { return Vhat_; }
        
        FFTBackend* fftBackend() const { return fft_backend_.get(); }

        int size() const { return N_; }
        int dim()  const { return dim_; }
        double Lbox() const { return Lbox_; }

        size_t sites()  const { return sites_; }
        size_t ksites() const { return ksites_; }

        double Vmax() const { return Vmax_; }
        double rhomax() const { return rhomax_; }
        double rho_mean() const { return rho_mean_; }
        bool   verb()  const { return verb_;}
        int   curr()  const { return curr_;}
        double time()  const { return s_;}
        double a()  const { return a_;}
        int   nsteps()  const { return nsteps_;}
        int   cosmo()  const { return cosmo_;}


    private:
        
        int N_;                 
        int dim_;                 
        int nthr_;
        double Lbox_;
        double norm_;
        double rho_mean_;
        
        bool verb_;

        int curr_ = 0;
        int nsteps_ = 0;

        int cosmo_;
        double ds_;           // Code time step
        double s_;            // s-time
        double a_;            // Scale factor

        size_t sites_ = 0;
        size_t ksites_ = 0;

        double Vmax_ = 0.0;
        double rhomax_ = 0.0;

        fftw_complex* psi_;
        fftw_complex* Vhat_;
        double*       V_;      
        
        std::unique_ptr<FFTBackend> fft_backend_;
};


#endif

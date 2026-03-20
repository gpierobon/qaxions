#ifndef FFT_H
#define FFT_H

#include <memory>
#include <fftw3.h>
#include "../parse.h"
#include "../core/enum.h"
#include "../core/types.h"

class FFTBackend
{
    public:
        virtual ~FFTBackend() = default;

        virtual void forward_c2c (Complex* data) = 0;
        virtual void backward_c2c(Complex* data) = 0;
        virtual void forward_r2c (Real* in_real, Complex* out_complex) = 0;
        virtual void backward_c2r(Complex* in_complex, Real* out_real) = 0;

        virtual size_t sites()  const = 0; 
        virtual size_t ksites() const = 0;

        virtual bool owns_zero_mode() const   { return true; }
        virtual bool owns_zero_mode_k() const { return true; }

        virtual ptrdiff_t global_N() const = 0; 
};

class FFTWOpenMPBackend : public FFTBackend
{
    private:
        int dim_;
        size_t N_;
        int nthr_;
        unsigned plan_flags_;
        size_t sites_;
        size_t ksites_;

        FFTW_PLAN plan_fwd_c2c_ = nullptr;
        FFTW_PLAN plan_bwd_c2c_ = nullptr;
        FFTW_PLAN plan_fwd_r2c_ = nullptr;
        FFTW_PLAN plan_bwd_c2r_ = nullptr;

        unsigned int setPlan(FFTPlanType type) const
        {
            switch (type)
            {
                case FFTPlanType::ESTIMATE: return FFTW_ESTIMATE;
                case FFTPlanType::MEASURE: return FFTW_MEASURE;
                case FFTPlanType::PATIENT: return FFTW_PATIENT;
                case FFTPlanType::EXHAUSTIVE: return FFTW_EXHAUSTIVE;
            }
            return FFTW_ESTIMATE;
        }

    public:
        FFTWOpenMPBackend(int dim, size_t N, FFTPlanType plan_type, 
                          bool verbose, int nthr)
        : dim_(dim), N_(N), nthr_(nthr), plan_flags_(setPlan(plan_type))
        {
            if (dim_ != 2 && dim_ != 3)
                throw std::invalid_argument("Dimension must be 2 or 3");
            
            sites_  = (dim_ == 3) ? N_ * N_ * N_ : N_ * N_;
            ksites_ = (dim_ == 3) ? N_ * N_  * (N_/2 + 1) : N_ * (N_/2 + 1);

            if (fftw_init_threads() == 0) 
                throw std::runtime_error("FFTW threads initialization failed");
            
            FFTW_PLAN_WITH_NTHREADS(nthr_);
            
            // Allocate dummy arrays, FFTW requires aligned memory for plans
            Complex* dpsi = (Complex*)FFTW_MALLOC(sizeof(Complex) * sites_);
            Complex* dhat = (Complex*)FFTW_MALLOC(sizeof(Complex) * ksites_);
            Real*   dreal = (Real*)   FFTW_MALLOC(sizeof(Real)    * sites_);

            if (!dpsi || !dreal || !dhat) 
                throw std::bad_alloc();

            if (verbose)
            {
                std::cout << "Creating FFTW plans with: ";
                switch (plan_type) {
                    case FFTPlanType::ESTIMATE: std::cout << "ESTIMATE\n"; break;
                    case FFTPlanType::MEASURE: std::cout << "MEASURE\n"; break;
                    case FFTPlanType::PATIENT: std::cout << "PATIENT\n"; break;
                    case FFTPlanType::EXHAUSTIVE: std::cout << "EXHAUSTIVE\n"; break;
                }
            }

            size_t n = static_cast<int>(N_);
            if (dim_ == 3)
            {
                plan_fwd_c2c_ = FFTW_PLAN_DFT_3D(n, n, n, dpsi, dpsi, FFTW_FORWARD, plan_flags_);
                plan_bwd_c2c_ = FFTW_PLAN_DFT_3D(n, n, n, dpsi, dpsi, FFTW_BACKWARD, plan_flags_);
                plan_fwd_r2c_ = FFTW_PLAN_DFT_R2C_3D(n, n, n, dreal, dhat, plan_flags_);
                plan_bwd_c2r_ = FFTW_PLAN_DFT_C2R_3D(n, n, n, dhat, dreal, plan_flags_);
            }
            else
            {
                plan_fwd_c2c_ = FFTW_PLAN_DFT_2D(n, n, dpsi, dpsi, FFTW_FORWARD, plan_flags_);
                plan_bwd_c2c_ = FFTW_PLAN_DFT_2D(n, n, dpsi, dpsi, FFTW_BACKWARD, plan_flags_);
                plan_fwd_r2c_ = FFTW_PLAN_DFT_R2C_2D(n, n, dreal, dhat, plan_flags_);
                plan_bwd_c2r_ = FFTW_PLAN_DFT_C2R_2D(n, n, dhat, dreal, plan_flags_);
            }

            if (!plan_fwd_c2c_ || !plan_bwd_c2c_ || !plan_fwd_r2c_ || !plan_bwd_c2r_) 
                throw std::runtime_error("Failed to create FFTW plan");

            FFTW_FREE(dpsi);
            FFTW_FREE(dreal);
            FFTW_FREE(dhat);
        }

        ~FFTWOpenMPBackend() override
        {
            FFTW_DESTROY_PLAN(plan_fwd_c2c_);
            FFTW_DESTROY_PLAN(plan_bwd_c2c_);
            FFTW_DESTROY_PLAN(plan_fwd_r2c_);
            FFTW_DESTROY_PLAN(plan_bwd_c2r_);
        }

        void forward_c2c(Complex* data) override
        {
            FFTW_EXECUTE_DFT(plan_fwd_c2c_, data, data);
        }

        void backward_c2c(Complex* data) override
        {
            FFTW_EXECUTE_DFT(plan_bwd_c2c_, data, data);
        }

        void forward_r2c(Real* in_real, Complex* out_complex) override
        {
            FFTW_EXECUTE_DFT_R2C(plan_fwd_r2c_, in_real, out_complex);
        }

        void backward_c2r(Complex* in_complex, Real* out_real) override
        {
            FFTW_EXECUTE_DFT_C2R(plan_bwd_c2r_, in_complex, out_real);
        }

        size_t sites() const override { return sites_; }
        size_t ksites() const override { return ksites_; }

        bool owns_zero_mode() const override   { return true; }
        bool owns_zero_mode_k() const override { return true; }

        ptrdiff_t global_N()      const override { return static_cast<ptrdiff_t>(N_); }
};


#endif


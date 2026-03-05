
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "spectrum.h"
#include "../field.h"


Spectrum::Spectrum(Field& f)
    : N_        (f.size())
    , dim_      (f.dim())
    , Lbox_     (f.Lbox())
    , a_        (f.a())
    , rho_mean_ (f.rho_mean())
    , sites_    (f.sites())
    , psi_      (f.psi())
    , Vhat_     (f.Vhat())
    , V_        (f.V())
    , fft_backend_(f.fftBackend())
{
    if (!psi_ || !Vhat_ || !V_)
        throw std::runtime_error("PowerSpectrum: Field buffers are null.");

    const int Nmodes = N_ / 2 + 1;
    Pk_   .assign(Nmodes, 0.0);
    count_.assign(Nmodes, 0);
    k_    .assign(Nmodes, 0.0);
}

void Spectrum::compute()
{
    const int Nmodes = N_ / 2 + 1;
    std::fill(Pk_.begin(),    Pk_.end(),    0.0);
    std::fill(count_.begin(), count_.end(), 0);

    // Necessary??
    //#pragma omp parallel for simd
    //for (size_t i = 0; i < sites_; ++i)
    //{
    //    double re  = psi_[i][0];
    //    double im  = psi_[i][1];
    //    double rho = re*re + im*im;
    //    V_[i] = (rho - rho_mean_) / rho_mean_;
    //}

    fft_backend_->forward_r2c(V_, Vhat_);

    if (dim_ == 3)
        bin3D();
    else
        bin2D();

    // Average over modes in each shell
    for (int b = 0; b < Nmodes; ++b)
        if (count_[b] > 0) Pk_[b] /= count_[b];

    buildKAxis();
}


void Spectrum::bin3D()
{
    const int    hN     = N_ / 2;
    const int    hN1    = N_ / 2 + 1;
    const int    Nmodes = hN + 1;
    const double norm   = 1.0 / (static_cast<double>(sites_) *
                                  static_cast<double>(sites_));

    // Local raw arrays so OMP array-reduction compiles on all backends
    std::vector<double> pk_loc   (Nmodes, 0.0);
    std::vector<int>    cnt_loc  (Nmodes, 0);
    double* pk_raw  = pk_loc .data();
    int*    cnt_raw = cnt_loc.data();

    #pragma omp parallel for collapse(3) \
        reduction(+: pk_raw[:Nmodes], cnt_raw[:Nmodes])
    for (int iz = 0; iz < N_;  ++iz)
    for (int iy = 0; iy < N_;  ++iy)
    for (int ix = 0; ix < hN1; ++ix)
    {
        int nx = ix;
        int ny = (iy <= hN) ? iy : iy - N_;
        int nz = (iz <= hN) ? iz : iz - N_;

        int kbin = static_cast<int>(
            std::round(std::sqrt(double(nx*nx + ny*ny + nz*nz))));
        if (kbin >= Nmodes) continue;

        // Hermitian correction: ix==0 or ix==hN has no conjugate in the
        // stored half-array, every other ix appears twice
        double mult = (ix == 0 || ix == hN) ? 1.0 : 2.0;

        size_t idx = static_cast<size_t>(iz) * N_ * hN1
                   + static_cast<size_t>(iy) * hN1
                   + ix;

        double re = Vhat_[idx][0];
        double im = Vhat_[idx][1];

        pk_raw [kbin] += mult * norm * (re*re + im*im);
        cnt_raw[kbin] += static_cast<int>(mult);
    }

    for (int b = 0; b < Nmodes; ++b)
    {
        Pk_   [b] += pk_loc [b];
        count_[b] += cnt_loc[b];
    }
}


void Spectrum::bin2D()
{
    const int    hN     = N_ / 2;
    const int    hN1    = N_ / 2 + 1;
    const int    Nmodes = hN + 1;
    const double norm   = 1.0 / (static_cast<double>(sites_) *
                                  static_cast<double>(sites_));

    std::vector<double> pk_loc  (Nmodes, 0.0);
    std::vector<int>    cnt_loc (Nmodes, 0);
    double* pk_raw  = pk_loc .data();
    int*    cnt_raw = cnt_loc.data();

    #pragma omp parallel for collapse(2) \
        reduction(+: pk_raw[:Nmodes], cnt_raw[:Nmodes])
    for (int iy = 0; iy < N_;  ++iy)
    for (int ix = 0; ix < hN1; ++ix)
    {
        int nx = ix;
        int ny = (iy <= hN) ? iy : iy - N_;

        int kbin = static_cast<int>(
            std::round(std::sqrt(double(nx*nx + ny*ny))));
        if (kbin >= Nmodes) continue;

        double mult = (ix == 0 || ix == hN) ? 1.0 : 2.0;

        size_t idx = static_cast<size_t>(iy) * hN1 + ix;

        double re = Vhat_[idx][0];
        double im = Vhat_[idx][1];

        pk_raw [kbin] += mult * norm * (re*re + im*im);
        cnt_raw[kbin] += static_cast<int>(mult);
    }

    for (int b = 0; b < Nmodes; ++b)
    {
        Pk_   [b] += pk_loc [b];
        count_[b] += cnt_loc[b];
    }
}



void Spectrum::buildKAxis()
{
    const double dk = 2.0 * M_PI / Lbox_;
    for (int n = 0; n < static_cast<int>(k_.size()); ++n)
        k_[n] = n * dk;
}

// write  — ASCII output: k  P(k)  count
void Spectrum::write(const std::string& filename) const
{
    std::ofstream out(filename);
    if (!out)
        throw std::runtime_error("Spectrum::write: cannot open " + filename);

    out << "# a: " << a_ << std::endl;
    out << "# k_phys      P(k)          count\n";
    for (int n = 0; n < static_cast<int>(Pk_.size()); ++n)
    {
        if (count_[n] == 0) continue;   // skip empty shells (k=0 or corners)
        out << std::scientific
            << k_[n]     << "  "
            << Pk_[n]    << "  "
            << count_[n] << "\n";
    }
}



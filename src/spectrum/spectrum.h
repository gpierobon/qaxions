#ifndef SPEC_H
#define SPEC_H

#include <vector>
#include <string>
#include <fftw3.h>

// Forward declaration — avoids pulling in all of Field.hh
class Field;

// Usage:
//   PowerSpectrum ps(field);
//   ps.compute();                        // run at current time-step
//   ps.write("pk_step0042.dat");         // write k  P(k)  count
//   auto& Pk = ps.Pk();                  // access raw binned values
// ---------------------------------------------------------------------------
class Spectrum
{
    public:
        // Construct from a live Field.  No data is copied; pointers are borrowed.
        explicit Spectrum(Field& field);

        // Compute P(k) from the current field state.  Safe to call repeatedly.
        void compute();

        // Write two-column ASCII output:  k_phys  P(k)  count
        // filename: path to output file (will be overwritten if it exists)
        void write(const std::string& filename) const;

        const std::vector<double>& Pk()     const { return Pk_;    }
        const std::vector<int>&    counts() const { return count_; }
        const std::vector<double>& kmodes() const { return k_;     }

    private:
        int             N_;
        int             dim_;
        double          Lbox_;
        double          a_;
        double          rho_mean_;
        size_t          sites_;
        fftw_complex*   psi_;     // read-only view of Field::psi_
        fftw_complex*   Vhat_;    // reused as scratch (borrowed from Field)
        double*         V_;       // reused as scratch (borrowed from Field)

        //class FFTWOpenMPBackend* fft_backend_;
        class FFTBackend* fft_backend_;

        // Output arrays (size = N/2 + 1)
        std::vector<double> Pk_;
        std::vector<int>    count_;
        std::vector<double> k_;

        void bin3D();
        void bin2D();
        void buildKAxis();
};

#endif

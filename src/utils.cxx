#include <iomanip>
#include <vector>
#include <sstream>
#include <iostream>
#include "utils.h"
#include "field/field.h"

std::vector<size_t> generateMeasList(size_t num_steps, size_t n_bins)
{
    std::vector<size_t> bins;
    bins.reserve(n_bins);

    if (n_bins == 0 || num_steps == 0)
        return bins;

    if (n_bins == 1) {
        bins.push_back(num_steps - 1);
        return bins;
    }

    for (size_t k = 0; k < n_bins; ++k)
    {
        size_t i_bin = k * (num_steps - 1) / (n_bins - 1);
        bins.push_back(i_bin);
    }

    return bins;
}

std::string formatDuration(std::chrono::duration<double> dur)
{
    using namespace std::chrono;

    double seconds = dur.count();
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    if (seconds < 1.0)
    {
        int ms = static_cast<int>(seconds * 1000);
        out << ms << "ms";
    }
    else if (seconds < 60.0)
    {
        out << seconds << "s";
    }
    else if (seconds < 3600.0)
    {
        int mins = static_cast<int>(seconds) / 60;
        double rem = seconds - mins * 60;
        out << mins << "m " << rem << "s";
    }
    else
    {
        int hrs  = static_cast<int>(seconds) / 3600;
        int mins = (static_cast<int>(seconds) % 3600) / 60;
        double rem = seconds - hrs * 3600 - mins * 60;
        out << hrs << "h " << mins << "m " << rem << "s";
    }

    return out.str();
}

void printStatus(const Field& f, int step, size_t num_steps, int meas, 
                 std::chrono::duration<double> dur)
{
    double percent = 100.0 * step / num_steps;
    std::string time_str = formatDuration(dur);

    double rhomax = f.rhomax();
    double a = f.a();

    std::cout << "============================================================="
              << "=====================================\n";
    std::cout << std::fixed    <<  std::setprecision(1)
              << std::setw(6)  << "Meas #"    << std::setw(4) << meas 
              << std::setw(2)  << " |   a: "
              << std::setw(8) << std::setprecision(4) << a
              << std::setw(2)  << " |   max(δ): "
              << std::setw(12) << std::setprecision(5) << rhomax
              << std::setw(4)  << "    |  "        << std::setw(6) << std::setprecision(1) << percent << "%"
              << std::setw(12) << "Walltime:"  << " " << std::setw(12) << time_str << std::endl;
    std::cout << "============================================================="
              << "=====================================\n";
}


void printMemoryUsage()
{
    std::ifstream file("/proc/self/status");
    std::string line;

    std::cout << "Memory allocated!" << std::endl;
    while (std::getline(file, line))
    {
        if (line.find("VmPeak:") != std::string::npos)
            std::cout << line << std::endl;
    }
}

void printParams(const Field& f, const Params& p)
{
    auto icToString = [](ICType ic)
    {
        switch (ic)
        {
            case ICType::SOLITONS: return "SOLITONS";
            case ICType::SPECTRUM: return "SPECTRUM";
            default: return "UNKNOWN";
        }
    };
    
    auto fftToString = [](FFTPlanType p)
    {
        switch (p)
        {
            case FFTPlanType::ESTIMATE:   return "ESTIMATE";
            case FFTPlanType::MEASURE:    return "MEASURE";
            case FFTPlanType::PATIENT:    return "PATIENT";
            case FFTPlanType::EXHAUSTIVE: return "EXHAUSTIVE";
            default: return "UNKNOWN";
        }
    };

    auto measToString = [](uint32_t m) -> std::string
    {
        if (m == 0) return "NONE";

        std::string result;
        const std::pair<MeasureType, const char*> flags[] =
        {
            { MeasureType::SPECTRUM,  "SPECTRUM"  },
            { MeasureType::RHO_MAX,   "RHO_MAX"   },
            { MeasureType::RHO_SLICE, "RHO_SLICE" },
            { MeasureType::RHO_GRID,  "RHO_GRID"  },
            { MeasureType::PSI_GRID,  "PSI_GRID"  },
        };

        for (auto& [flag, name] : flags)
        {
            if (m & static_cast<uint32_t>(flag))
            {
                if (!result.empty()) 
                    result += " | ";
                result += name;
            }
        }

        return result.empty() ? "UNKNOWN" : result;
    };

    auto cosmoToString = [](CosmoType ctype)
    {
        switch (ctype)
        {
            case CosmoType::STATIC: return "STATIC";
            case CosmoType::MRE: return "MRE";
            default: return "UNKNOWN";
        }
    };

    std::cout << " " << std::endl; 
    std::cout << "--------------------------------------------------\n";
    std::cout << "                    RUN PARAMETERS                \n";
    std::cout << "--------------------------------------------------\n";

    std::cout << "Grid:\n";
    std::cout << "  dim            = " << f.dim() << "\n";
    std::cout << "  N              = " << f.size() << "\n";
    std::cout << "  Lbox           = " << f.Lbox() << "\n\n";

    std::cout << "Physics:\n";
    std::cout << "  Cosmo type     = " << cosmoToString(p.cosmotype) << "\n";
    std::cout << "  dt (init)      = " << f.ds() << "\n";
    std::cout << "  nsteps         = " << p.nsteps << "\n";
    if (f.cosmo() == 1)
        std::cout << "  a_i            = " << p.ai << "\n";

    std::cout << "  norm           = " << f.norm() << "\n";
    std::cout << "  IC type        = " << icToString(p.ictype) << "\n";
    if (p.ictype == ICType::SPECTRUM)
        std::cout << "  IC file        = " << p.pk_file << "\n";
    std::cout << "  IC seed        = " << p.seed << "\n\n";

    std::cout << "Measurement:\n";
    std::cout << "  meas flag(s)   = " << measToString(static_cast<uint32_t>
                                                       (p.measinfo)) << "\n";
    std::cout << "  # meas         = " << p.nmeas << "\n\n";

    std::cout << "FFT:\n";
#ifdef USE_GPU
    std::cout << "  Backend        = cuFFT " << std::endl;
#else 
    std::cout << "  Backend        = FFTW " << std::endl;
    std::cout << "  plan           = " << fftToString(p.plan) << "\n";
    std::cout << "  # threads      = " << p.nthr << "\n\n";
#endif

    std::cout << "IO:\n";
    std::cout << "  output dir     = " << p.dir << "\n";
    std::cout << "  verbose        = " << (p.verb  ? "true" : "false") << "\n";

    std::cout << "--------------------------------------------------\n";
    std::cout << " " << std::endl; 
}


void printHelp()
{
    std::cout << R"(
    qaxions - 3D solver for axion dark matter

    Usage:
      ./qaxions [options]

    Options:
      --dim    <int>     Grid dimension (default: 3)
      --N      <int>     Grid size (default: 64)
      --nthr   <int>     Number of threads per process (default: 1)
      --ai     <float>   Initial scale factor (default: 0.1)
      --norm   <float>   Poisson's equation normalisation
      --dt     <float>   Time step
      --steps  <int>     Number of time steps
      --nmeas  <int>     Number of measurements
      --meas   <int>     Type of measurements (use --measinfo)
      --t      <float>   Final time
      --fft    <int>     FFTW plan: estimate (0) | measure (1) | patient | exhaustive
      --help          Show help
    )";
}


void printBanner()
{
    std::cout << "\033[1;96m";
    std::cout << R"(
          _ \                _|
         |   |   _` | \ \  /  |   _ \   __ \    __|
         |   |  (   |  `  <   |  |   |  |   | \__ \
        \__\_\ \__,_|  _/\_\ _| \___/  _|  _| ____/

    )" << std::endl;
    std::cout << "\033[0m";
}



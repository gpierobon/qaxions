#include <iomanip>
#include <vector>
#include <sstream>
#include <iostream>
#include "utils.h"
#include "field.h"

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

    std::cout << "============================================================="
              << "=========================\n";
    std::cout << std::fixed    <<  std::setprecision(1)
              << std::setw(6)  << "Meas #"    << std::setw(4) << meas 
              << std::fixed    <<  std::setprecision(5)
              << std::setw(2)  << " |   max(ρ): "
              << std::setw(12) << std::setprecision(5) << rhomax
              << std::setw(4)  << "    |  "        << std::setw(6) << std::setprecision(1) << percent << "%"
              << std::setw(12) << "Walltime:"  << " " << std::setw(12) << time_str << std::endl;
    std::cout << "============================================================="
              << "=========================\n";
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
            // case ICType::SPECTRUM: return "SPECTRUM";
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

    std::cout << " " << std::endl; 
    std::cout << "--------------------------------------------------\n";
    std::cout << "                    RUN PARAMETERS                \n";
    std::cout << "--------------------------------------------------\n";

    std::cout << "  dim            = " << f.dim() << "\n";
    std::cout << "  N              = " << f.size() << "\n";
    std::cout << "  Lbox           = " << f.Lbox() << "\n\n";

    std::cout << "  dt             = " << p.dt << "\n";
    std::cout << "  dtr            = " << p.dtr << "\n";
    std::cout << "  nsteps         = " << p.nsteps << "\n";
    std::cout << "  ai             = " << p.ai << "\n\n";

    std::cout << "  norm           = " << p.norm << "\n";
    std::cout << "  IC type        = " << icToString(p.ictype) << "\n\n";

    std::cout << "FFT:\n";
    std::cout << "  plan           = " << fftToString(p.plan) << "\n";
    std::cout << "  threads        = " << p.nthr << "\n\n";

    std::cout << "IO:\n";
    std::cout << "  output dir     = " << p.dir << "\n";
    std::cout << "  verbose        = " << (p.verb  ? "true" : "false") << "\n";

    std::cout << "--------------------------------------------------\n";
    std::cout << " " << std::endl; 
}

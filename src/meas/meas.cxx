#include "meas.h"
#include "../core/enum.h"
#include "../utils.h"
#include "../io/io.h"
#include "../spectrum/spectrum.h"


MeasureType parseMeasureType(uint32_t value)
{
    return static_cast<MeasureType>(value);
}

void measure(Field& f, const Params& p, size_t measNumber, Clock::time_point st)
{
    // Status printing
    MeasureType meas = p.measinfo;

    if (meas == MeasureType::NONE)
        return;

#ifdef USE_GPU
    // only sync if device has valid data
    if (f.gpu_active())
        f.toHost();
#endif

    int curr_step = f.curr();
    int nsteps = f.nsteps();
    bool verb = f.verb();

    auto dur = Clock::now() - st;

    if (verb)
        std::cout << "\n\n";

    printStatus(f, curr_step, nsteps, measNumber, dur);

    f.computeEnergy();

    // Scalar/slice measurements as meas_*.h5
    const MeasureType smeas = MeasureType::RHO_MAX | 
                              MeasureType::RHO_SLICE;
    if (meas & smeas)
    {
        std::ostringstream fname;
        fname << p.dir << "/meas_"
              << std::setw(4) << std::setfill('0') << measNumber << ".h5";
        IO io(fname.str());

        std::vector<Real> slice;
        bool write_slice  = meas & MeasureType::RHO_SLICE;
        bool write_rhomax = meas & MeasureType::RHO_MAX;

        if (write_slice)
            slice = f.computeProjection();

        io.writeMeas(f, slice, write_slice, write_rhomax);
        io.close();
    }

    // Spectrum files in output/spec
    if (meas & MeasureType::SPECTRUM)
    {
        std::filesystem::path odir = p.dir;
        std::filesystem::path spec_dir = odir / "spec";

        if (!std::filesystem::create_directory(spec_dir))
        {
            if (!std::filesystem::exists(spec_dir))
                throw std::runtime_error("Failed to create directory: " + \
                                          spec_dir.string());
        }

        if (verb)
            std::cout << "[measure] Saving power spectrum" << std::endl;
            
        Spectrum spec(f);
        spec.compute();
        std::ostringstream fname;
        fname << spec_dir.string() << "/spec_"
              << std::setw(4) << std::setfill('0') << measNumber << ".txt";
        spec.write(fname.str());
    }

    // Save energy or psi grid as output/field_*.h5
    if (meas & (MeasureType::RHO_GRID | MeasureType::PSI_GRID))
    {
        bool writePsi = meas & MeasureType::PSI_GRID;
        if (verb)
        {
            if (writePsi)
                std::cout << "[measure] Saving psi grid" << std::endl;
            else
                std::cout << "[measure] Saving energy grid" << std::endl;
        }
        std::ostringstream fname;
        fname << p.dir << "/field_"
              << std::setw(4) << std::setfill('0') << measNumber << ".h5";
        IO io(fname.str());
        io.writeConf(f, writePsi);
        io.close();
    }

    if (verb)
        std::cout << "\n\n";
}



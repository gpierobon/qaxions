#include "enum.h"
#include "meas.h"
#include "utils.h"
#include "io/io.h"
#include "spectrum/spectrum.h"


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

    int curr_step = f.curr();
    int nsteps = f.nsteps();
    bool verb = f.verb();

    auto dur = Clock::now() - st;

    if (verb)
        std::cout << "\n\n";

    printStatus(f, curr_step, nsteps, measNumber, dur);

    f.computeEnergy();
    
    if (meas & MeasureType::SPECTRUM)
    {
        if (verb)
            std::cout << "[measure] Saving power spectrum" << std::endl;
            
        Spectrum spec(f);
        spec.compute();
        std::ostringstream fname;
        fname << p.dir << "/spec_"
              << std::setw(4) << std::setfill('0') << measNumber << ".txt";
        spec.write(fname.str());
    }

    // Save energy grid
    if (meas & MeasureType::RHO_GRID)
    {
        if (verb)
            std::cout << "[measure] Saving energy grid" << std::endl;
        std::ostringstream fname;
        fname << p.dir << "/field_"
              << std::setw(4) << std::setfill('0') << measNumber << ".h5";

        IO io(fname.str());
        io.writeConf(f, false);
        io.close();
    }

    if (verb)
        std::cout << "\n\n";
}



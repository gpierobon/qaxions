#include <fftw3.h>
#include <hdf5.h>
#include <math.h>
#include <omp.h>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "parse.h"
#include "utils.h"
#include "field.h"
#include "io/io.h"
#include "ic/ic.h"
#include "profiler.h"




int main( int argc, char* argv[] )
{   
    // Start the clock
    Clock::time_point start = Clock::now();
    
    // Pass parameters
    Params pars = init(argc, argv);
    
    // Start profilers 
    initProfilers();

    // Create Field pointer
    std::unique_ptr<Field> field;

    // ICs, read or create
    if (pars.readj)
    {
        field = std::make_unique<Field>();
        IO reader("jaxions_2D.hdf5", FileMode::ReadOnly);
        reader.readJaxions(pars, *field);
        std::cout << "\nField created, it took " << timeSince(start) << std::endl;
        std::cout << "Restart successful. Grid: N=" << field->size()
                  << ", dim=" << field->dim()
                  << ", sites=" << field->sites() << "\n";
    }
    else
    {
        field = std::make_unique<Field>(pars);
        auto ic = createIC(pars.ictype, pars);
        ic->apply(*field);
        std::cout << "\nField created, it took " << timeSince(start) << std::endl;
    }

    printParams(*field, pars);
    
    // Set measurement list
    size_t measn = 0;
    size_t next_meas = 0;
    size_t nsteps = static_cast<size_t>(pars.nsteps);
    size_t nmeas  = static_cast<size_t>(pars.nmeas);
    std::vector<size_t> mlist = generateMeasList(nsteps, nmeas);

    // Dump ICs
    std::ostringstream icf;
    icf << pars.dir <<  "/field_0000.h5";
    field->computeEnergy();
    auto dur = Clock::now() - start;
    printStatus(*field, 0, nsteps, measn, dur);
    IO io(icf.str());
    io.writeConf(*field, true);
    io.close();

    // Set the potential
    field->updatePotential();

    for (size_t idx=0; idx<nsteps; ++idx) 
    {
        field->propagate();

        if (next_meas < mlist.size() && idx == mlist[next_meas])
        {
            ++measn; ++next_meas;
            auto dur = Clock::now() - start;
            printStatus(*field, idx, nsteps, measn, dur);

            std::ostringstream fname;
            fname << pars.dir <<  "/field_" 
                  << std::setw(4) << std::setfill('0') << measn << ".h5";

            field->computeEnergy();
            IO io_tmp(fname.str());
            io_tmp.writeConf(*field);
        }
    }
    
    std::cout << "\nSimulation complete. " 
              << nmeas << " outputs saved" << std::endl;
    std::cout << "Finished: it took " 
              << timeSince(start) << std::endl;  

    printProfStats(pars, start);

    return 0;
}


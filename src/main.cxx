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
#include "field/field.h"
#include "meas/meas.h"
#include "io/io.h"
#include "ic/ic.h"
#include "spectrum/spectrum.h"
#include "core/profiler.h"
#include "propagator/propagator.h"




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
    field = std::make_unique<Field>(pars);
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);
    std::cout << "\nField created, it took " << timeSince(start) << "\n\n";

    printParams(*field, pars);
    
    // Set measurement list
    size_t measn = 0;
    size_t next_meas = 0;
    size_t nsteps = static_cast<size_t>(pars.nsteps);
    size_t nmeas  = static_cast<size_t>(pars.nmeas);
    std::vector<size_t> mlist = generateMeasList(nsteps, nmeas);

    // Meas 0: on host
    measure(*field, pars, 0, start); 

    // Transfer to device once after ICs and step-0 measurement
#ifdef USE_GPU
    field->toDevice();
#endif

    // Initial potential before time loop
    field->updatePotential();

    std::cout << "\nStarting time loop ... \n" << std::endl;
    
    half_kick(*field); // Offset kicks
    for (size_t idx = 0; idx < nsteps; ++idx)
    {
        drift_update(*field);
        
        // measure after drift (positions at integer time)
        if (next_meas < mlist.size() && idx == mlist[next_meas])
        {
            ++measn; ++next_meas;
            measure(*field, pars, measn, start); // GPU-aware
        }

        // full kick except maybe last step
        if (idx != nsteps - 1)
            full_kick(*field);
    }
    half_kick(*field); // Final kick
    
    // Final sync to host
#ifdef USE_GPU
    field->toHost();
#endif

    std::cout << "\nSimulation complete. " 
              << nmeas << " outputs saved" << std::endl;
    std::cout << "Finished: it took " 
              << timeSince(start) << std::endl;  

    printProfStats(pars, start);

    return 0;
}


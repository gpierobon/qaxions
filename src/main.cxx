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
#include "meas.h"
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

    measure(*field, pars, 0, start); 
    field->updatePotential();

    std::cout << "\nStarting time loop ... \n" << std::endl;
    for (size_t idx=0; idx<nsteps; ++idx) 
    {
        field->propagate();

        if (next_meas < mlist.size() && idx == mlist[next_meas])
        {
            ++measn; ++next_meas;
            auto dur = Clock::now() - start;
            measure(*field, pars, measn, start); 
        }
    }
    
    std::cout << "\nSimulation complete. " 
              << nmeas << " outputs saved" << std::endl;
    std::cout << "Finished: it took " 
              << timeSince(start) << std::endl;  

    printProfStats(pars, start);

    return 0;
}


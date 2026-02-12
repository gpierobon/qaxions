#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "../field.h"
#include "../parse.h"
#include "../profiler.h"
#include "../ic/ic.h"

namespace py = pybind11;

double test_kick(int n, int N, double dt, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = n;
    pars.norm = 1;
    pars.verb = false;

    initProfilers(); // Crashes without
                  
    auto field = std::make_unique<Field>(pars); 
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    field->updatePotential();
    field->kick(dt);

    return field->getPsiIdx(idx);
}

double test_drift(int n, int N, double dt, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = n;
    pars.norm = 1;
    pars.verb = false;

    initProfilers(); // Crashes without
                  
    auto field = std::make_unique<Field>(pars);
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    field->drift(dt);

    return field->getPsiIdx(idx);
}


double test_potential(int n, int N, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = n;
    pars.norm = 1;
    pars.verb = false;

    initProfilers(); // Crashes without
                  
    auto field = std::make_unique<Field>(pars); 
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    field->updatePotential();

    return field->getVIdx(idx);
}

double test_kick_drift_step(int n, int N, double dt, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = n;
    pars.norm = 1;
    pars.verb = false;

    initProfilers(); // Crashes without
                  
    auto field = std::make_unique<Field>(pars);
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    field->updatePotential();

    field->kick(0.5 * dt);
    field->drift(dt);
    field->updatePotential();
    field->kick(0.5 * dt);

    return field->getPsiIdx(idx);
}


PYBIND11_MODULE(fieldtest, m)
{
    m.def("test_kick", &test_kick, "");
    m.def("test_drift", &test_drift, "");
    m.def("test_potential", &test_potential, "");
    m.def("test_kick_drift_step", &test_kick_drift_step, "");
}


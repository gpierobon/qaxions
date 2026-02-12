#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../field.h"
#include "../parse.h"
#include "../profiler.h"
#include "../ic/ic.h"

double test_solitons(int N, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = 2;
    pars.norm = 1;
    pars.verb = false;

    initProfilers();
                  
    auto field = std::make_unique<Field>(pars); 
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    return field->getPsiIdx(idx);
}

double test_solitons_3D(int N, int idx)
{
    Params pars;
    defaults(pars);
    pars.N = N;
    pars.dim = 3;
    pars.norm = 1;
    pars.verb = false;

    initProfilers();
                  
    auto field = std::make_unique<Field>(pars); 
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field);

    return field->getPsiIdx(idx);
}



PYBIND11_MODULE(ictest, m)
{
    m.def("test_solitons", &test_solitons, "");
    m.def("test_solitons_3D", &test_solitons_3D, "");
}

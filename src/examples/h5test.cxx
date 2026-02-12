#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "../field.h"
#include "../parse.h"
#include "../profiler.h"
#include "../io/io.h"
#include "../ic/ic.h"

namespace py = pybind11;


double test_read_write(int idx, const std::string& fname)
{
    Params pars;
    defaults(pars);
    pars.verb = false;

    initProfilers();

    auto field1 = std::make_unique<Field>(pars);
    auto ic = createIC(pars.ictype, pars);
    ic->apply(*field1);

    const double real1 = field1->getPsiIdx(idx);
    const double imag1 = field1->getPsiImagIdx(idx);

    {
        IO writer(fname, FileMode::Create);
        writer.writeConf(*field1, true);
        writer.flush(); 
    }

    Field field2;
    Params pars2 = pars;

    {
        IO reader(fname, FileMode::ReadOnly);
        reader.readConf(pars2, field2);
    }

    const double real2 = field2.getPsiIdx(idx);
    const double imag2 = field2.getPsiImagIdx(idx);
    double diff_real = std::abs(real1 - real2);
    double diff_imag = std::abs(imag1 - imag2);
    double max_diff  = std::max(diff_real, diff_imag);

    return max_diff;
}

double test_read_jaxions(int idx)
{
    Params pars;
    defaults(pars);
    pars.verb = false;
    
    initProfilers(); // Crashes without
    
    auto field = std::make_unique<Field>();

    IO reader("jaxions_2D.hdf5", FileMode::ReadOnly);
    reader.readJaxions(pars, *field);
    return field->getPsiIdx(idx);
}


PYBIND11_MODULE(h5test, m)
{
    m.def("test_read_write", &test_read_write, "");
    m.def("test_read_jaxions", &test_read_jaxions, "");
}

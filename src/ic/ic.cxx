#include "ic.h"
#include "solitons.h"
#include "ic_spectrum.h"
#include "ic_read.h"
#include "ic_jaxions.h"

std::unique_ptr<InitialCondition> createIC(ICType type, Params& p)
{
    switch (type)
    {
        case ICType::SOLITONS:      return std::make_unique<SolitonsIC>(p);
        case ICType::SPECTRUM:      return std::make_unique<SpectrumIC>(p);
        case ICType::READCONF:      return std::make_unique<ReadConfIC>(p);
        case ICType::JAXIONS:       return std::make_unique<JaxionsIC>(p);
        default:
            throw std::runtime_error("Unknown ICType");
    }
}

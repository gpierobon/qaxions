#include "ic.h"
#include "solitons.h"

std::unique_ptr<InitialCondition> createIC(ICType type, const Params& p)
{
    switch (type)
    {
        case ICType::SOLITONS:      return std::make_unique<SolitonsIC>(p);
        //case ICType::SPECTRUM:      return std::make_unique<SpectrumIC>(p);
        default:
            throw std::runtime_error("Unknown ICType");
    }
}

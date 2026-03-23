#ifndef SPEC_IC_H
#define SPEC_IC_H

#include "ic.h"

class SpectrumIC : public InitialCondition
{
    public:
        explicit SpectrumIC(const Params& p) : p_(p) {}
        void apply(Field& field) const override;
        std::string name() const override { return "spectrum"; }
    private:
        const Params& p_;
};

#endif

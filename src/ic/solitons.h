#ifndef SOL_IC_H
#define SOL_IC_H

#include "ic.h"

class SolitonsIC : public InitialCondition
{
    public:
        explicit SolitonsIC(const Params& p) : p_(p) {}
        void apply(Field& field) const override;
        std::string name() const override { return "solitons"; }
    private:
        const Params& p_;
};

#endif

#ifndef JAX_IC_H
#define JAX_IC_H

#include "ic.h"

class JaxionsIC : public InitialCondition
{
    public:
        explicit JaxionsIC(Params& p) : p_(&p) {}
        void apply(Field& field) const override;
        std::string name() const override { return "jaxions"; }
    private:
        Params* p_;
};

#endif

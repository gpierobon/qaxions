#ifndef READ_IC_H
#define READ_IC_H

#include "ic.h"

class ReadConfIC : public InitialCondition
{
    public:
        explicit ReadConfIC(Params& p) : p_(&p) {}
        void apply(Field& field) const override;
        std::string name() const override { return "readConf"; }
    private:
        Params* p_;
};

#endif

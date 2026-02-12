#ifndef IC_H
#define IC_H

#include "../field.h"
#include "../parse.h"
#include <memory>
#include <string>

class InitialCondition
{
    public:
        virtual ~InitialCondition() = default;
        virtual void apply(Field& field) const = 0;
        virtual std::string name() const = 0;
};

// Factory
std::unique_ptr<InitialCondition> createIC(ICType type, const Params& p);

#endif

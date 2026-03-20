#include "propagator.h"


void half_kick(Field& f)
{
    if (f.verb())
        std::cout << "Half-kick!" << std::endl;
    f.kick(0.5 * f.ds());
}

void full_kick(Field& f)
{
    f.kick(f.ds());
    if (f.verb())
        std::cout << "==========================================" 
                  << "==============\n" << std::endl;
}

void drift_update(Field& f)
{
    if (f.verb())
        std::cout << "\n[Step: " << f.curr() << "] =================" 
                  << "============================= " << std::endl;
    f.drift(f.ds());
    f.updateTime();
    f.updatePotential();
}

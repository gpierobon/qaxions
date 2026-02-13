#ifndef __MEAS_H
#define __MEAS_H

#include "enum.h"
#include "utils.h"


inline MeasureType operator|(MeasureType a, MeasureType b)
{
    return static_cast<MeasureType>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline MeasureType& operator|=(MeasureType& a, MeasureType b)
{
    a = a | b;
    return a;
}

inline bool operator&(MeasureType a, MeasureType b)
{
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}


MeasureType parseMeasureType(uint32_t value);

void measure(Field& f, const Params& p,
             size_t measNumber, Clock::time_point st);

#endif

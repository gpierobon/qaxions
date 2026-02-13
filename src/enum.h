#ifndef ENUM_H
#define ENUM_H

#include <cstdint>

enum class ICType
{
    SOLITONS,
    SPECTRUM
};


enum class FFTPlanType
{
    ESTIMATE,   
    MEASURE,    
    PATIENT,
    EXHAUSTIVE  
};


enum class ProfType
{
    FFT,
    IO,
    IC,
    FIELD,
    DRIFT,
    KICK,
    POISSON
};


enum class MeasureType : uint32_t
{
    NONE        = 0,
    SPECTRUM    = 1 << 0,  // 1
    RHO_MAX     = 1 << 1,  // 2
    RHO_SLICE   = 1 << 2,  // 4
    RHO_GRID    = 1 << 3,  // 8
    PSI_GRID    = 1 << 4   // 16
};


#endif

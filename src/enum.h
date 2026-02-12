#ifndef ENUM_H
#define ENUM_H


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


#endif

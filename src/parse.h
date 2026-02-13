#ifndef PARSE_H
#define PARSE_H

#include <iostream>
#include <cstring>
#include <filesystem>
#include "enum.h"


typedef struct
{
    int N;
    int dim;
    int nthr;

    double ai;
    double Lbox;
    double dtr;
    double dt;
    int nsteps;
    int nmeas;
    double norm;

    bool verb;
    bool readj;

    std::string dir;

    ICType ictype;
    FFTPlanType plan; 
    MeasureType measinfo;

} Params;

void defaults(Params& p);
void printHelp();
void parseArgs(int argc, char* argv[], Params* p);
void setDir(Params* p);
void printBanner();
Params init(int argc, char* argv[]);


#endif

#include <omp.h>
#include "parse.h"
#include "meas/meas.h"
#include <unordered_map>


void defaults(Params& pars)
{
    pars.N = 64;
    pars.dim = 3;
    pars.nthr = 1;
    pars.ai = 0.1;
    pars.Lbox = 1.0;
    pars.dtr = 4.0;
    pars.ai = 0.1;
    pars.dt = 0.0001;
    pars.nsteps = 10;
    pars.nmeas = 20;
    pars.norm = 4000;
    pars.sol_bkg = 1.0;
    pars.verb = false;
    pars.readj = false;
    pars.seed = 9;

    pars.dir = "output";
    pars.pk_file = "Pk.txt";

    pars.plan = FFTPlanType::ESTIMATE;
    pars.ictype = ICType::SOLITONS;      // To change into SPECTRUM
    pars.cosmotype = CosmoType::STATIC;
    pars.measinfo = MeasureType::NONE;
}


void parseArgs(int argc, char* argv[], Params* pars)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i]; 
        if      (arg == "--N"      && i+1 < argc) { pars->N       = atoi(argv[++i]); }
        if      (arg == "--nthr"   && i+1 < argc) { pars->nthr    = atoi(argv[++i]); }
        else if (arg == "--ai"     && i+1 < argc) { pars->ai      = atof(argv[++i]); }
        else if (arg == "--norm"   && i+1 < argc) { pars->norm    = atof(argv[++i]); }
        else if (arg == "--L"      && i+1 < argc) { pars->Lbox    = atof(argv[++i]); }
        else if (arg == "--dt"     && i+1 < argc) { pars->dt      = atof(argv[++i]); }
        else if (arg == "--steps"  && i+1 < argc) { pars->nsteps  = atoi(argv[++i]); }
        else if (arg == "--nmeas"  && i+1 < argc) { pars->nmeas   = atoi(argv[++i]); }
        else if (arg == "--seed"   && i+1 < argc) { pars->seed    = atoi(argv[++i]); }
        else if (arg == "--dir"    && i+1 < argc) { pars->dir     = argv[++i]; }
        else if (arg == "--pkfile" && i+1 < argc) { pars->pk_file = argv[++i]; }
        else if (arg == "--verb"               ) { pars->verb   = true; }
        else if (arg == "--readj"              ) { pars->readj  = true; }
        else if (arg == "--meas"               )
        {
            pars->measinfo = parseMeasureType(atoi(argv[++i]));
        }
        else if (arg == "--dim"   && i+1 < argc)
        {
            pars->dim = atoi(argv[++i]);
            if (pars->dim != 2 && pars->dim != 3) {
                std::cerr << "Dimension must be 2 or 3!! ( --dim <2/3> )\n";
                std::exit(1);
            }
        }
        else if (arg == "--ic" && i+1 < argc)
        {
            std::string s = argv[++i];
            const std::unordered_map<std::string, ICType> map = {
                {"solitons", ICType::SOLITONS},
                {"spectrum", ICType::SPECTRUM}
            };
            auto it = map.find(s);
            if (it != map.end())
                pars->ictype = it->second;
            else
            {
                std::cerr << "Unknown IC: " << s << "\n";
                printHelp();
                std::exit(1);
            }
        }
        else if (arg == "--fft" && i+1 < argc)
        {
            int val = std::atoi(argv[++i]);
            if (val >= 0 && val <= 3)
                pars->plan = static_cast<FFTPlanType>(val);
            else
            {
                std::cerr << "Invalid --fft value: " << val << " (must be 0-3)\n";
                std::exit(1);
            }
        }
        else if (arg == "--cosmo" && i+1 < argc)
        {
            int val = std::atoi(argv[++i]);
            if (val >= 0 && val <= 1)
                pars->cosmotype = static_cast<CosmoType>(val);
            else
            {
                std::cerr << "Invalid --cosmo value: " << val << " (must be 0-1)\n";
                std::exit(1);
            }
        }
        else if (arg == "--help") { printHelp(); std::exit(0); }
    } // end for
}


void setDir(Params* pars)
{
    std::filesystem::path out_dir = pars->dir;
    if (std::filesystem::exists(out_dir))
    {
        std::filesystem::remove_all(out_dir);
        std::filesystem::create_directory(out_dir);
    }
    else
        std::filesystem::create_directory(out_dir);
}



Params init(int argc, char* argv[])
{
    Params params;
    defaults(params);
    parseArgs(argc, argv, &params); 
    printBanner();
    setDir(&params);
    
    omp_set_num_threads(params.nthr);
    bool verb = params.verb;

    if (verb)
        std::cout << "Using " << params.nthr << " threads\n";

    return params;
}

#include <omp.h>
#include "parse.h"
#include "meas.h"
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
    pars.verb = false;
    pars.readj = false;

    pars.dir = "output";

    pars.ictype = ICType::SOLITONS,
    pars.plan = FFTPlanType::ESTIMATE;
    pars.measinfo = MeasureType::NONE;

}

void printHelp()
{
    std::cout << R"(
    qaxions - 3D solver for axion dark matter

    Usage:
      ./qaxions [options]

    Options:
      --dim    <int>     Grid dimension (default: 3)
      --N      <int>     Grid size (default: 64)
      --nthr   <int>     Number of threads per process (default: 1)
      --ai     <float>   Initial scale factor (default: 0.1)
      --norm   <float>   Poisson's equation normalisation
      --dt     <float>   Time step
      --steps  <int>     Number of time steps
      --nmeas  <int>     Number of measurements
      --meas   <int>     Type of measurements (use --measinfo)
      --t      <float>   Final time
      --fft    <int>     FFTW plan: estimate (0) | measure (1) | patient | exhaustive
      --help          Show help
    )";
}

void parseArgs(int argc, char* argv[], Params* pars)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if      (arg == "--N"     && i+1 < argc) { pars->N      = atoi(argv[++i]); }
        if      (arg == "--nthr"  && i+1 < argc) { pars->nthr   = atoi(argv[++i]); }
        else if (arg == "--ai"    && i+1 < argc) { pars->ai     = atof(argv[++i]); }
        else if (arg == "--norm"  && i+1 < argc) { pars->norm   = atof(argv[++i]); }
        else if (arg == "--L"     && i+1 < argc) { pars->Lbox   = atof(argv[++i]); }
        else if (arg == "--dt"    && i+1 < argc) { pars->dt     = atof(argv[++i]); }
        else if (arg == "--steps" && i+1 < argc) { pars->nsteps = atoi(argv[++i]); }
        else if (arg == "--nmeas" && i+1 < argc) { pars->nmeas  = atoi(argv[++i]); }
        else if (arg == "--dir"   && i+1 < argc) { pars->dir    = argv[++i]; }
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
                {"solitons", ICType::SOLITONS}//,
                //{"spectrum", ICType::SPECTRUM}
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


void printBanner()
{
    std::cout << "\033[1;96m";
    std::cout << R"(
          _ \                _|
         |   |   _` | \ \  /  |   _ \   __ \    __|
         |   |  (   |  `  <   |  |   |  |   | \__ \
        \__\_\ \__,_|  _/\_\ _| \___/  _|  _| ____/

    )" << std::endl;
    std::cout << "\033[0m";
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

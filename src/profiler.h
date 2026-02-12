#ifndef	PROFILER_H
#define	PROFILER_H

#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "utils.h"
#include "parse.h"


struct FlopCounter
{
    double flops = 0.0;
    double bytes = 0.0;
    double time  = 0.0;

    void add(double f, double b, double t) { flops += f; bytes += b; time += t; }
    void reset() { flops = bytes = time = 0.0; }

    double GFlops() const { return time > 0 ? flops / time * 1e-9 : 0.0; }
    double GBs()    const { return time > 0 ? bytes / time * 1e-9 : 0.0; }
    double secs()   const { return time; }
};


class ScopedTimer
{
    Clock::time_point start_;
    double& target_time_;
    public:
        ScopedTimer(double& t) : start_(Clock::now()), target_time_(t) {}
        ~ScopedTimer() {
            target_time_ += std::chrono::duration<double>(Clock::now() - start_).count();
        }
};


class Profiler
{
    std::string name_;
    std::map<std::string, FlopCounter> counters_;

    public:
        explicit Profiler(std::string name) : name_(std::move(name)) {}

        ScopedTimer time(const std::string& section) {
            return ScopedTimer(counters_[section].time);
        }

        void add(const std::string& section, double flops, double bytes)
        {
            counters_[section].flops += flops;
            counters_[section].bytes += bytes;
        }

        std::string name() { return name_ ; }
        auto& counters() { return counters_; }
        const auto& counters() const { return counters_; }
};


inline std::map<ProfType, std::unique_ptr<Profiler>> gProfilers;

inline void initProfilers()
{
    gProfilers[ProfType::FFT]     = std::make_unique<Profiler>("FFT");
    gProfilers[ProfType::IO]      = std::make_unique<Profiler>("I/O");
    gProfilers[ProfType::IC]      = std::make_unique<Profiler>("Initial Conditions");
    gProfilers[ProfType::DRIFT]   = std::make_unique<Profiler>("Drift");
    gProfilers[ProfType::KICK]    = std::make_unique<Profiler>("Kick");
    gProfilers[ProfType::POISSON] = std::make_unique<Profiler>("Poisson");
    gProfilers[ProfType::FIELD]   = std::make_unique<Profiler>("Field");
}


inline Profiler& getProfiler(ProfType type) { return *gProfilers.at(type); }

inline void printProfStats(const Params& p, Clock::time_point sim_start)
{
    std::ofstream log(p.dir + "/profile.log");
    if (!log) {
        std::cerr << "Failed to open profile.log\n";
        return;
    }

    double total_profiled = 0.0;

    // Avoid double-counting (e.g. FFT profiler)
    for (auto type : { ProfType::KICK,
                       ProfType::DRIFT,
                       ProfType::POISSON,
                       ProfType::IO,
                       ProfType::IC,
                       ProfType::FIELD
                      })
    {
        double prof_time = 0.0;
        for (const auto& [name, c]: gProfilers[type]->counters()) {
            prof_time += c.secs();
        }
        total_profiled += prof_time;
    }

    for (auto& [type, prof] : gProfilers)
    {
        log << "\n--- Profiler: " << prof->name() << " ----------------------\n";
        log << std::left
            << std::setw(20) << "Section"
            << std::setw(15) << "Time (s)"
            << std::setw(15) << "GFlops"
            << std::setw(15) << "GB/s"
            << "\n";
        for (const auto& [name, c] : prof->counters())
        {
            log << std::left
                << std::setw(20) << name
                << std::setw(15) << std::fixed << std::setprecision(6) << c.secs()
                << std::setw(15) << std::fixed << std::setprecision(3) << c.GFlops()
                << std::setw(15) << std::fixed << std::setprecision(3) << c.GBs()
                << "\n";
        }
        log << "-------------------------------------------------------------\n";
    }

    double fft_time = 0.0;
    double fft_perc = 0.0;
    
    if (gProfilers.count(ProfType::FFT))
    {
        for (const auto& [name, c] : gProfilers[ProfType::FFT]->counters())
            fft_time += c.secs();
    }

    if (total_profiled > 1e-9)
        fft_perc = (fft_time / total_profiled) * 100.0;


    double total_runtime = std::chrono::duration<double>(Clock::now() - sim_start).count();
    double unaccounted = total_runtime - total_profiled;

    log << "\n=== Runtime Summary =========================================\n\n";
    log << "Total runtime:       " << std::fixed << std::setprecision(6) << total_runtime << " s\n";
    log << "Profiled time:       " << total_profiled << " s\n";
    log << "Unaccounted time:    " << unaccounted << " s ";
    if (total_runtime > 0)
        log << "(" << (unaccounted/total_runtime*100) << "%)\n";
    log << "FFT dominance:       " << std::fixed << std::setprecision(1) 
        << fft_perc << "% \n";
    log << "\n";
}


// Macros to use
#define PROFILE(section) \
    auto __prof_start = std::chrono::high_resolution_clock::now(); \
    double __prof_flops = 0.0; \
    double __prof_bytes = 0.0; \
    const char* __prof_name = #section; \
    auto& __prof_profiler = getProfiler(ProfType::section); \
    auto& __prof_counter = __prof_profiler.counters()[__prof_name]; \
    struct __prof_defer { \
        double* flops; \
        double* bytes; \
        std::chrono::time_point<std::chrono::high_resolution_clock> start; \
        FlopCounter* counter; \
        ~__prof_defer() { \
            auto end = std::chrono::high_resolution_clock::now(); \
            double time = std::chrono::duration<double>(end - start).count(); \
            counter->flops += *flops; \
            counter->bytes += *bytes; \
            counter->time += time; \
        } \
    } __prof_defer_inst = { &__prof_flops, &__prof_bytes, __prof_start, &__prof_counter };


#define ADD_FLOPS(flops, bytes) \
    __prof_flops += (flops); \
    __prof_bytes += (bytes);

#define FLOP_COUNT(flops_per_site, bytes_per_site) \
    ADD_FLOPS((flops_per_site) * N_*N_*N_, (bytes_per_site) * N_*N_*N_)


#endif 


#ifndef _UTILS_H__
#define _UTILS_H__

#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>
#include "field.h"

using Clock = std::chrono::high_resolution_clock;

void printStatus(const Field& f, int step, size_t num_steps, int meas, 
                 std::chrono::duration<double> dur);
std::vector<size_t> generateMeasList(size_t num_steps, size_t n_bins);

std::string formatDuration(std::chrono::duration<double> dur);
inline std::string timeSince(Clock::time_point start) {
    return formatDuration(Clock::now() - start);
}
void printMemoryUsage();
void printParams(const Field& f, const Params& p);

#endif 

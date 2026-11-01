#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

#include "core/NonPreemptiveScheduler.h"
#include "io/CsvHandler.h"

namespace {
double peakRssMiB() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) return -1;
    return static_cast<double>(counters.PeakWorkingSetSize) / (1024 * 1024);
#else
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0) return -1;
#ifdef __APPLE__
    return static_cast<double>(usage.ru_maxrss) / (1024 * 1024);
#else
    return static_cast<double>(usage.ru_maxrss) / 1024;
#endif
#endif
}

std::uint64_t checksum(const ScheduleResult& result) {
    std::uint64_t hash = 14695981039346656037ULL;
    const auto mix = [&](long long value) {
        hash = (hash ^ static_cast<std::uint64_t>(value)) * 1099511628211ULL;
    };
    for (const auto& row : result.processes) {
        mix(row.pid); mix(row.completionTime); mix(row.turnaroundTime);
        mix(row.waitingTime); mix(row.responseTime);
    }
    for (const auto& segment : result.gantt) {
        mix(segment.pid); mix(segment.startTime); mix(segment.endTime);
    }
    return hash;
}
} // namespace

int main(int argc, char** argv) {
    int runs = 5;
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: benchmark_np input/perf_1000000.csv [runs=5]\n";
        return 1;
    }
    if (argc == 3) {
        const std::string argument = argv[2];
        const auto parsed = std::from_chars(argument.data(), argument.data() + argument.size(), runs);
        if (parsed.ec != std::errc{} || parsed.ptr != argument.data() + argument.size() || runs < 1) {
            std::cerr << "runs must be a positive integer\n";
            return 1;
        }
    }
    std::vector<Process> input;
    std::string error;
    if (!CsvHandler::readProcesses(argv[1], input, error)) {
        std::cerr << error << '\n';
        return 1;
    }
    std::cout << "processes,run,milliseconds,peak_process_rss_mib,checksum\n";
    double total = 0;
    std::vector<double> times;
    for (int run = 1; run <= runs; ++run) {
        const auto start = std::chrono::steady_clock::now();
        const auto result = NonPreemptiveScheduler::schedule(input);
        const auto end = std::chrono::steady_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(end - start).count();
        times.push_back(ms);
        total += ms;
        // CSV parsing, Metrics, hashing, output and result destruction are outside the timer.
        std::cout << input.size() << ',' << run << ',' << std::fixed << std::setprecision(3)
                  << ms << ',' << peakRssMiB() << ',' << checksum(result) << '\n';
    }
    std::sort(times.begin(), times.end());
    std::cerr << std::fixed << std::setprecision(3) << "mean_ms=" << total / runs
              << " median_ms=" << (times[(runs - 1) / 2] + times[runs / 2]) / 2
              << " peak_process_rss_mib=" << peakRssMiB() << '\n';
}

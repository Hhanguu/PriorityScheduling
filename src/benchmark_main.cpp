#include "core/PreemptiveScheduler.h"
#include "io/CsvHandler.h"
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    if (argc != 2) {
        // No built-in default: benchmark data is generated, not committed, so any
        // hard-coded path would point at a file that does not exist on a fresh clone.
        std::cerr << "Usage: benchmark_runner <file.csv>\n"
                     "  Generate input first: ./build/generate_perf --out input --sizes 100000\n"
                     "  Then:                 ./build/benchmark_runner input/perf_100000.csv\n";
        return 1;
    }

    std::vector<Process> processes;
    std::string error;
    if (!CsvHandler::readProcesses(argv[1], processes, error)) {
        std::cerr << "Lỗi đọc file: " << error << "\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    ScheduleResult result = PreemptiveScheduler::schedule(processes);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Số tiến trình (processes): " << processes.size() << "\n";
    std::cout << "Thời gian thực thi PreemptiveScheduler: " << duration << " ms (" << duration / 1000.0 << " s)\n";
    return 0;
}

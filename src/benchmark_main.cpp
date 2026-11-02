#include "core/PreemptiveScheduler.h"
#include "io/CsvHandler.h"
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    std::string path = "samples/perf_100000.csv";
    if (argc > 1) {
        path = argv[1];
    }
    std::vector<Process> processes;
    std::string error;
    if (!CsvHandler::readProcesses(path, processes, error)) {
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

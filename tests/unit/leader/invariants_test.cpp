#include "doctest.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Process.h"
#include "core/ScheduleResult.h"

namespace {

std::vector<std::vector<long long>> readCsv(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    std::getline(file, line); // bỏ header

    std::vector<std::vector<long long>> rows;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::stringstream stream(line);
        std::string cell;
        std::vector<long long> row;
        while (std::getline(stream, cell, ',')) {
            row.push_back(std::stoll(cell));
        }
        rows.push_back(row);
    }
    return rows;
}

ScheduleResult loadResult(const std::string& resultFile, const std::string& ganttFile) {
    ScheduleResult result;
    for (const auto& r : readCsv(resultFile)) {
        result.processes.push_back({static_cast<int>(r[0]), r[1], r[2], r[3], r[4]});
    }
    for (const auto& r : readCsv(ganttFile)) {
        result.gantt.push_back({static_cast<int>(r[0]), r[1], r[2]});
    }
    return result;
}

std::vector<Process> loadInput(const std::string& path) {
    std::vector<Process> input;
    for (const auto& r : readCsv(path)) {
        input.push_back({static_cast<int>(r[0]), r[1], r[2], static_cast<int>(r[3])});
    }
    return input;
}

void checkInvariants(const std::vector<Process>& input, const ScheduleResult& result) {
    REQUIRE(result.processes.size() == input.size());
    if (input.empty()) {
        CHECK(result.gantt.empty());
        return;
    }

    long long previousEnd = 0;
    int previousPid = 0;
    std::unordered_map<int, long long> firstStart;

    for (const GanttSegment& segment : result.gantt) {
        CHECK(segment.startTime == previousEnd); // I2: kín, không hở/chồng
        CHECK(segment.endTime > segment.startTime);
        CHECK(segment.pid != previousPid);       // I3: đã gộp đoạn liền kề
        CHECK(segment.pid >= -1);                // I4: -1 = IDLE
        if (!firstStart.count(segment.pid)) {
            firstStart[segment.pid] = segment.startTime;
        }
        previousEnd = segment.endTime;
        previousPid = segment.pid;
    }

    long long maxCompletion = 0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        const ProcessResult& out = result.processes[i];
        CHECK(out.pid == input[i].pid); // I1: đúng thứ tự input
        CHECK(out.turnaroundTime == out.completionTime - input[i].arrivalTime);
        CHECK(out.waitingTime == out.turnaroundTime - input[i].burstTime);
        CHECK(out.responseTime == firstStart.at(out.pid) - input[i].arrivalTime);
        maxCompletion = std::max(maxCompletion, out.completionTime);
    }
    CHECK(previousEnd == maxCompletion);

    CHECK(result.averageWaitingTime == doctest::Approx(0.0)); // I5
    CHECK(result.averageTurnaroundTime == doctest::Approx(0.0));
    CHECK(result.averageResponseTime == doctest::Approx(0.0));
}

std::string dataPath(const std::string& name) {
    return std::string(PS_TEST_DATA_DIR) + "/" + name;
}

} // namespace

TEST_CASE("fixture NP giu bat bien") {
    checkInvariants(loadInput(dataPath("leader/sample.csv")),
                    loadResult(dataPath("ket_qua/leader/sample_np_result.csv"),
                               dataPath("ket_qua/leader/sample_np_gantt.csv")));
}

TEST_CASE("fixture P giu bat bien") {
    checkInvariants(loadInput(dataPath("leader/sample.csv")),
                    loadResult(dataPath("ket_qua/leader/sample_p_result.csv"),
                               dataPath("ket_qua/leader/sample_p_gantt.csv")));
}

TEST_CASE("input rong tra ket qua rong") {
    checkInvariants({}, ScheduleResult{});
}

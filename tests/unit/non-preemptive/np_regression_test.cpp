#include "doctest.h"

#include <algorithm>
#include <limits>
#include <random>
#include <vector>

#include "core/NonPreemptiveScheduler.h"

namespace {
// Deliberately simple O(n^2) oracle, independent of arrival sorting and FIFOs.
ScheduleResult reference(const std::vector<Process>& input) {
    ScheduleResult result;
    result.processes.resize(input.size());
    std::vector<bool> done(input.size(), false);
    long long time = 0;
    for (std::size_t completed = 0; completed < input.size();) {
        std::size_t chosen = input.size();
        long long nextArrival = std::numeric_limits<long long>::max();
        for (std::size_t i = 0; i < input.size(); ++i) {
            if (done[i]) continue;
            nextArrival = std::min(nextArrival, input[i].arrivalTime);
            if (input[i].arrivalTime > time) continue;
            if (chosen == input.size() || input[i].priority < input[chosen].priority ||
                (input[i].priority == input[chosen].priority &&
                 input[i].arrivalTime < input[chosen].arrivalTime)) chosen = i;
            // Scanning ascending indices preserves vector order for exact ties.
        }
        if (chosen == input.size()) {
            result.gantt.push_back({-1, time, nextArrival});
            time = nextArrival;
            continue;
        }
        const auto& process = input[chosen];
        const long long start = time;
        time += process.burstTime;
        result.gantt.push_back({process.pid, start, time});
        result.processes[chosen] = {process.pid, time, time - process.arrivalTime,
                                    start - process.arrivalTime, start - process.arrivalTime};
        done[chosen] = true;
        ++completed;
    }
    return result;
}

void checkAgainstReference(const std::vector<Process>& input) {
    const auto expected = reference(input);
    const auto actual = NonPreemptiveScheduler::schedule(input);
    REQUIRE(actual.processes.size() == expected.processes.size());
    REQUIRE(actual.gantt.size() == expected.gantt.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        CAPTURE(i);
        CHECK(actual.processes[i].pid == expected.processes[i].pid);
        CHECK(actual.processes[i].completionTime == expected.processes[i].completionTime);
        CHECK(actual.processes[i].turnaroundTime == expected.processes[i].turnaroundTime);
        CHECK(actual.processes[i].waitingTime == expected.processes[i].waitingTime);
        CHECK(actual.processes[i].responseTime == expected.processes[i].responseTime);
    }
    for (std::size_t i = 0; i < expected.gantt.size(); ++i) {
        CAPTURE(i);
        CHECK(actual.gantt[i].pid == expected.gantt[i].pid);
        CHECK(actual.gantt[i].startTime == expected.gantt[i].startTime);
        CHECK(actual.gantt[i].endTime == expected.gantt[i].endTime);
    }
    CHECK(actual.averageWaitingTime == 0.0);
    CHECK(actual.averageTurnaroundTime == 0.0);
    CHECK(actual.averageResponseTime == 0.0);
}
} // namespace

TEST_CASE("NP optimized FIFO matches reference for randomized arrivals and priorities") {
    std::mt19937 random(42);
    std::uniform_int_distribution<int> arrivals(0, 1000), bursts(1, 20), priorities(-20, 20);
    for (int trial = 0; trial < 200; ++trial) {
        CAPTURE(trial);
        std::vector<Process> input;
        for (int i = 0; i < trial % 101; ++i) {
            input.push_back({1000 - i, arrivals(random), bursts(random), priorities(random)});
        }
        checkAgainstReference(input);
    }
}

TEST_CASE("NP optimized FIFO handles extreme unique priorities and repeated idle gaps") {
    checkAgainstReference({{9, 0, 10, 0}, {7, 1, 1, std::numeric_limits<int>::min()},
                           {5, 1, 1, std::numeric_limits<int>::max()}, {3, 1, 1, -1},
                           {1, 1, 1, std::numeric_limits<int>::min()}, {11, 100, 1, 0},
                           {13, 200, 1, 0}});
    std::vector<Process> input;
    for (int i = 0; i < 200; ++i) input.push_back({1000 - i, 0, 1, 100 - i});
    checkAgainstReference(input);
    for (auto& process : input) process.priority = -3;
    checkAgainstReference(input);
}

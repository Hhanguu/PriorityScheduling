#include "doctest.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/NonPreemptiveScheduler.h"
#include "core/Metrics.h"

namespace {
std::vector<std::vector<long long>> readRows(const std::string& relative) {
    std::ifstream file(std::string(PS_TEST_DATA_DIR) + "/" + relative);
    REQUIRE(file.is_open());
    std::string line;
    std::getline(file, line);
    std::vector<std::vector<long long>> rows;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream stream(line);
        std::string cell;
        std::vector<long long> row;
        while (std::getline(stream, cell, ',')) row.push_back(std::stoll(cell));
        rows.push_back(row);
    }
    return rows;
}

std::vector<Process> input(const std::string& name) {
    std::vector<Process> processes;
    for (const auto& row : readRows("non-preemptive/" + name + ".csv")) {
        REQUIRE(row.size() == 4);
        processes.push_back({static_cast<int>(row[0]), row[1], row[2], static_cast<int>(row[3])});
    }
    return processes;
}

void checkOrder(const ScheduleResult& actual, const std::vector<int>& expected) {
    REQUIRE(actual.gantt.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        INFO("segment=", i, " expected pid=", expected[i], " actual pid=", actual.gantt[i].pid);
        CHECK(actual.gantt[i].pid == expected[i]);
    }
}
} // namespace

TEST_CASE("NP empty and single process") {
    const auto empty = NonPreemptiveScheduler::schedule({});
    CHECK(empty.processes.empty());
    CHECK(empty.gantt.empty());
    CHECK(empty.averageWaitingTime == 0.0);
    CHECK(empty.averageTurnaroundTime == 0.0);
    CHECK(empty.averageResponseTime == 0.0);
    const auto single = NonPreemptiveScheduler::schedule({{42, 0, 3, -2}});
    checkOrder(single, {42});
    CHECK(single.processes[0].completionTime == 3);
    CHECK(single.processes[0].turnaroundTime == 3);
    CHECK(single.processes[0].waitingTime == 0);
    CHECK(single.processes[0].responseTime == 0);
    CHECK(single.averageTurnaroundTime == 0.0);
}

TEST_CASE("NP tie uses arrival then vector position rather than pid") {
    checkOrder(NonPreemptiveScheduler::schedule(input("tie_same_arrival")),
               {30, 10, 20, 40, 50, 60, 70, 80, 90, 100});
    checkOrder(NonPreemptiveScheduler::schedule(input("tie_arrival")),
               {2, 1, 3, 4, 5, 6, 7, 8, 9, 10});
    checkOrder(NonPreemptiveScheduler::schedule({{9, 0, 5, 0}, {8, 2, 1, 1}, {7, 1, 1, 1}}),
               {9, 7, 8});
}

TEST_CASE("NP lower priority number first and no preemption") {
    checkOrder(NonPreemptiveScheduler::schedule(input("priority_order")),
               {4, 6, 3, 8, 1, 9, 5, 10, 2, 7});
    const auto actual = NonPreemptiveScheduler::schedule(input("np_no_preempt"));
    checkOrder(actual, {1, 2, 6, 10, 3, 7, 4, 8, 5, 9});
    CHECK(actual.gantt[0].startTime == 0);
    CHECK(actual.gantt[0].endTime == 20);
    CHECK(actual.processes[1].responseTime == 19);
}

TEST_CASE("NP records initial and middle IDLE") {
    const auto start = NonPreemptiveScheduler::schedule(input("idle_start"));
    REQUIRE_FALSE(start.gantt.empty());
    CHECK(start.gantt[0].pid == -1);
    CHECK(start.gantt[0].startTime == 0);
    CHECK(start.gantt[0].endTime == 5);
    const auto middle = NonPreemptiveScheduler::schedule(input("idle_middle"));
    checkOrder(middle, {1, 2, 3, 4, 5, -1, 6, 7, 8, 9, 10});
    CHECK(middle.gantt[5].startTime == 8);
    CHECK(middle.gantt[5].endTime == 20);
}

TEST_CASE("NP actual metrics and Gantt match expected CSV fixtures") {
    for (const std::string name : {"sample", "sample_20"}) {
        CAPTURE(name);
        const auto processes = input(name);
        auto actual = NonPreemptiveScheduler::schedule(processes);
        const auto expected = readRows("ket_qua/non-preemptive/" + name + "_np_result.csv");
        REQUIRE(actual.processes.size() == expected.size());
        double waiting = 0, turnaround = 0, response = 0;
        for (std::size_t i = 0; i < expected.size(); ++i) {
            CAPTURE(i);
            REQUIRE(expected[i].size() == 5);
            const auto& row = actual.processes[i];
            CHECK(row.pid == processes[i].pid);
            CHECK(row.pid == expected[i][0]);
            CHECK(row.completionTime == expected[i][1]);
            CHECK(row.turnaroundTime == expected[i][2]);
            CHECK(row.waitingTime == expected[i][3]);
            CHECK(row.responseTime == expected[i][4]);
            waiting += expected[i][3];
            turnaround += expected[i][2];
            response += expected[i][4];
        }
        CHECK(actual.averageWaitingTime == 0.0);
        CHECK(actual.averageTurnaroundTime == 0.0);
        CHECK(actual.averageResponseTime == 0.0);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(waiting / expected.size()));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(turnaround / expected.size()));
        CHECK(actual.averageResponseTime == doctest::Approx(response / expected.size()));
        const auto gantt = readRows("ket_qua/non-preemptive/" + name + "_np_gantt.csv");
        REQUIRE(actual.gantt.size() == gantt.size());
        for (std::size_t i = 0; i < gantt.size(); ++i) {
            CAPTURE(i);
            REQUIRE(gantt[i].size() == 3);
            CHECK(actual.gantt[i].pid == gantt[i][0]);
            CHECK(actual.gantt[i].startTime == gantt[i][1]);
            CHECK(actual.gantt[i].endTime == gantt[i][2]);
        }
    }
}

TEST_CASE("NP unsorted input is preserved and long long times are retained") {
    const auto processes = input("unsorted_arrival");
    const auto actual = NonPreemptiveScheduler::schedule(processes);
    REQUIRE(actual.processes.size() == processes.size());
    for (std::size_t i = 0; i < processes.size(); ++i) CHECK(actual.processes[i].pid == processes[i].pid);
    const auto large = NonPreemptiveScheduler::schedule({{1, 3000000000LL, 4000000000LL, 1}});
    CHECK(large.processes[0].completionTime == 7000000000LL);
    CHECK(large.processes[0].turnaroundTime == 4000000000LL);
}

TEST_CASE("NP leaves averages zero for Metrics to fill after scheduling") {
    for (const std::string name : {"sample", "sample_20", "tie_same_arrival", "tie_arrival",
                                   "priority_order", "np_no_preempt", "idle_start",
                                   "idle_middle", "unsorted_arrival", "np_large_100",
                                   "np_large_1000"}) {
        CAPTURE(name);
        const auto processes = input(name);
        auto actual = NonPreemptiveScheduler::schedule(processes);
        REQUIRE(actual.processes.size() == processes.size());
        CHECK(actual.averageWaitingTime == 0.0);
        CHECK(actual.averageTurnaroundTime == 0.0);
        CHECK(actual.averageResponseTime == 0.0);
        Metrics::fillAverages(actual);
        CHECK(actual.averageTurnaroundTime > 0.0);
        CHECK(actual.averageWaitingTime == doctest::Approx(actual.averageResponseTime));
    }
}
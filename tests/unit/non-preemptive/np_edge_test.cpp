#include "doctest.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <vector>

#include "core/NonPreemptiveScheduler.h"

// Edge cases for NonPreemptiveScheduler, complementing N01-N12 in
// docs/test-cases/non-preemptive.md. Each case targets a specific way the
// current implementation could break, instead of repeating existing fixtures.
namespace {

void checkPids(const ScheduleResult& actual, const std::vector<int>& expected) {
    REQUIRE(actual.gantt.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        INFO("segment=", i);
        CHECK(actual.gantt[i].pid == expected[i]);
    }
}

void checkSegment(const ScheduleResult& actual, std::size_t index, int pid, long long start,
                  long long end) {
    REQUIRE(index < actual.gantt.size());
    INFO("segment=", index);
    CHECK(actual.gantt[index].pid == pid);
    CHECK(actual.gantt[index].startTime == start);
    CHECK(actual.gantt[index].endTime == end);
}

// The Gantt must cover the time axis: each segment starts exactly when the
// previous one ends.
void checkContiguous(const ScheduleResult& actual) {
    for (std::size_t i = 1; i < actual.gantt.size(); ++i) {
        INFO("segment=", i);
        CHECK(actual.gantt[i].startTime == actual.gantt[i - 1].endTime);
    }
}

} // namespace

TEST_CASE("NP edge: every process arrives at 0, so the index decides the order") {
    // 40 processes arrive together, so no arrival time can distinguish them.
    // Order is decided entirely by (priority, index). Two conditions make this
    // case able to catch a bug: exactly 40 elements (std::sort only uses
    // insertion sort for runs <= 16) and pid counting down against the index,
    // so tie-breaking by pid differs from tie-breaking by index.
    constexpr int count = 40;
    std::vector<Process> processes;
    processes.reserve(count);
    for (int i = 0; i < count; ++i) {
        processes.push_back({count - i, 0, (i % 3) + 1, i % 4});
    }

    // Expected: priority ascending, same priority keeps index ascending.
    std::vector<std::size_t> order(count);
    for (int i = 0; i < count; ++i) order[i] = static_cast<std::size_t>(i);
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        if (processes[a].priority != processes[b].priority) {
            return processes[a].priority < processes[b].priority;
        }
        return a < b;
    });
    std::vector<int> expected;
    expected.reserve(count);
    long long totalBurst = 0;
    for (const auto index : order) {
        expected.push_back(processes[index].pid);
        totalBurst += processes[index].burstTime;
    }

    const auto actual = NonPreemptiveScheduler::schedule(processes);
    checkPids(actual, expected);
    checkContiguous(actual);
    CHECK(actual.gantt.front().startTime == 0);
    CHECK(actual.gantt.back().endTime == totalBurst);

    // No IDLE: every process is already available at t = 0.
    for (const auto& segment : actual.gantt) CHECK(segment.pid != -1);

    // process index 0 (priority 0, smallest index) runs first
    CHECK(actual.processes[0].completionTime == 1);
    CHECK(actual.processes[0].waitingTime == 0);
    CHECK(actual.processes[0].responseTime == 0);
    // process index 39 (priority 3) runs last
    CHECK(actual.processes[39].completionTime == totalBurst);
}

TEST_CASE("NP edge: a process arriving exactly when the CPU frees must run at once") {
    // P2's arrival time equals P1's completion time exactly. If the ready-queue
    // refill condition changes from <= to <, P2 is pushed back and every
    // CT/TAT/WT/RT value shifts.
    const auto actual = NonPreemptiveScheduler::schedule({{1, 0, 5, 5}, {2, 5, 1, 1}});

    checkPids(actual, {1, 2});
    checkSegment(actual, 0, 1, 0, 5);
    checkSegment(actual, 1, 2, 5, 6);

    // No zero-length IDLE segment at t = 5.
    CHECK(actual.gantt.size() == 2);
    CHECK(actual.processes[1].completionTime == 6);
    CHECK(actual.processes[1].waitingTime == 0);
    CHECK(actual.processes[1].responseTime == 0);
}

TEST_CASE("NP edge: chain of processes arriving exactly when the previous one ends") {
    // Three processes back to back with no gap: AT[1] == CT[0], AT[2] == CT[1].
    const auto actual =
        NonPreemptiveScheduler::schedule({{1, 0, 5, 2}, {2, 5, 2, 3}, {3, 7, 1, 1}});

    checkPids(actual, {1, 2, 3});
    checkContiguous(actual);
    CHECK(actual.gantt.back().endTime == 8);
    for (const auto& row : actual.processes) {
        CHECK(row.waitingTime == 0);
        CHECK(row.responseTime == 0);
    }
}

TEST_CASE("NP edge: negative priority and both int extremes compare correctly") {
    // CsvHandler only rejects priorities outside int32, so negatives reach the
    // scheduler.
    const auto actual = NonPreemptiveScheduler::schedule(
        {{1, 0, 1, INT_MIN}, {2, 0, 1, 0}, {3, 0, 1, INT_MAX}});

    checkPids(actual, {1, 2, 3});
    checkSegment(actual, 0, 1, 0, 1);
    checkSegment(actual, 2, 3, 2, 3);
    CHECK(actual.processes[2].responseTime == 2);

    // A very low priority still cannot preempt once the CPU is taken.
    const auto late = NonPreemptiveScheduler::schedule({{1, 0, 3, 0}, {2, 1, 1, INT_MIN}});
    checkPids(late, {1, 2});
    checkSegment(late, 1, 2, 3, 4);
}

TEST_CASE("NP edge: a 1e12-long IDLE stays a single segment") {
    // Arrival exactly at CsvHandler's kMaxTime bound (1e12).
    const long long arrival = 1000000000000LL;
    const auto actual = NonPreemptiveScheduler::schedule({{1, arrival, 5, 0}});

    checkPids(actual, {-1, 1});
    checkSegment(actual, 0, -1, 0, arrival);
    checkSegment(actual, 1, 1, arrival, arrival + 5);

    CHECK(actual.processes[0].completionTime == arrival + 5);
    CHECK(actual.processes[0].turnaroundTime == 5);
    CHECK(actual.processes[0].waitingTime == 0);
    CHECK(actual.processes[0].responseTime == 0);
}

TEST_CASE("NP edge: several IDLE gaps and no trailing IDLE") {
    const auto actual = NonPreemptiveScheduler::schedule(
        {{1, 0, 2, 1}, {2, 1000000, 1, 1}, {3, 2000000, 3, 1}});

    checkPids(actual, {1, -1, 2, -1, 3});
    checkSegment(actual, 0, 1, 0, 2);
    checkSegment(actual, 1, -1, 2, 1000000);
    checkSegment(actual, 2, 2, 1000000, 1000001);
    checkSegment(actual, 3, -1, 1000001, 2000000);
    checkSegment(actual, 4, 3, 2000000, 2000003);
    checkContiguous(actual);

    // The Gantt ends exactly when the last process finishes, no padding.
    CHECK(actual.gantt.back().endTime == 2000003);
    for (const auto& row : actual.processes) {
        CHECK(row.waitingTime == 0);
        CHECK(row.responseTime == 0);
    }
}

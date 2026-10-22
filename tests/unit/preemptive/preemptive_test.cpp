#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "core/PreemptiveScheduler.h"

TEST_CASE("Preemptive Priority Scheduling Tests") {
    SUBCASE("Leader sample data (sample_p)") {
        std::vector<Process> processes = {
            {1, 0, 5, 7}, {2, 1, 3, 2}, {3, 2, 8, 1}, {4, 4, 2, 4}, {5, 6, 4, 9},
            {6, 10, 1, 1}, {7, 12, 3, 8}, {8, 15, 2, 6}, {9, 18, 5, 10}, {10, 21, 2, 7}
        };
        ScheduleResult res = PreemptiveScheduler::schedule(processes);

        REQUIRE(res.processes.size() == 10);
        CHECK(res.processes[0].completionTime == 21);
        CHECK(res.processes[1].completionTime == 13);
        CHECK(res.gantt.size() == 12);
    }
}
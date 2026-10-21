#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "core/PreemptiveScheduler.h"

TEST_CASE("Preemptive Priority Scheduling") {
    SUBCASE("Sample Data from api.md (10 processes)") {
        // Dữ liệu y hệt mục 8 trong file api.md
        std::vector<Process> processes = {
            {1, 0, 5, 3}, {2, 1, 3, 2}, {3, 2, 8, 1}, {4, 4, 2, 2}, {5, 6, 4, 4},
            {6, 9, 1, 1}, {7, 12, 3, 3}, {8, 15, 2, 2}, {9, 18, 5, 5}, {10, 21, 2, 1}
        };

        ScheduleResult res = PreemptiveScheduler::schedule(processes);

        REQUIRE(res.processes.size() == 10);

        // Kiểm tra CT, TAT, WT, RT (Khớp bảng kết quả chuẩn của Leader)
        CHECK(res.processes[0].completionTime == 21); // P1
        CHECK(res.processes[0].waitingTime == 16);

        CHECK(res.processes[1].completionTime == 13); // P2
        CHECK(res.processes[1].waitingTime == 9);

        CHECK(res.processes[2].completionTime == 10); // P3
        CHECK(res.processes[2].waitingTime == 0);

        CHECK(res.processes[5].completionTime == 11); // P6
        CHECK(res.processes[5].waitingTime == 1);

        CHECK(res.processes[9].completionTime == 23); // P10
        CHECK(res.processes[9].waitingTime == 0);

        // Kiểm tra Gantt (gộp đoạn liền kề)
        REQUIRE(res.gantt.size() == 12);
        CHECK(res.gantt[0].pid == 1);  CHECK(res.gantt[0].startTime == 0);  CHECK(res.gantt[0].endTime == 1);
        CHECK(res.gantt[1].pid == 2);  CHECK(res.gantt[1].startTime == 1);  CHECK(res.gantt[1].endTime == 2);
        CHECK(res.gantt[2].pid == 3);  CHECK(res.gantt[2].startTime == 2);  CHECK(res.gantt[2].endTime == 10);
        CHECK(res.gantt[11].pid == 9); CHECK(res.gantt[11].startTime == 30); CHECK(res.gantt[11].endTime == 35);
    }
}
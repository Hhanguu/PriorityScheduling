#include "doctest.h"

#include "core/Process.h"
#include "core/ScheduleResult.h"

TEST_CASE("khung test unit chay duoc") {
    CHECK(1 + 1 == 2);
}

TEST_CASE("struct dung chung co day du truong") {
    Process p{ 1, 0, 8, 3 };
    CHECK(p.pid == 1);
    CHECK(p.arrivalTime == 0);
    CHECK(p.burstTime == 8);
    CHECK(p.priority == 3);

    ScheduleResult result;
    CHECK(result.averageWaitingTime == doctest::Approx(0.0));
    CHECK(result.averageTurnaroundTime == doctest::Approx(0.0));
    CHECK(result.averageResponseTime == doctest::Approx(0.0));
}
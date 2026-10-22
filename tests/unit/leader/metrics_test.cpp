#include "doctest.h"

#include "core/Metrics.h"
#include "core/ScheduleResult.h"

namespace {

ScheduleResult makeSampleNp() {
    ScheduleResult result;
    result.processes = {
        {1, 5, 5, 0, 0},
        {2, 17, 16, 13, 13},
        {3, 13, 11, 3, 3},
        {4, 19, 15, 13, 13},
        {5, 30, 24, 20, 20},
        {6, 14, 5, 4, 4},
        {7, 26, 14, 11, 11},
        {8, 21, 6, 4, 4},
        {9, 35, 17, 12, 12},
        {10, 23, 2, 0, 0},
    };
    return result;
}

} // namespace

TEST_CASE("rong thi ba trung binh bang 0") {
    ScheduleResult empty;
    Metrics::fillAverages(empty);
    CHECK(empty.averageWaitingTime == doctest::Approx(0.0));
    CHECK(empty.averageTurnaroundTime == doctest::Approx(0.0));
    CHECK(empty.averageResponseTime == doctest::Approx(0.0));
}

TEST_CASE("mot tien trinh thi trung binh bang chinh no") {
    ScheduleResult result;
    result.processes = {{1, 8, 8, 3, 3}};
    Metrics::fillAverages(result);
    CHECK(result.averageWaitingTime == doctest::Approx(3.0));
    CHECK(result.averageTurnaroundTime == doctest::Approx(8.0));
    CHECK(result.averageResponseTime == doctest::Approx(3.0));
}

TEST_CASE("sample NP cho 8.00 / 11.50 / 8.00") {
    ScheduleResult result = makeSampleNp();
    Metrics::fillAverages(result);
    CHECK(result.averageWaitingTime == doctest::Approx(8.0));
    CHECK(result.averageTurnaroundTime == doctest::Approx(11.5));
    CHECK(result.averageResponseTime == doctest::Approx(8.0));
}

TEST_CASE("goi lai nhieu lan cho cung ket qua") {
    ScheduleResult result = makeSampleNp();
    Metrics::fillAverages(result);
    const double first = result.averageWaitingTime;
    Metrics::fillAverages(result);
    Metrics::fillAverages(result);
    CHECK(result.averageWaitingTime == doctest::Approx(first));
    CHECK(result.averageTurnaroundTime == doctest::Approx(11.5));
}

TEST_CASE("compare doc dung ba truong trung binh") {
    ScheduleResult np = makeSampleNp();
    ScheduleResult p = makeSampleNp();
    np.averageWaitingTime = 8.0;
    np.averageTurnaroundTime = 11.5;
    np.averageResponseTime = 8.0;
    p.averageWaitingTime = 7.8;
    p.averageTurnaroundTime = 11.3;
    p.averageResponseTime = 5.3;

    const Comparison comparison = Metrics::compare(np, p);
    CHECK(comparison.waitingTime.np == doctest::Approx(8.0));
    CHECK(comparison.waitingTime.p == doctest::Approx(7.8));
    CHECK(comparison.turnaroundTime.np == doctest::Approx(11.5));
    CHECK(comparison.turnaroundTime.p == doctest::Approx(11.3));
    CHECK(comparison.responseTime.np == doctest::Approx(8.0));
    CHECK(comparison.responseTime.p == doctest::Approx(5.3));
}

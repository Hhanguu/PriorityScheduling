#include "doctest.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "core/PreemptiveScheduler.h"
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
    for (const auto& row : readRows("preemptive/" + name + ".csv")) {
        REQUIRE(row.size() == 4);
        processes.push_back({static_cast<int>(row[0]), row[1], row[2], static_cast<int>(row[3])});
    }
    return processes;
}

void checkGantt(const ScheduleResult& actual, const std::vector<std::vector<long long>>& expectedGantt) {
    REQUIRE(actual.gantt.size() == expectedGantt.size());
    for (std::size_t i = 0; i < expectedGantt.size(); ++i) {
        CAPTURE(i);
        REQUIRE(expectedGantt[i].size() == 3);
        CHECK(actual.gantt[i].pid == expectedGantt[i][0]);
        CHECK(actual.gantt[i].startTime == expectedGantt[i][1]);
        CHECK(actual.gantt[i].endTime == expectedGantt[i][2]);
    }
}
} // namespace

TEST_CASE("Preemptive Priority Scheduling Tests P01-P18") {
    SUBCASE("P15 - Empty process list") {
        auto res = PreemptiveScheduler::schedule({});
        CHECK(res.processes.empty());
        CHECK(res.gantt.empty());
        CHECK(res.averageWaitingTime == 0.0);
    }

    SUBCASE("P01 & P02 — Sample chuẩn, RT và dữ liệu lớn") {
        auto processes = input("sample");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(7.80));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(11.30));
        CHECK(actual.averageResponseTime == doctest::Approx(5.30));

        auto expectedRes = readRows("ket_qua/preemptive/sample_p_result.csv");
        REQUIRE(actual.processes.size() == expectedRes.size());
        for (std::size_t i = 0; i < expectedRes.size(); ++i) {
            CHECK(actual.processes[i].pid == expectedRes[i][0]);
            CHECK(actual.processes[i].completionTime == expectedRes[i][1]);
            CHECK(actual.processes[i].turnaroundTime == expectedRes[i][2]);
            CHECK(actual.processes[i].waitingTime == expectedRes[i][3]);
            CHECK(actual.processes[i].responseTime == expectedRes[i][4]);
        }

        auto expectedGantt = readRows("ket_qua/preemptive/sample_p_gantt.csv");
        checkGantt(actual, expectedGantt);

        // P02: P1 response time is 0 (first run at t=0)
        CHECK(actual.processes[0].responseTime == 0);
    }

    SUBCASE("P03 — Không ngắt khi priority bằng nhau") {
        auto processes = input("preempt_same_priority");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(3.80));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(5.50));
        CHECK(actual.averageResponseTime == doctest::Approx(3.80));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 3}, {2, 3, 5}, {3, 5, 6}, {4, 6, 8}, {5, 8, 9},
            {6, 9, 11}, {7, 11, 12}, {8, 12, 14}, {9, 14, 15}, {10, 15, 17}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P04 — Nhiều lần ngắt") {
        auto processes = input("multi_preempt");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(3.20));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(5.60));
        CHECK(actual.averageResponseTime == doctest::Approx(0.30));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 1}, {2, 1, 2}, {3, 2, 3}, {4, 3, 4}, {5, 4, 6},
            {4, 6, 7}, {3, 7, 8}, {7, 8, 9}, {6, 9, 10}, {8, 10, 11},
            {2, 11, 12}, {9, 12, 14}, {10, 14, 15}, {1, 15, 24}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P05 — Ngắt đúng biên") {
        auto processes = input("preempt_boundary");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(1.80));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(4.00));
        CHECK(actual.averageResponseTime == doctest::Approx(1.20));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 2}, {2, 2, 4}, {3, 4, 6}, {4, 6, 8}, {1, 8, 10},
            {6, 10, 12}, {7, 12, 14}, {5, 14, 16}, {9, 16, 18}, {10, 18, 20}, {8, 20, 22}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P06 — IDLE đầu") {
        auto processes = input("idle_start");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(4.10));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(5.80));
        CHECK(actual.averageResponseTime == doctest::Approx(3.90));

        std::vector<std::vector<long long>> expectedGantt = {
            {-1, 0, 5}, {1, 5, 8}, {3, 8, 9}, {2, 9, 10}, {6, 10, 12},
            {2, 12, 13}, {5, 13, 14}, {10, 14, 16}, {8, 16, 18}, {4, 18, 20},
            {9, 20, 21}, {7, 21, 22}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P07 — IDLE giữa") {
        auto processes = input("idle_middle");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(1.20));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(2.80));
        CHECK(actual.averageResponseTime == doctest::Approx(1.20));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 2}, {2, 2, 3}, {3, 3, 5}, {4, 5, 6}, {5, 6, 8},
            {-1, 8, 20}, {6, 20, 22}, {7, 22, 23}, {8, 23, 25}, {9, 25, 26}, {10, 26, 28}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P08 — Priority nhỏ hơn thắng") {
        auto processes = input("priority_order");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(18.00));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(22.00));
        CHECK(actual.averageResponseTime == doctest::Approx(18.00));

        std::vector<std::vector<long long>> expectedGantt = {
            {4, 0, 4}, {6, 4, 8}, {3, 8, 12}, {8, 12, 16}, {1, 16, 20},
            {9, 20, 24}, {5, 24, 28}, {10, 28, 32}, {2, 32, 36}, {7, 36, 40}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P09 — Hòa priority, khác AT") {
        auto processes = input("tie_arrival");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(2.00));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(4.10));
        CHECK(actual.averageResponseTime == doctest::Approx(2.00));

        std::vector<std::vector<long long>> expectedGantt = {
            {2, 0, 3}, {1, 3, 6}, {3, 6, 9}, {4, 9, 11}, {5, 11, 12},
            {6, 12, 14}, {7, 14, 17}, {8, 17, 18}, {9, 18, 20}, {10, 20, 21}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P10 — Không sort input theo AT") {
        auto processes = input("unsorted_arrival");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(0.50));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(2.50));
        CHECK(actual.averageResponseTime == doctest::Approx(0.30));

        std::vector<std::vector<long long>> expectedGantt = {
            {2, 0, 3}, {4, 3, 5}, {3, 5, 6}, {6, 6, 7}, {-1, 7, 8},
            {5, 8, 10}, {1, 10, 13}, {7, 13, 15}, {8, 15, 17}, {7, 17, 18},
            {9, 18, 19}, {-1, 19, 20}, {10, 20, 22}
        };
        checkGantt(actual, expectedGantt);

        REQUIRE(actual.processes.size() == processes.size());
        for (std::size_t i = 0; i < processes.size(); ++i) {
            CHECK(actual.processes[i].pid == processes[i].pid);
        }
    }

    SUBCASE("P11 — Cùng priority, chạy lần lượt") {
        auto processes = input("preempt_fcfs");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(4.80));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(6.70));
        CHECK(actual.averageResponseTime == doctest::Approx(4.80));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 3}, {2, 3, 5}, {3, 5, 6}, {4, 6, 8}, {5, 8, 11},
            {6, 11, 13}, {7, 13, 14}, {8, 14, 16}, {9, 16, 17}, {10, 17, 19}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P12 — Ngắt liên tiếp 9 lần") {
        auto processes = input("preempt_staircase");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(0.90));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(3.30));
        CHECK(actual.averageResponseTime == doctest::Approx(0.00));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 1}, {2, 1, 2}, {3, 2, 3}, {4, 3, 4}, {5, 4, 5},
            {6, 5, 6}, {7, 6, 7}, {8, 7, 8}, {9, 8, 9}, {10, 9, 10}, {1, 10, 24}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P13 — Bị ngắt rồi quay lại nhiều lần") {
        auto processes = input("preempt_alternate");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(1.30));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(3.60));
        CHECK(actual.averageResponseTime == doctest::Approx(0.00));

        std::vector<std::vector<long long>> expectedGantt = {
            {1, 0, 1}, {2, 1, 3}, {1, 3, 4}, {3, 4, 6}, {1, 6, 7},
            {4, 7, 9}, {1, 9, 10}, {5, 10, 12}, {6, 12, 13}, {1, 13, 14},
            {7, 14, 15}, {1, 15, 16}, {8, 16, 17}, {1, 17, 18}, {9, 18, 19},
            {1, 19, 20}, {10, 20, 21}, {1, 21, 23}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P14 — Hòa priority, cùng AT") {
        auto processes = input("tie_same_arrival");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(13.30));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(15.80));
        CHECK(actual.averageResponseTime == doctest::Approx(13.30));

        std::vector<std::vector<long long>> expectedGantt = {
            {30, 0, 5}, {10, 5, 8}, {20, 8, 10}, {40, 10, 14}, {50, 14, 15},
            {60, 15, 17}, {70, 17, 20}, {80, 20, 21}, {90, 21, 23}, {100, 23, 25}
        };
        checkGantt(actual, expectedGantt);
    }

    SUBCASE("P16 — Dữ liệu lớn 100 process") {
        auto processes = input("p_large_100");
        auto actual = PreemptiveScheduler::schedule(processes);
        CHECK(actual.processes.size() == 100);
        REQUIRE_FALSE(actual.gantt.empty());
        CHECK(actual.gantt[0].pid == -1);
        CHECK(actual.gantt[0].startTime == 0);
        CHECK(actual.gantt[0].endTime == 2);
        CHECK(actual.gantt.size() > 100);
    }

    SUBCASE("P17 — Dữ liệu lớn 1000 process") {
        auto processes = input("p_large_1000");
        auto actual = PreemptiveScheduler::schedule(processes);
        CHECK(actual.processes.size() == 1000);
        CHECK(actual.gantt.size() > 1000);
        long long maxEnd = 0;
        for (const auto& seg : actual.gantt) {
            if (seg.endTime > maxEnd) maxEnd = seg.endTime;
        }
        CHECK(maxEnd == 5546);
    }

    SUBCASE("P18 — Demo 20 process") {
        auto processes = input("sample_20");
        auto actual = PreemptiveScheduler::schedule(processes);
        Metrics::fillAverages(actual);
        CHECK(actual.averageWaitingTime == doctest::Approx(21.60));
        CHECK(actual.averageTurnaroundTime == doctest::Approx(25.60));
        CHECK(actual.averageResponseTime == doctest::Approx(18.45));

        auto expectedRes = readRows("ket_qua/preemptive/sample_20_p_result.csv");
        REQUIRE(actual.processes.size() == expectedRes.size());
        for (std::size_t i = 0; i < expectedRes.size(); ++i) {
            CHECK(actual.processes[i].completionTime == expectedRes[i][1]);
            CHECK(actual.processes[i].turnaroundTime == expectedRes[i][2]);
            CHECK(actual.processes[i].waitingTime == expectedRes[i][3]);
            CHECK(actual.processes[i].responseTime == expectedRes[i][4]);
        }
        auto expectedGantt = readRows("ket_qua/preemptive/sample_20_p_gantt.csv");
        checkGantt(actual, expectedGantt);
    }
}

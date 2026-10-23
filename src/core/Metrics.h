#pragma once

#include <cstddef>

#include "core/ScheduleResult.h"

struct ComparisonRow {
    double np;
    double p;
};

struct Comparison {
    ComparisonRow waitingTime;
    ComparisonRow turnaroundTime;
    ComparisonRow responseTime;
};

class Metrics {
public:
    // Điền 3 trường average* từ result.processes; rỗng -> 0.0.
    static void fillAverages(ScheduleResult& result);

    // Bảng so sánh 2 biến thể, đọc 3 trường average* đã điền.
    static Comparison compare(const ScheduleResult& np,
                              const ScheduleResult& p);
};

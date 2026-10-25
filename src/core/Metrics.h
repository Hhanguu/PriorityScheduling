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
    static void fillAverages(ScheduleResult& result);
    static Comparison compare(const ScheduleResult& np, const ScheduleResult& p);
};
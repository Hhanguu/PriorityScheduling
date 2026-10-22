#pragma once
#include <vector>
#include "core/GanttSegment.h"

struct ProcessResult {
    int pid;
    long long completionTime;
    long long turnaroundTime;
    long long waitingTime;
    long long responseTime;
};

struct ScheduleResult {
    std::vector<ProcessResult> processes;
    std::vector<GanttSegment> gantt;
    double averageWaitingTime = 0.0;
    double averageTurnaroundTime = 0.0;
    double averageResponseTime = 0.0;
};
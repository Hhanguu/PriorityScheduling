#pragma once

#include <vector>
#include "core/GanttSegment.h"

struct ProcessResult {
    int pid;
    long long completionTime;    // CT
    long long turnaroundTime;    // TAT = CT - AT
    long long waitingTime;       // WT = TAT - BT
    long long responseTime;      // RT = ST - AT
};

struct ScheduleResult {
    std::vector<ProcessResult> processes;  // thứ tự = thứ tự input
    std::vector<GanttSegment>  gantt;      // đoạn liền kề cùng pid đã gộp
    double averageWaitingTime = 0.0;
    double averageTurnaroundTime = 0.0;
    double averageResponseTime = 0.0;
};
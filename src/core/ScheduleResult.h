
#pragma once

#include <vector>
#include "GanttSegment.h"

struct ProcessResult {
    int pid; //Mã tiến trình//
    long long completionTime; //CT — thời điểm hoàn thành//
    long long turnaroundTime; //TAT — thời gian quay vòng//
    long long waitingTime; //WT — thời gian chờ//
    long long responseTime; //RT — thời gian phản hồi//
};

struct ScheduleResult {
    std::vector<ProcessResult> processes;
    std::vector<GanttSegment> gantt;

    double averageWaitingTime = 0.0;
    double averageTurnaroundTime = 0.0;
};
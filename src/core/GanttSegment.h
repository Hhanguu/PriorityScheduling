#pragma once

struct GanttSegment {
    int pid;                 // -1 = IDLE
    long long startTime;
    long long endTime;       // endTime > startTime
};
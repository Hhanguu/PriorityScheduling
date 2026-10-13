
#pragma once

struct GanttSegment {
    int pid; //Tiến trình đang chiếm CPU//
    long long startTime; //Thời điểm bắt đầu chạy//
    long long endTime; //Thời điểm kết thúc đoạn chạy//
};
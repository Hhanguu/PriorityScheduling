
#include <iostream>

#include "core/Process.h"
#include "core/GanttSegment.h"
#include "core/ScheduleResult.h"

int main() {
    Process p{1, 0, 5, 2};
    GanttSegment segment{p.pid, 0, 5};

    ScheduleResult result;
    result.gantt.push_back(segment);

    std::cout << "Priority Scheduling\n";
    std::cout << "PID: " << p.pid << '\n';
    std::cout << "Gantt: "
              << segment.startTime << " -> "
              << segment.endTime << '\n';

    return 0;
}
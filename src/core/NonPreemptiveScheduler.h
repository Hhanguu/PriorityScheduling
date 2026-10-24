#pragma once

#include <vector>

#include "core/Process.h"
#include "core/ScheduleResult.h"

class NonPreemptiveScheduler {
public:
    // Requires valid processes: unique positive pid, AT >= 0, BT > 0.
    static ScheduleResult schedule(const std::vector<Process>& processes);
};

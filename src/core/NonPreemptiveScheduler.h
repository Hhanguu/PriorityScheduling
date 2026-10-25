#pragma once

#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class NonPreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};

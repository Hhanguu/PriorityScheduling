#include "core/NonPreemptiveScheduler.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace {
void addSegment(ScheduleResult& result, int pid, long long start, long long end) {
    if (end <= start) return;
    if (!result.gantt.empty() &&
        result.gantt.back().pid == pid &&
        result.gantt.back().endTime == start) {
        result.gantt.back().endTime = end;
    } else {
        result.gantt.push_back({pid, start, end});
    }
}
}

ScheduleResult NonPreemptiveScheduler::schedule(
    const std::vector<Process>& processes)
{
    ScheduleResult result;
    const std::size_t n = processes.size();
    if (n == 0) return result;

    std::vector<bool> finished(n, false);
    std::vector<ProcessResult> byInput(n);
    std::vector<bool> hasResult(n, false);
    std::size_t completed = 0;
    long long time = 0;

    while (completed < n) {
        std::size_t best = n;
        for (std::size_t i = 0; i < n; ++i) {
            if (finished[i] || processes[i].arrivalTime > time) continue;
            if (best == n ||
                processes[i].priority < processes[best].priority ||
                (processes[i].priority == processes[best].priority &&
                 processes[i].arrivalTime < processes[best].arrivalTime)) {
                best = i;
            }
        }

        if (best == n) {
            long long nextArrival = -1;
            for (std::size_t i = 0; i < n; ++i) {
                if (!finished[i] &&
                    (nextArrival < 0 || processes[i].arrivalTime < nextArrival)) {
                    nextArrival = processes[i].arrivalTime;
                }
            }
            if (nextArrival > time) {
                addSegment(result, -1, time, nextArrival);
                time = nextArrival;
            }
            continue;
        }

        const Process& p = processes[best];
        const long long start = time;
        const long long end = time + p.burstTime;
        addSegment(result, p.pid, start, end);

        const long long tat = end - p.arrivalTime;
        byInput[best] = {
            p.pid,
            end,
            tat,
            tat - p.burstTime,
            start - p.arrivalTime
        };
        hasResult[best] = true;
        finished[best] = true;
        ++completed;
        time = end;
    }

    result.processes.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        if (hasResult[i]) result.processes.push_back(byInput[i]);
    }
    return result;
}

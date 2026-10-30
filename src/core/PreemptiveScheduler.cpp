#include "core/PreemptiveScheduler.h"
#include <algorithm>
#include <climits>
#include <numeric>
#include <queue>

ScheduleResult PreemptiveScheduler::schedule(const std::vector<Process>& processes) {
    ScheduleResult result;
    int n = static_cast<int>(processes.size());
    if (n == 0) return result;

    result.processes.resize(n);
    std::vector<long long> remainingBurst(n);
    std::vector<long long> startTime(n, -1);

    for (int i = 0; i < n; ++i) {
        remainingBurst[i] = processes[i].burstTime;
    }

    std::vector<std::size_t> arrivals(n);
    std::iota(arrivals.begin(), arrivals.end(), std::size_t{0});
    std::stable_sort(arrivals.begin(), arrivals.end(), [&](std::size_t a, std::size_t b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });

    auto cmp = [&](std::size_t a, std::size_t b) {
        if (processes[a].priority != processes[b].priority) {
            return processes[a].priority > processes[b].priority; // min-heap: smaller priority value first
        }
        if (processes[a].arrivalTime != processes[b].arrivalTime) {
            return processes[a].arrivalTime > processes[b].arrivalTime;
        }
        return a > b;
    };

    std::priority_queue<std::size_t, std::vector<std::size_t>, decltype(cmp)> ready(cmp);

    long long currentTime = 0;
    std::size_t nextArrivalIdx = 0;
    int completed = 0;

    while (completed < n) {
        while (nextArrivalIdx < n && processes[arrivals[nextArrivalIdx]].arrivalTime <= currentTime) {
            ready.push(arrivals[nextArrivalIdx++]);
        }

        if (ready.empty()) {
            if (nextArrivalIdx >= n) break;
            long long nextTime = processes[arrivals[nextArrivalIdx]].arrivalTime;
            if (!result.gantt.empty() && result.gantt.back().pid == -1) {
                result.gantt.back().endTime = nextTime;
            } else {
                result.gantt.push_back({ -1, currentTime, nextTime });
            }
            currentTime = nextTime;
            continue;
        }

        std::size_t curr = ready.top();
        ready.pop();

        long long nextEventTime = currentTime + remainingBurst[curr];

        while (nextArrivalIdx < n && processes[arrivals[nextArrivalIdx]].arrivalTime < nextEventTime) {
            std::size_t arrIdx = arrivals[nextArrivalIdx];
            if (processes[arrIdx].priority < processes[curr].priority) {
                nextEventTime = processes[arrIdx].arrivalTime;
                break;
            } else {
                ready.push(arrIdx);
                nextArrivalIdx++;
            }
        }

        if (startTime[curr] == -1) {
            startTime[curr] = currentTime;
        }

        long long dt = nextEventTime - currentTime;
        remainingBurst[curr] -= dt;

        if (!result.gantt.empty() && result.gantt.back().pid == processes[curr].pid) {
            result.gantt.back().endTime = nextEventTime;
        } else {
            result.gantt.push_back({ processes[curr].pid, currentTime, nextEventTime });
        }

        currentTime = nextEventTime;

        if (remainingBurst[curr] == 0) {
            completed++;
            ProcessResult pr;
            pr.pid = processes[curr].pid;
            pr.completionTime = currentTime;
            pr.turnaroundTime = pr.completionTime - processes[curr].arrivalTime;
            pr.waitingTime = pr.turnaroundTime - processes[curr].burstTime;
            pr.responseTime = startTime[curr] - processes[curr].arrivalTime;
            result.processes[curr] = pr;
        } else {
            ready.push(curr);
        }
    }

    return result;
}

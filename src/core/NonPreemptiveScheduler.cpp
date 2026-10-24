#include "core/NonPreemptiveScheduler.h"

#include <algorithm>
#include <numeric>
#include <queue>

ScheduleResult NonPreemptiveScheduler::schedule(const std::vector<Process>& processes) {
    ScheduleResult result;
    result.processes.resize(processes.size());

    // Sort indices only, so results and tie breaks retain input order.
    std::vector<std::size_t> arrivals(processes.size());
    std::iota(arrivals.begin(), arrivals.end(), std::size_t{0});
    std::stable_sort(arrivals.begin(), arrivals.end(), [&](auto a, auto b) {
        return processes[a].arrivalTime < processes[b].arrivalTime;
    });
    const auto later = [&](std::size_t a, std::size_t b) {
        if (processes[a].priority != processes[b].priority) {
            return processes[a].priority > processes[b].priority;
        }
        if (processes[a].arrivalTime != processes[b].arrivalTime) {
            return processes[a].arrivalTime > processes[b].arrivalTime;
        }
        return a > b;
    };
    std::priority_queue<std::size_t, std::vector<std::size_t>, decltype(later)> ready(later);
    long long time = 0;
    std::size_t next = 0;
    while (next < arrivals.size() || !ready.empty()) {
        if (ready.empty() && time < processes[arrivals[next]].arrivalTime) {
            const long long arrival = processes[arrivals[next]].arrivalTime;
            result.gantt.push_back({-1, time, arrival});
            time = arrival;
        }
        while (next < arrivals.size() && processes[arrivals[next]].arrivalTime <= time) {
            ready.push(arrivals[next++]);
        }
        const std::size_t index = ready.top();
        ready.pop();
        const Process& process = processes[index];
        const long long start = time;
        time += process.burstTime; // Run to completion, including across new arrivals.
        result.gantt.push_back({process.pid, start, time});
        const long long turnaround = time - process.arrivalTime;
        result.processes[index] = {process.pid, time, turnaround,
                                   turnaround - process.burstTime,
                                   start - process.arrivalTime};
    }

    return result;
}

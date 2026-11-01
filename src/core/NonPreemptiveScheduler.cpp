#include "core/NonPreemptiveScheduler.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <utility>

namespace {

template <typename Index>
ScheduleResult scheduleImpl(const std::vector<Process>& processes) {
    ScheduleResult result;
    const std::size_t count = processes.size();
    result.processes.resize(count);
    result.gantt.reserve(count);

    // Sort indices only; input and result retain vector order.
    std::vector<Index> arrivals(count);
    for (std::size_t i = 0; i < count; ++i) arrivals[i] = static_cast<Index>(i);
    std::sort(arrivals.begin(), arrivals.end(), [&](Index a, Index b) {
        return processes[a].arrivalTime < processes[b].arrivalTime ||
               (processes[a].arrivalTime == processes[b].arrivalTime && a < b);
    });

    // Arrival order already resolves ties within a priority, so each priority
    // needs only a FIFO. The map orders the distinct ready priorities, not n jobs.
    std::vector<Index> links(count);
    // Each map entry stores the head/tail indices of an intrusive FIFO.
    // A single links array avoids one deque allocation per distinct priority.
    std::map<int, std::pair<Index, Index>> ready;
    long long time = 0;
    std::size_t next = 0;
    while (next < count || !ready.empty()) {
        if (ready.empty() && time < processes[arrivals[next]].arrivalTime) {
            result.gantt.push_back({-1, time, processes[arrivals[next]].arrivalTime});
            time = processes[arrivals[next]].arrivalTime;
        }
        while (next < count && processes[arrivals[next]].arrivalTime <= time) {
            const Index index = arrivals[next++];
            auto inserted = ready.try_emplace(processes[index].priority, index, index);
            if (!inserted.second) {
                links[inserted.first->second.second] = index;
                inserted.first->second.second = index;
            }
        }
        auto first = ready.begin();
        const Index index = first->second.first;
        if (index == first->second.second) ready.erase(first);
        else first->second.first = links[index];

        const Process& process = processes[index];
        const long long start = time;
        time += process.burstTime;
        result.gantt.push_back({process.pid, start, time});
        const long long turnaround = time - process.arrivalTime;
        result.processes[index] = {process.pid, time, turnaround,
                                   turnaround - process.burstTime,
                                   start - process.arrivalTime};
    }
    return result;
}

} // namespace

ScheduleResult NonPreemptiveScheduler::schedule(const std::vector<Process>& processes) {
    // Compact scratch indices for normal workloads; retain size_t support for
    // vectors larger than the uint32_t range. Public time fields stay long long.
    if (processes.size() <= std::numeric_limits<std::uint32_t>::max()) {
        return scheduleImpl<std::uint32_t>(processes);
    }
    return scheduleImpl<std::size_t>(processes);
}

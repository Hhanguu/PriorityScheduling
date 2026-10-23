#include "core/Metrics.h"

namespace {

double average(long double sum, std::size_t count) {
    if (count == 0) {
        return 0.0;
    }
    return sum / count;
}

} // namespace

void Metrics::fillAverages(ScheduleResult& result) {
    long double waiting = 0.0;
    long double turnaround = 0.0;
    long double response = 0.0;

    for (const ProcessResult& process : result.processes) {
        waiting += process.waitingTime;
        turnaround += process.turnaroundTime;
        response += process.responseTime;
    }

    const std::size_t count = result.processes.size();
    result.averageWaitingTime = average(waiting, count);
    result.averageTurnaroundTime = average(turnaround, count);
    result.averageResponseTime = average(response, count);
}

Comparison Metrics::compare(const ScheduleResult& np, const ScheduleResult& p) {
    Comparison comparison;
    comparison.waitingTime = {np.averageWaitingTime, p.averageWaitingTime};
    comparison.turnaroundTime = {np.averageTurnaroundTime, p.averageTurnaroundTime};
    comparison.responseTime = {np.averageResponseTime, p.averageResponseTime};
    return comparison;
}

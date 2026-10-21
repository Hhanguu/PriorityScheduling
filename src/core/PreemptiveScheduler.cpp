#include "core/PreemptiveScheduler.h"
#include <algorithm>
#include <climits>

ScheduleResult PreemptiveScheduler::schedule(const std::vector<Process>& processes) {
    ScheduleResult result;
    int n = processes.size();
    if (n == 0) return result;

    std::vector<long long> remainingBurstTime(n);
    std::vector<long long> startTime(n, -1);

    for (int i = 0; i < n; ++i) {
        remainingBurstTime[i] = processes[i].burstTime;
    }

    long long currentTime = 0;
    int completed = 0;

    // Khởi tạo mảng kết quả bằng kích thước input để giữ đúng thứ tự (Bất biến I1)
    result.processes.resize(n);

    while (completed < n) {
        int bestIdx = -1;
        int highestPriority = INT_MAX;
        long long earliestArrival = LLONG_MAX;

        for (int i = 0; i < n; ++i) {
            if (processes[i].arrivalTime <= currentTime && remainingBurstTime[i] > 0) {
                if (processes[i].priority < highestPriority) {
                    highestPriority = processes[i].priority;
                    earliestArrival = processes[i].arrivalTime;
                    bestIdx = i;
                }
                else if (processes[i].priority == highestPriority) {
                    // Tie-break: Không xét pid, chỉ xét AT và thứ tự vector
                    if (processes[i].arrivalTime < earliestArrival) {
                        earliestArrival = processes[i].arrivalTime;
                        bestIdx = i;
                    }
                }
            }
        }

        if (bestIdx == -1) {
            long long nextArrival = LLONG_MAX;
            for (int i = 0; i < n; ++i) {
                if (remainingBurstTime[i] > 0 && processes[i].arrivalTime > currentTime) {
                    nextArrival = std::min(nextArrival, processes[i].arrivalTime);
                }
            }
            if (nextArrival == LLONG_MAX) break;

            if (!result.gantt.empty() && result.gantt.back().pid == -1) {
                result.gantt.back().endTime = nextArrival;
            }
            else {
                result.gantt.push_back({ -1, currentTime, nextArrival });
            }
            currentTime = nextArrival;
            continue;
        }

        if (startTime[bestIdx] == -1) {
            startTime[bestIdx] = currentTime;
        }

        if (!result.gantt.empty() && result.gantt.back().pid == processes[bestIdx].pid) {
            result.gantt.back().endTime = currentTime + 1;
        }
        else {
            result.gantt.push_back({ processes[bestIdx].pid, currentTime, currentTime + 1 });
        }

        remainingBurstTime[bestIdx]--;
        currentTime++;

        if (remainingBurstTime[bestIdx] == 0) {
            completed++;
            ProcessResult pr;
            pr.pid = processes[bestIdx].pid;
            pr.completionTime = currentTime;
            pr.turnaroundTime = pr.completionTime - processes[bestIdx].arrivalTime;
            pr.waitingTime = pr.turnaroundTime - processes[bestIdx].burstTime;
            pr.responseTime = startTime[bestIdx] - processes[bestIdx].arrivalTime;

            // Lưu vào đúng vị trí index trong mảng input (Bất biến I1)
            result.processes[bestIdx] = pr;
        }
    }

    return result;
}
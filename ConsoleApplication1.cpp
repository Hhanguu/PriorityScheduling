#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <climits>
#include <string>

using namespace std;

struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int start_time;
    int remaining_burst_time;

    Process(int id, int at, int bt, int pri) :
        pid(id), arrival_time(at), burst_time(bt), priority(pri),
        completion_time(0), turnaround_time(0), waiting_time(0),
        response_time(0), start_time(-1), remaining_burst_time(bt) {
    }
};

struct ScheduleResult {
    vector<Process> processes;
    double avg_tat;
    double avg_wt;
    double avg_rt;
};

void calculateMetrics(ScheduleResult& result) {
    double total_tat = 0, total_wt = 0, total_rt = 0;
    int n = result.processes.size();

    for (auto& p : result.processes) {
        p.turnaround_time = p.completion_time - p.arrival_time;
        p.waiting_time = p.turnaround_time - p.burst_time;
        p.response_time = p.start_time - p.arrival_time;

        total_tat += p.turnaround_time;
        total_wt += p.waiting_time;
        total_rt += p.response_time;
    }

    if (n > 0) {
        result.avg_tat = total_tat / n;
        result.avg_wt = total_wt / n;
        result.avg_rt = total_rt / n;
    }
    else {
        result.avg_tat = result.avg_wt = result.avg_rt = 0.0;
    }
}

bool comparePID(const Process& a, const Process& b) {
    return a.pid < b.pid;
}

ScheduleResult priorityPreemptive(vector<Process> processes) {
    int n = processes.size();
    int completed = 0;
    int current_time = 0;
    vector<Process> result_procs;

    while (completed < n) {
        int best_idx = -1;
        int highest_priority = INT_MAX;
        int earliest_at = INT_MAX;
        int smallest_pid = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_burst_time > 0) {
                if (processes[i].priority < highest_priority ||
                    (processes[i].priority == highest_priority && processes[i].arrival_time < earliest_at) ||
                    (processes[i].priority == highest_priority && processes[i].arrival_time == earliest_at && processes[i].pid < smallest_pid)) {

                    highest_priority = processes[i].priority;
                    earliest_at = processes[i].arrival_time;
                    smallest_pid = processes[i].pid;
                    best_idx = i;
                }
            }
        }

        if (best_idx == -1) {
            current_time++;
            continue;
        }

        Process& p = processes[best_idx];

        if (p.start_time == -1) {
            p.start_time = current_time;
        }

        p.remaining_burst_time--;
        current_time++;

        if (p.remaining_burst_time == 0) {
            p.completion_time = current_time;
            result_procs.push_back(p);
            completed++;
        }
    }

    sort(result_procs.begin(), result_procs.end(), comparePID);

    ScheduleResult res;
    res.processes = result_procs;
    calculateMetrics(res);
    return res;
}

void printResult(const ScheduleResult& res) {
    cout << "--- PREEMPTIVE PRIORITY SCHEDULING ---" << endl;

    cout << left << setw(6) << "PID" << setw(6) << "AT" << setw(6) << "BT"
        << setw(6) << "Pri" << setw(6) << "CT" << setw(6) << "TAT"
        << setw(6) << "WT" << setw(6) << "RT" << endl;

    for (const auto& p : res.processes) {
        cout << left << setw(6) << p.pid << setw(6) << p.arrival_time << setw(6) << p.burst_time
            << setw(6) << p.priority << setw(6) << p.completion_time << setw(6) << p.turnaround_time
            << setw(6) << p.waiting_time << setw(6) << p.response_time << endl;
    }

    cout << fixed << setprecision(2);
    cout << "Average TAT: " << res.avg_tat
        << ", Average WT: " << res.avg_wt
        << ", Average RT: " << res.avg_rt << "\n\n";
}

int main() {
    vector<Process> process_list = {
        Process(1, 0, 5, 2),
        Process(2, 1, 3, 1),
        Process(3, 2, 4, 3)
    };

    cout << "=== KIEM THU PREEMPTIVE PRIORITY SCHEDULING (C++) ===\n\n";

    ScheduleResult result = priorityPreemptive(process_list);
    printResult(result);

    return 0;
}
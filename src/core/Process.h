
#pragma once

struct Process {
    int pid;  //Mã tiến trình//
    long long arrivalTime; //Thời điểm tiến trình đến (AT)//
    long long burstTime; //Thời gian CPU cần chạy (BT)//
    int priority; //Độ ưu tiên, số càng nhỏ càng ưu tiên //
    };
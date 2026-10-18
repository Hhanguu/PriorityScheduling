# TÀI LIỆU THIẾT KẾ MODULE NON-PREEMPTIVE PRIORITY SCHEDULING

**Đề tài:** Priority Scheduling – Non-preemptive và Preemptive  
**Phần phụ trách:** Non-preemptive Priority Scheduling  
**Giai đoạn:** Tài liệu phân tích và thiết kế (chưa triển khai code)  
**Phiên bản:** 1.0 – Bản thảo tuần này

## 1. Mục tiêu và phạm vi

Module mô phỏng lập lịch CPU theo độ ưu tiên **không ngắt (non-preemptive)**. Tại mỗi thời điểm CPU rảnh, thuật toán chọn một tiến trình đã đến và chưa hoàn thành theo thứ tự ưu tiên quy định. Một khi bắt đầu thực thi, tiến trình chạy liên tục đến hết Burst Time; tiến trình ưu tiên cao hơn đến sau không được giành CPU.

**Phạm vi module:** chọn thứ tự và thời gian chạy; xử lý IDLE; tính kết quả từng tiến trình; tạo dữ liệu Gantt; tính giá trị trung bình; trả về `ScheduleResult`. Giao diện, phần Preemptive và biểu đồ so sánh do các phần khác đảm nhiệm (trừ khi nhóm phân công thêm).

## 2. Quy tắc nghiệp vụ

1. **Priority càng nhỏ, mức ưu tiên càng cao**: Priority 1 cao hơn Priority 2.
2. Chỉ xét các tiến trình có `Arrival Time <= thời điểm hiện tại` và chưa hoàn thành.
3. Nếu hai tiến trình có cùng Priority, chọn tiến trình có Arrival Time nhỏ hơn.
4. Nếu cả Priority và Arrival Time đều bằng nhau, chọn tiến trình được **nhập trước** (inputOrder nhỏ hơn), không dựa trên PID.
5. Sau khi chọn, tiến trình chạy đến khi hoàn thành (**không ngắt**).
6. Nếu không có tiến trình sẵn sàng, CPU ở trạng thái **IDLE** cho đến Arrival Time gần nhất tiếp theo; đoạn IDLE cũng được ghi vào dữ liệu Gantt.
7. Không thay đổi danh sách đầu vào; duy trì thứ tự kết quả theo thứ tự nhập để thuận tiện tích hợp.
8. **Starvation:** tiến trình có ưu tiên thấp có thể chờ lâu khi nhiều tiến trình có ưu tiên cao đến trước lúc CPU được cấp cho nó. Phân tích hiện tượng này trong tài liệu; Aging là hướng mở rộng tùy yêu cầu nhóm, **không mặc định triển khai**.

## 3. Thuật ngữ và công thức

| Ký hiệu | Ý nghĩa | Xác định / công thức |
|---|---|---|
| AT | Arrival Time | Thời điểm tiến trình đến (đầu vào) |
| BT | Burst Time | Tổng thời gian CPU cần (đầu vào) |
| ST | Start Time | Thời điểm tiến trình được chạy lần đầu |
| CT | Completion Time | Thời điểm tiến trình hoàn thành |
| TAT | Turnaround Time | CT - AT |
| WT | Waiting Time | TAT - BT |
| RT | Response Time | ST - AT |

Với Non-preemptive đơn CPU, không tính thời gian chuyển ngữ cảnh và không có I/O chờ: **WT = RT** cho mỗi tiến trình. Các giá trị trung bình được tính bằng tổng giá trị chia cho số tiến trình `n` (n > 0): `AvgWT`, `AvgTAT`, `AvgRT`.

## 4. Chức năng chính và trách nhiệm

| Chức năng | Nội dung | Đầu ra |
|---|---|---|
| Nhận và kiểm tra dữ liệu | Nhận danh sách Process; kiểm tra AT >= 0, BT > 0, Priority hợp lệ theo quy ước nhóm, thứ tự nhập duy nhất | Danh sách hợp lệ hoặc thông báo lỗi |
| Chọn tiến trình | Tìm tiến trình đã đến, chưa hoàn thành; so sánh Priority -> AT -> inputOrder | Process được chọn |
| Xử lý không ngắt | Chạy process từ `currentTime` đến `currentTime + BT` | Khoảng chạy và thời điểm hoàn thành |
| Xử lý IDLE | Nếu chưa có process sẵn sàng, nhảy đến AT nhỏ nhất trong các process chưa chạy | Đoạn Gantt IDLE |
| Tính thống kê | Tính ST, CT, TAT, WT, RT của từng process và trung bình | ProcessResult và các chỉ số |
| Xuất lịch chạy | Tạo danh sách đoạn Gantt theo thời gian | Danh sách GanttSegment |
| Tích hợp | Trả về đúng kiểu ScheduleResult chung của nhóm | ScheduleResult |

## 5. Đầu vào, đầu ra và cách gọi hàm (thiết kế dự kiến)

### 5.1. Đầu vào

`vector<Process>`; mỗi phần tử dự kiến gồm `pid`, `arrivalTime`, `burstTime`, `priority`, `inputOrder`. Tên trường và kiểu dữ liệu phải **được cả nhóm thống nhất trước khi code**.

### 5.2. Giao diện hàm đề xuất (C++)

```cpp
ScheduleResult nonPreemptive(const vector<Process>& processes);
```

Đây là **chữ ký hàm dự kiến**, chưa phải code hoàn chỉnh; nếu nhóm chọn Python hoặc thay kiểu dữ liệu thì chỉnh lại tài liệu thống nhất.

### 5.3. Đầu ra

`ScheduleResult` gồm: (1) danh sách `ProcessResult` với ST, CT, TAT, WT, RT; (2) danh sách `GanttSegment` với PID/IDLE, start, end; (3) `avgWaitingTime`, `avgTurnaroundTime`, `avgResponseTime`. Hợp đồng xử lý danh sách rỗng và dữ liệu sai phải được nhóm thống nhất (đề xuất: trả lỗi xác thực thay vì chia cho 0).

## 6. Luồng hoạt động (flow)

```text
BẮT ĐẦU
   |
   v
Nhận danh sách Process và kiểm tra dữ liệu
   |
   v
currentTime = 0; đánh dấu tất cả chưa chạy
   |
   v
Còn tiến trình chưa hoàn thành? -- Không --> Tính trung bình --> ScheduleResult --> KẾT THÚC
   |
  Có
   v
Có tiến trình AT <= currentTime chưa chạy?
   | Có                               | Không
   v                                   v
Chọn Priority nhỏ nhất;              Tìm AT gần nhất
hòa: AT -> inputOrder                 Ghi Gantt IDLE
   |                                   Nhảy currentTime đến AT đó
   v                                   |
ST = currentTime                       |
CT = currentTime + BT                  |
Ghi Gantt [ST, CT)                     |
Tính TAT, WT, RT                       |
currentTime = CT                       |
Đánh dấu hoàn thành                    |
   +------------------<----------------+
   |
   +--> Lặp lại kiểm tra còn tiến trình
```

### Giả mã

```text
nonPreemptive(processes):
    validate(processes)
    currentTime = 0
    while còn process chưa hoàn thành:
        ready = các process chưa chạy có AT <= currentTime
        if ready rỗng:
            nextTime = min(AT của process chưa chạy)
            thêm đoạn Gantt (IDLE, currentTime, nextTime)
            currentTime = nextTime
            continue
        p = min(ready, theo khóa (Priority, AT, inputOrder))
        ST[p] = currentTime
        CT[p] = currentTime + BT[p]
        ghi đoạn Gantt (p, ST[p], CT[p])
        TAT[p] = CT[p] - AT[p]
        WT[p] = TAT[p] - BT[p]
        RT[p] = ST[p] - AT[p]
        currentTime = CT[p]
        đánh dấu p đã hoàn thành
    tính trung bình và trả ScheduleResult
```

## 7. Ví dụ minh họa có kết quả kiểm chứng

### 7.1. Bộ dữ liệu

| Thứ tự nhập | PID | AT | BT | Priority |
|---:|---|---:|---:|---:|
| 1 | P1 | 0 | 5 | 3 |
| 2 | P2 | 1 | 3 | 1 |
| 3 | P3 | 2 | 2 | 2 |
| 4 | P4 | 4 | 4 | 1 |

### 7.2. Thứ tự và Gantt Chart

Tại t=0 chỉ có P1 nên P1 chạy 0–5, không bị P2 ngắt khi P2 đến lúc t=1. Tại t=5, P2 và P4 cùng priority=1, nhưng P2 có AT=1 nhỏ hơn P4 có AT=4. Sau đó chạy P4 rồi P3.

```text
0       5       8           12    14
|  P1   |  P2   |    P4     | P3 |
```

Các đoạn Gantt chính xác: `(P1,0,5)`, `(P2,5,8)`, `(P4,8,12)`, `(P3,12,14)`.

### 7.3. Kết quả

| PID | AT | BT | ST | CT | TAT | WT | RT |
|---|---:|---:|---:|---:|---:|---:|---:|
| P1 | 0 | 5 | 0 | 5 | 5 | 0 | 0 |
| P2 | 1 | 3 | 5 | 8 | 7 | 4 | 4 |
| P3 | 2 | 2 | 12 | 14 | 12 | 10 | 10 |
| P4 | 4 | 4 | 8 | 12 | 8 | 4 | 4 |

- `AvgWT = (0+4+10+4)/4 = 4.5`
- `AvgTAT = (5+7+12+8)/4 = 8.0`
- `AvgRT = (0+4+10+4)/4 = 4.5`

### 7.4. Ví dụ IDLE

Với `P1: AT=3, BT=2, Priority=1` và `P2: AT=7, BT=3, Priority=2`, Gantt: `(IDLE,0,3)`, `(P1,3,5)`, `(IDLE,5,7)`, `(P2,7,10)`.

## 8. Cấu trúc và thiết kế file (đề xuất)

```text
PriorityScheduling/
├── include/
│   ├── Process.h              # Struct Process dùng chung
│   ├── ScheduleResult.h       # Struct kết quả dùng chung
│   └── NonPreemptive.h        # Khai báo hàm của module
├── src/
│   └── NonPreemptive.cpp      # Cài đặt thuật toán (làm ở giai đoạn code)
├── tests/
│   └── test_nonpreemptive.cpp # Kiểm thử module độc lập (giai đoạn code)
└── docs/
    └── NonPreemptive.md       # Tài liệu module
```

**Struct dự kiến:**

- `Process`: `pid`, `arrivalTime`, `burstTime`, `priority`, `inputOrder`.
- `ProcessResult`: `pid`, `startTime`, `completionTime`, `turnaroundTime`, `waitingTime`, `responseTime`.
- `GanttSegment`: `pid` (hoặc cờ IDLE), `startTime`, `endTime`.
- `ScheduleResult`: danh sách ProcessResult, danh sách GanttSegment và các chỉ số trung bình.

**Hàm chính:** `nonPreemptive(processes)` đảm nhiệm toàn bộ mô phỏng; hàm kiểm tra dữ liệu và tính thống kê có thể là helper nội bộ hoặc thư viện dùng chung tùy quyết định nhóm. Không chỉnh sửa `Process` đầu vào và không sử dụng biến toàn cục.

**Tương tác module:** Module nhập dữ liệu / UI tạo `vector<Process>` -> gọi `nonPreemptive` -> nhận `ScheduleResult` -> module Gantt/so sánh hiển thị. Phần Preemptive phải sử dụng cùng các struct và quy tắc đầu vào, đầu ra.

## 9. Kế hoạch kiểm thử module độc lập

| Mã test | Trường hợp | Kết quả mong đợi |
|---|---|---|
| T01 | Chỉ 1 tiến trình AT=0 | Chạy liên tục; WT=RT=0 |
| T02 | Tiến trình ưu tiên cao đến khi CPU đang bận | Không được ngắt tiến trình đang chạy |
| T03 | Hai tiến trình cùng Priority, khác AT | AT nhỏ hơn được chọn trước khi CPU rảnh |
| T04 | Cùng Priority và AT | Giữ thứ tự nhập, không sắp theo PID |
| T05 | Chưa có process tại t=0 | Ghi đoạn IDLE đầu lịch |
| T06 | Có khoảng trống giữa các process | Ghi đủ IDLE ở giữa |
| T07 | Đầu vào không theo thứ tự AT | Chọn đúng theo quy tắc, không phụ thuộc thứ tự danh sách trừ hòa hoàn toàn |
| T08 | Bộ dữ liệu tại mục 7 | Đúng Gantt và toàn bộ CT/TAT/WT/RT |
| T09 | Danh sách rỗng hoặc BT không hợp lệ | Phát hiện dữ liệu sai theo hợp đồng chung |

Mỗi test sẽ được hiện thực trong `tests/test_nonpreemptive.cpp` ở giai đoạn lập trình, chạy đạt trước khi tạo Pull Request vào nhánh `main`.

## 10. Giới hạn, giả định và điểm cần thống nhất với nhóm

- Mô hình một CPU, thời gian nguyên không âm; BT > 0; không xét I/O, overhead context switch hoặc nhiều lõi CPU.
- Trong non-preemptive, mỗi process tạo một đoạn Gantt chạy liên tục (ngoại trừ các đoạn IDLE riêng).
- Không tự động thực hiện Aging; chỉ phân tích starvation, trừ khi nhóm quyết định bổ sung.
- Nhóm cần chốt: C++ hay Python; kiểu PID; giới hạn Priority; cách báo lỗi; tên và kiểu trường của `Process`/`ScheduleResult`; quy ước biểu diễn IDLE; quy tắc ghi nhận các mốc thời gian.
- Đây là tài liệu **thiết kế dự kiến**, không khẳng định các file, hàm hoặc bài test đã được cài đặt.

## 11. Kết quả bàn giao tuần này

1. `NonPreemptive.md` – tài liệu Markdown để lưu tại thư mục docs và review trên GitHub.
2. `NonPreemptive_Documentation.docx` – phiên bản Word phục vụ đọc, nhận xét và nộp báo cáo.
3. Nội dung tài liệu bao gồm: mô tả thuật toán, chức năng, hướng dẫn gọi hàm, input/output, flow, thiết kế cấu trúc và kế hoạch test.

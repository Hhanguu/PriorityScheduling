# TÀI LIỆU MODULE PREEMPTIVE SCHEDULER – PRIORITY SCHEDULING

**Pseudocode và flowchart:** [algorithms.md](algorithms.md).

Nhánh: `feature/preemptive` · File phụ trách: `src/core/PreemptiveScheduler.h`, `src/core/PreemptiveScheduler.cpp`

## 1. Nghiên cứu về phần mình đảm nhiệm

### 1.1. Vai trò của module Preemptive Scheduler
Module Preemptive Scheduler là **bộ não xử lý thuật toán** cho biến thể lập lịch ưu tiên độc quyền:
- **Đầu vào:** Nhận danh sách tiến trình (đã được đọc và kiểm tra hợp lệ từ module CSV).
- **Đầu ra:** Trả về cấu trúc `ScheduleResult` chứa thông tin chi tiết của từng tiến trình (CT, TAT, WT, RT) và biểu đồ Gantt.
- **Cơ chế cốt lõi:** Nếu một tiến trình mới đến có độ ưu tiên cao hơn (số Priority nhỏ hơn) tiến trình đang chạy, CPU sẽ ngay lập tức bị thu hồi (preempt) để nhường cho tiến trình mới.

### 1.2. Quy ước chung của nhóm ảnh hưởng đến module
| Quy ước | Ảnh hưởng đến Scheduler |
|---|---|
| Priority nhỏ hơn = ưu tiên cao hơn | Thuật toán luôn tìm giá trị `priority` nhỏ nhất trong hàng đợi. |
| Cùng priority: AT nhỏ hơn được chọn trước; nếu vẫn bằng thì theo **thứ tự nhập ban đầu** | Cần duyệt danh sách theo thứ tự dòng CSV ban đầu để làm tiêu chí phụ (tie-breaker). |
| Không có tiến trình sẵn sàng thì CPU ở trạng thái IDLE | Ghi nhận đoạn Gantt với `pid = -1` đến thời điểm AT gần nhất. |
| Không có chi phí chuyển ngữ cảnh (Context switch) | Thời gian chạy các tiến trình nối tiếp nhau không bị cộng thêm delay. |
| Mọi trường thời gian là `long long` | Tránh tràn số khi thời gian chờ (WT) quá lớn. |

### 1.3. Các vấn đề kỹ thuật cần xử lý
- **Lưu trạng thái thời gian còn lại (Remaining Burst Time):** Do bị ngắt quãng liên tục, cần một mảng phụ `remainingBurst` để lưu thời gian còn lại của từng tiến trình.
- **Ghi nhận `startTime`:** Chỉ ghi nhận thời điểm bắt đầu (để tính Response Time) ở lần đầu tiên tiến trình được cấp CPU.
- **Gộp đoạn Gantt:** Nếu cùng một tiến trình chạy ở 2 thời điểm liên tiếp (hoặc CPU IDLE liên tiếp), phải gộp lại thành một đoạn duy nhất trong mảng `gantt` (theo mục 7 của conventions).
- **Hiệu năng:** Không mô phỏng từng đơn vị thời gian. Cài đặt dùng **hàng đợi ưu tiên (min-heap)** cho ready queue và **nhảy theo sự kiện** (arrival / preemption / hết burst), nhờ đó 100.000 tiến trình chạy dưới 1 giây (xem mục 7).

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần

| Thành phần | Nhiệm vụ |
|---|---|
| `PreemptiveScheduler.h` | Khai báo class `PreemptiveScheduler` với 1 hàm public duy nhất là `schedule()`. |
| `PreemptiveScheduler.cpp` | Cài đặt logic thuật toán: min-heap theo priority, xử lý ngắt quãng theo sự kiện, ghi Gantt và các chỉ số. |
| `Process`, `ScheduleResult`, `GanttSegment` | Các cấu trúc dữ liệu dùng chung (đã được định nghĩa trong thư mục `core/`). |

### Chức năng chính của PreemptiveScheduler
1. **Tìm tiến trình ưu tiên:** Duy trì ready queue bằng `std::priority_queue` (min-heap) theo khóa `(priority, arrivalTime, thứ tự input)`; mỗi lần chỉ lấy phần tử đầu hàng.
2. **Xử lý ngắt quãng (Preemption):** Tiến trình hiện tại chạy tới **sự kiện kế tiếp** — hết burst, có arrival mới, hoặc bị arrival có priority nhỏ hơn ngắt. Không lặp theo từng đơn vị thời gian.
3. **Cập nhật Gantt Chart:** Ghi nhận các mốc thời gian (`startTime`, `endTime`) của từng đoạn chạy, gộp các đoạn liền kề cùng PID (kể cả IDLE `pid = -1`).
4. **Trả kết quả (Không tính trung bình):** Tính CT, TAT, WT, RT cho từng tiến trình. **Không** tự tính các giá trị trung bình (việc này do module `Metrics` đảm nhận theo quy định).

## 3. Hướng dẫn sử dụng

### 3.1. Cách gọi hàm
```cpp
#include "core/PreemptiveScheduler.h"
#include "core/Process.h"
#include <vector>

int main() {
    // Giả sử processes đã được đọc từ CsvHandler
    std::vector<Process> processes = {
        {1, 0, 8, 3}, // pid=1, AT=0, BT=8, Pri=3
        {2, 1, 4, 1}, // pid=2, AT=1, BT=4, Pri=1
        {3, 2, 2, 2}  // pid=3, AT=2, BT=2, Pri=2
    };

    // Gọi hàm static của class
    ScheduleResult result = PreemptiveScheduler::schedule(processes);
    
    // result.processes chứa danh sách ProcessResult (CT, TAT, WT, RT)
    // result.gantt chứa các đoạn GanttSegment
    return 0;
}
```

- **Đầu vào:** Danh sách các tiến trình (cấu trúc `Process`).
- **Đầu ra:** Trả về `ScheduleResult`. Không ném exception, không in ra màn hình, không đọc/ghi file.

### 3.2. Ví dụ minh họa đầy đủ
Dữ liệu vào: P1 (AT 0, BT 8, Pri 3), P2 (AT 1, BT 4, Pri 1), P3 (AT 2, BT 2, Pri 2).

- **Diễn biến:**
  - `t=0`: P1 đến, chạy P1 (Pri 3).
  - `t=1`: P2 đến (Pri 1 < 3). Ngắt P1, chạy P2 (Pri 1). P1 còn 7.
  - `t=2`: P3 đến (Pri 2 > 1). P2 tiếp tục chạy.
  - `t=5`: P2 xong (đã chạy 4). Ready queue: P1 (Pri 3), P3 (Pri 2). Chọn P3 chạy.
  - `t=7`: P3 xong (đã chạy 2). Chọn P1 chạy (còn 7).
  - `t=14`: P1 xong. Kết thúc.

- **Gantt dự kiến:** `P1[0-1] -> P2[1-5] -> P3[5-7] -> P1[7-14]`.

## 4. Flow hoạt động

### 4.1. Luồng xử lý thuật toán (bên trong hàm `schedule`)
1. **Khởi tạo:**
   - Nếu danh sách rỗng, trả `ScheduleResult` rỗng ngay.
   - Tạo `result.processes` kích thước `n`; copy `remainingBurst = burstTime`; gán `startTime = -1` cho mọi tiến trình.
   - Tạo mảng chỉ số `arrivals` `0..n-1` và `stable_sort` theo `arrivalTime` (giữ nguyên thứ tự input khi AT bằng nhau).
   - Khởi tạo ready queue là `std::priority_queue` (min-heap) với khóa so sánh: `priority` nhỏ hơn → `arrivalTime` nhỏ hơn → chỉ số input nhỏ hơn.
   - `currentTime = 0`, `nextArrivalIdx = 0`, `completed = 0`.
2. **Vòng lặp (chạy đến khi `completed == n`):**
   - **Bước 1 - Nạp arrival:** Đẩy mọi tiến trình có `arrivalTime <= currentTime` vào ready queue.
   - **Bước 2 - Xử lý IDLE:** Nếu ready queue rỗng, nhảy `currentTime` đến `arrivalTime` của tiến trình kế tiếp, ghi/gộp đoạn Gantt `pid = -1` từ `currentTime` cũ đến mốc mới, rồi quay lại Bước 1.
   - **Bước 3 - Chọn tiến trình:** Lấy `curr = ready.top()` và pop khỏi hàng đợi.
   - **Bước 4 - Xác định sự kiện kế tiếp:** Đặt `nextEventTime = currentTime + remainingBurst[curr]`, rồi duyệt các arrival có `arrivalTime < nextEventTime`:
     - Nếu arrival có `priority` nhỏ hơn `priority` của `curr`: đặt `nextEventTime = arrivalTime` và dừng duyệt (bị ngắt).
     - Ngược lại (priority bằng hoặc lớn hơn): đẩy arrival vào ready queue và duyệt tiếp.
     - Dùng điều kiện `<` để arrival đúng biên `nextEventTime` không tạo đoạn 0 giây; arrival đó sẽ được nạp ở vòng lặp sau.
   - **Bước 5 - Thực thi:** Nếu `startTime[curr] == -1` thì gán `startTime[curr] = currentTime`. Giảm `remainingBurst[curr] -= nextEventTime - currentTime`, cập nhật `currentTime = nextEventTime`, ghi/gộp đoạn Gantt `[start, end)` (trùng PID với đoạn trước thì nới `endTime`).
   - **Bước 6 - Kiểm tra hoàn thành:**
     - Nếu `remainingBurst[curr] == 0`: Tính `CT = currentTime`, `TAT = CT - AT`, `WT = TAT - BT`, `RT = startTime - AT`; lưu `ProcessResult`; tăng `completed`.
     - Ngược lại: đẩy `curr` trở lại ready queue.
3. **Kết thúc:** Trả về `ScheduleResult` (ba trường trung bình để mặc định `0.0`, không tính ở đây).

### 4.2. Luồng tổng thể của chương trình

```text
                  Danh sách tiến trình (đã được CsvHandler đọc và kiểm tra)
                         │
                         ▼
            PreemptiveScheduler::schedule()
                         │
                         ├──► Khởi tạo remainingBurst, startTime = -1, currentTime = 0
                         ├──► Sắp xếp chỉ số theo AT (stable_sort)
                         ├──► Ready queue = min-heap (priority, AT, thứ tự input)
                         │
                         ▼
            [Vòng lặp: completed < n]
                         │
                         ├──► Nạp mọi arrival có AT <= currentTime vào ready
                         │
                         ├──► Nếu ready rỗng:
                         │       - Nhảy currentTime đến AT gần nhất
                         │       - Ghi/gộp Gantt IDLE (-1)
                         │
                         ├──► Nếu có: curr = ready.top()
                         │       - nextEventTime = currentTime + remainingBurst[curr]
                         │       - Duyệt arrival có AT < nextEventTime:
                         │           + priority nhỏ hơn curr → ngắt tại AT
                         │           + ngược lại → đẩy vào ready
                         │       - Ghi startTime (lần đầu), chạy tới nextEventTime
                         │       - Ghi/gộp GanttSegment (gộp nếu trùng PID)
                         │
                         └──► Nếu remainingBurst[curr] == 0:
                                 - Tính CT, TAT, WT, RT
                                 - Lưu ProcessResult, completed++
                              Ngược lại: đẩy curr lại ready
                         │
                         ▼
                  ScheduleResult (trả về)
                         │
       ┌─────────────────┴─────────────────┐
       ▼                                   ▼
Metrics (tính trung bình)         CSV / UI (ghi file, vẽ biểu đồ)
```

### 4.3. Cách các thành phần tương tác
- **CSV → Scheduler:** Truyền danh sách tiến trình; Scheduler không quan tâm dữ liệu đến từ file hay nhập tay.
- **Scheduler → Metrics/CSV/UI:** Trả `ScheduleResult` đã có các thông số CT, TAT, WT, RT và Gantt. Scheduler **không tính trung bình**, module `Metrics` sẽ nhận `ScheduleResult` này để tính các giá trị `averageWaitingTime`, v.v.

## 5. Cấu trúc và thiết kế

### 5.1. Cấu trúc file
```
src/core/
  PreemptiveScheduler.h
  PreemptiveScheduler.cpp
tests/unit/preemptive/
  preemptive_test.cpp          # P01–P18 (doctest)
src/
  benchmark_main.cpp           # target benchmark_runner, đo hiệu năng theo file CSV
```

### 5.2. Thiết kế Class (Tuân thủ tuyệt đối mục 4 - conventions.md)
```cpp
class PreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
```

### 5.3. Quyết định thiết kế (Đã thống nhất theo conventions.md)
- **Hàm `static`:** Vì thuật toán không cần lưu trữ trạng thái (stateless), dùng hàm static giúp gọi hàm dễ dàng từ `main` hoặc từ test mà không cần khởi tạo object.
- **Không phụ thuộc Qt/File:** Module này hoàn toàn "trong sạch", chỉ nhận dữ liệu từ danh sách đầu vào và trả về kết quả, đảm bảo tính đóng gói và dễ test độc lập.
- **Không tự tính trung bình:** Scheduler chỉ tính toán các chỉ số của từng tiến trình. Việc tính điểm trung bình (Avg WT, Avg TAT, Avg RT) được giao lại hoàn toàn cho module `Metrics` để đảm bảo đúng phân vai (mục 6 - conventions.md).
- **Hàng đợi ưu tiên + nhảy sự kiện:** Ready queue dùng `std::priority_queue` thay vì quét toàn bộ danh sách mỗi tick; mỗi tiến trình chỉ được đẩy/pop `O(log n)`. Thời gian được nhảy thẳng tới sự kiện kế tiếp (arrival, preemption hoặc hết burst) thay vì cộng từng đơn vị, nhờ đó độ phức tạp giảm từ `O(n × T)` xuống `O(n log n + g)` (`g` là số đoạn Gantt).
- **Xử lý IDLE thông minh:** Khi ready queue rỗng, thuật toán nhảy `currentTime` đến ngay `arrivalTime` của tiến trình tiếp theo và ghi một đoạn Gantt `pid = -1`, không tăng thời gian từng đơn vị.
- **Benchmark tách riêng:** `benchmark_runner` chỉ đo thời gian `schedule()`, không ảnh hưởng hợp đồng của module.

## 6. Kiểm thử độc lập

Test đặt trong `tests/unit/preemptive/preemptive_test.cpp`, sử dụng doctest. Các case P01–P15 và P18 kiểm tra **từng đoạn Gantt** (pid, start, end) cùng CT/TAT/WT/RT và averages; P16–P17 kiểm tra số đoạn Gantt, mốc IDLE đầu, tổng thời gian phủ và các bất biến.

| Case | Dữ liệu | Kỳ vọng chính |
|---|---|---|
| P01, P02 | `sample.csv` | Gantt và kết quả khớp `ket_qua/preemptive/sample_p_*.csv`; averages 7.80 / 11.30 / 5.30; P1 bị ngắt nhưng `RT = 0`. |
| P03 | `preempt_same_priority.csv` | Priority bằng nhau không bị ngắt; averages 3.80 / 5.50 / 3.80. |
| P04 | `multi_preempt.csv` | Nhiều lần ngắt, các đoạn bị cắt không bị gộp; averages 3.20 / 5.60 / 0.30. |
| P05 | `preempt_boundary.csv` | Arrival đúng biên không sinh đoạn 0 giây; averages 1.80 / 4.00 / 1.20. |
| P06, P07 | `idle_start.csv`, `idle_middle.csv` | Gantt có đoạn `-1`, IDLE đầu/giữa đúng mốc; averages lần lượt 4.10 / 5.80 / 3.90 và 1.20 / 2.80 / 1.20. |
| P08 | `priority_order.csv` | Priority nhỏ hơn thắng; averages 18.00 / 22.00 / 18.00. |
| P09 | `tie_arrival.csv` | Hòa priority xét AT nhỏ hơn; averages 2.00 / 4.10 / 2.00. |
| P10 | `unsorted_arrival.csv` | Không sort input; kết quả giữ thứ tự input; averages 0.50 / 2.50 / 0.30. |
| P11 | `preempt_fcfs.csv` | Cùng priority chạy lần lượt; averages 4.80 / 6.70 / 4.80. |
| P12 | `preempt_staircase.csv` | Bị ngắt 9 lần, P1 đúng 2 đoạn tổng 15; averages 0.90 / 3.30 / 0.00. |
| P13 | `preempt_alternate.csv` | P1 bị ngắt 5 lần, có 6 đoạn tổng 10; averages 1.30 / 3.60 / 0.00. |
| P14 | `tie_same_arrival.csv` | Hòa priority và AT thì theo thứ tự input (30, 10, 20, ...), không sort theo pid; averages 13.30 / 15.80 / 13.30. |
| P15 | `{}` | `processes` và `gantt` rỗng, averages `0.0`. |
| P16 | `p_large_100.csv` | Đủ 100 process, IDLE đầu `-1[0-2]`, tổng phủ 594, Gantt 109 đoạn > 100. |
| P17 | `p_large_1000.csv` | Đủ 1000 process, đoạn cuối kết thúc ở 5546, Gantt 1069 đoạn > 1000. |
| P18 | `sample_20.csv` | Khớp fixture, Gantt 25 đoạn phủ `[0-80]`; averages 21.60 / 25.60 / 18.45. |

Bất biến được kiểm tra ở mọi case:
- Kết quả giữ đúng thứ tự input; `WT = TAT - BT`, `TAT = CT - AT`, `RT = ST - AT`.
- Gantt phủ kín `[0, max(CT)]`, liên tục, không đoạn 0 giây, đoạn liền kề cùng PID đã gộp.
- Ba trường `average*` do scheduler để mặc định `0.0`; test gọi `Metrics::fillAverages()` khi cần so sánh averages.

## 7. Tiến độ và kết quả
- [x] Nghiên cứu thuật toán, hoàn thiện tài liệu và cập nhật đúng quy ước `conventions.md`.
- [x] Cài đặt `PreemptiveScheduler.cpp` với ready queue là min-heap và mô phỏng theo sự kiện.
- [x] Viết test P01–P18 bằng doctest, kiểm tra chi tiết Gantt và các chỉ số.
- [x] Tối ưu hiệu năng: 100.000 tiến trình khoảng **0,3–0,5 giây** (trước tối ưu khoảng 311 giây); 500.000 tiến trình khoảng 2,8 giây (bản Debug).
- [x] Giữ nguyên chữ ký `schedule()` và kết quả `sample.csv`: **7.80 / 11.30 / 5.30**.
- [x] Tạo PR #37 (`feature/preemptive` → `main`).

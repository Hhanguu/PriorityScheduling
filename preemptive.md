# TÀI LIỆU MODULE PREEMPTIVE SCHEDULER – PRIORITY SCHEDULING

Nhánh: `feature/preemptive` · File phụ trách: `src/core/PreemptiveScheduler.h`, `src/core/PreemptiveScheduler.cpp`

## 1. Nghiên cứu về phần mình đảm nhiệm

### 1.1. Vai trò của module Preemptive Scheduler
Module Preemptive Scheduler là **bộ não xử lý thuật toán** cho biến thể lập lịch ưu tiên độc quyền:
<<<<<<< HEAD
- **Đầu vào:** Nhận danh sách tiến trình (đã được đọc và kiểm tra hợp lệ từ module CSV).
=======
- **Đầu vào:** Nhận danh sách `std::vector<Process>` (đã được đọc và kiểm tra hợp lệ từ module CSV).
>>>>>>> main
- **Đầu ra:** Trả về cấu trúc `ScheduleResult` chứa thông tin chi tiết của từng tiến trình (CT, TAT, WT, RT) và biểu đồ Gantt.
- **Cơ chế cốt lõi:** Nếu một tiến trình mới đến có độ ưu tiên cao hơn (số Priority nhỏ hơn) tiến trình đang chạy, CPU sẽ ngay lập tức bị thu hồi (preempt) để nhường cho tiến trình mới.

### 1.2. Quy ước chung của nhóm ảnh hưởng đến module
| Quy ước | Ảnh hưởng đến Scheduler |
|---|---|
<<<<<<< HEAD
| Priority nhỏ hơn = ưu tiên cao hơn | Thuật toán luôn tìm giá trị `priority` nhỏ nhất trong hàng đợi. |
| Cùng priority: AT nhỏ hơn được chọn trước; nếu vẫn bằng thì theo **thứ tự nhập ban đầu** | Cần duyệt danh sách theo thứ tự dòng CSV ban đầu để làm tiêu chí phụ (tie-breaker). |
| Không có tiến trình sẵn sàng thì CPU ở trạng thái IDLE | Ghi nhận đoạn Gantt với `pid = -1` đến thời điểm AT gần nhất. |
| Không có chi phí chuyển ngữ cảnh (Context switch) | Thời gian chạy các tiến trình nối tiếp nhau không bị cộng thêm delay. |
=======
| Priority càng nhỏ thì ưu tiên càng cao | Thuật toán luôn tìm giá trị `priority` nhỏ nhất trong hàng đợi. |
| Cùng priority: AT nhỏ hơn được chọn trước; nếu vẫn bằng thì theo **thứ tự nhập** | Cần lưu lại thứ tự ban đầu của tiến trình để làm tiêu chí phụ (tie-breaker). |
| Không có tiến trình sẵn sàng thì CPU ở trạng thái IDLE | Khi không tìm thấy tiến trình nào thỏa mãn `arrivalTime <= currentTime`, ghi nhận đoạn Gantt với `pid = -1` và tăng thời gian. |
| Hai thuật toán dùng chung `Process` và trả về `ScheduleResult` | Module chỉ làm việc với hai cấu trúc này, tuyệt đối không tự định nghĩa lại. |
>>>>>>> main
| Mọi trường thời gian là `long long` | Tránh tràn số khi thời gian chờ (WT) quá lớn. |

### 1.3. Các vấn đề kỹ thuật cần xử lý
- **Lưu trạng thái thời gian còn lại (Remaining Burst Time):** Do bị ngắt quãng liên tục, cần một mảng phụ để lưu `remainingBurstTime` cho từng tiến trình.
- **Ghi nhận `startTime`:** Chỉ ghi nhận thời điểm bắt đầu (để tính Response Time) ở lần đầu tiên tiến trình được cấp CPU.
<<<<<<< HEAD
- **Gộp đoạn Gantt:** Nếu cùng một tiến trình chạy ở 2 thời điểm liên tiếp (hoặc CPU IDLE liên tiếp), phải gộp lại thành một đoạn duy nhất trong mảng `gantt` (theo mục 7 của conventions).
=======
- **Gộp đoạn Gantt:** Nếu cùng một tiến trình chạy ở 2 thời điểm liên tiếp (hoặc CPU IDLE liên tiếp), phải gộp lại thành một đoạn duy nhất trong mảng `gantt`.
>>>>>>> main

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần

| Thành phần | Nhiệm vụ |
|---|---|
| `PreemptiveScheduler.h` | Khai báo class `PreemptiveScheduler` với 1 hàm public duy nhất là `schedule()`. |
<<<<<<< HEAD
| `PreemptiveScheduler.cpp` | Cài đặt logic thuật toán: vòng lặp tìm tiến trình ưu tiên cao nhất, xử lý ngắt quãng. |
| `Process`, `ScheduleResult`, `GanttSegment` | Các cấu trúc dữ liệu dùng chung (đã được định nghĩa trong thư mục `core/`). |

### Chức năng chính của PreemptiveScheduler
1. **Tìm tiến trình ưu tiên:** Tìm trong danh sách các tiến trình đã đến và chưa chạy xong, chọn ra tiến trình có Priority nhỏ nhất (xử lý tie-breaker nếu bằng nhau).
2. **Xử lý ngắt quãng (Preemption):** Chạy tiến trình trong 1 đơn vị thời gian. Sau đó kiểm tra lại xem có tiến trình mới nào đến với Priority cao hơn không để ngắt.
3. **Cập nhật Gantt Chart:** Ghi nhận các mốc thời gian (`startTime`, `endTime`) của từng tiến trình, gộp các đoạn liền kề cùng PID.
4. **Trả kết quả (Không tính trung bình):** Tính CT, TAT, WT, RT cho từng tiến trình. **Không** tự tính các giá trị trung bình (việc này do module `Metrics` đảm nhận theo quy định).
=======
| `PreemptiveScheduler.cpp` | Cài đặt logic thuật toán: vòng lặp tìm tiến trình ưu tiên cao nhất, xử lý ngắt quãng, tính toán các chỉ số. |
| `Process`, `ScheduleResult`, `GanttSegment` | Các cấu trúc dữ liệu dùng chung (đã được định nghĩa trong thư mục `core/`). |

### Chức năng chính của PreemptiveScheduler
1. **Tìm tiến trình ưu tiên (findNextProcess):** Tìm trong danh sách các tiến trình đã đến và chưa chạy xong, chọn ra tiến trình có Priority nhỏ nhất (xử lý tie-breaker nếu bằng nhau).
2. **Xử lý ngắt quãng (Preemption):** Chạy tiến trình trong 1 đơn vị thời gian. Sau đó kiểm tra lại xem có tiến trình mới nào đến với Priority cao hơn không để ngắt.
3. **Cập nhật Gantt Chart:** Ghi nhận các mốc thời gian (`startTime`, `endTime`) của từng tiến trình, gộp các đoạn liền kề.
4. **Tính toán kết quả (Metrics):** Tính CT, TAT, WT, RT cho từng tiến trình sau khi hoàn thành.
>>>>>>> main

## 3. Hướng dẫn sử dụng

### 3.1. Cách gọi hàm
```cpp
#include "core/PreemptiveScheduler.h"
#include "core/Process.h"
#include <vector>
<<<<<<< HEAD
=======
#include <iostream>
>>>>>>> main

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

<<<<<<< HEAD
- **Đầu vào:** Danh sách các tiến trình (cấu trúc `Process`).
=======
- **Đầu vào:** `const std::vector<Process>&`.
>>>>>>> main
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

<<<<<<< HEAD
### 4.1. Luồng xử lý thuật toán (bên trong hàm `schedule`)
1. **Khởi tạo:** 
   - Nhận danh sách tiến trình. Khởi tạo `currentTime = 0`, `completed = 0`.
   - Tạo mảng `remainingBurstTime` copy từ `burstTime`.
   - Tạo mảng `startTime` gán bằng `-1` (để đánh dấu chưa chạy lần nào).
2. **Vòng lặp (chạy đến khi `completed == n`):**
   - **Bước 1 - Tìm tiến trình:** Duyệt danh sách, tìm tiến trình thỏa mãn `arrivalTime <= currentTime` và `remainingBurstTime > 0`. Áp dụng quy tắc: `priority` nhỏ nhất -> `arrivalTime` nhỏ nhất -> thứ tự nhập ban đầu (thứ tự dòng CSV).
   - **Bước 2 - Xử lý IDLE:** Nếu không tìm thấy tiến trình nào (CPU rảnh), tìm thời điểm `arrivalTime` nhỏ nhất của các tiến trình chưa chạy. Ghi nhận đoạn Gantt `pid = -1` từ `currentTime` đến `arrivalTime` đó. Cập nhật `currentTime = arrivalTime`. Quay lại Bước 1.
   - **Bước 3 - Thực thi:**
     - Nếu `startTime` của tiến trình đang là `-1`, gán `startTime = currentTime`.
     - Giảm `remainingBurstTime` đi 1.
     - Tăng `currentTime` lên 1.
     - Ghi nhận đoạn Gantt cho tiến trình này (nếu PID trùng với đoạn ngay trước đó thì nới rộng `endTime`, nếu không thì tạo đoạn mới).
   - **Bước 4 - Kiểm tra hoàn thành:**
     - Nếu `remainingBurstTime == 0`: Tính `CT = currentTime`, `TAT = CT - AT`, `WT = TAT - BT`, `RT = startTime - AT`.
     - Lưu kết quả vào danh sách `ProcessResult`.
     - Tăng `completed` lên 1.
3. **Kết thúc:** Trả về đối tượng `ScheduleResult` (Các trường trung bình để mặc định, không tính ở đây).

### 4.2. Luồng tổng thể của chương trình

```text
                  Danh sách tiến trình (đã được CsvHandler đọc và kiểm tra)
                         │
                         ▼
            PreemptiveScheduler::schedule()
                         │
                         ├──► Khởi tạo mảng remainingBurstTime, currentTime = 0
                         │
                         ▼
            [Vòng lặp: completed < n]
                         │
                         ├──► Tìm tiến trình có Priority nhỏ nhất (AT <= currentTime)
                         │
                         ├──► Nếu không có: 
                         │       - Nhảy currentTime đến AT gần nhất
                         │       - Ghi Gantt IDLE (-1)
                         │
                         ├──► Nếu có: 
                         │       - Ghi nhận startTime (lần đầu chạy)
                         │       - Chạy 1 đơn vị: remainingBurstTime--, currentTime++
                         │       - Ghi nhận GanttSegment (gộp nếu trùng PID)
                         │
                         └──► Nếu remainingBurstTime == 0:
                                 - Tính CT, TAT, WT, RT
                                 - Lưu vào ProcessResult, completed++
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
=======
### 4.1. Luồng xử lý thuật toán (Bên trong hàm `schedule`)
1. Khởi tạo `currentTime = 0`, `completed = 0`. Tạo mảng `remainingBurstTime` bằng `burstTime` ban đầu.
2. **Vòng lặp (while `completed < n`):**
   - **Bước 1:** Duyệt tìm tiến trình thỏa mãn `arrivalTime <= currentTime` và `remainingBurstTime > 0`. Ưu tiên `priority` nhỏ nhất -> `arrivalTime` nhỏ nhất -> `pid` nhỏ nhất.
   - **Bước 2:** Nếu không tìm thấy -> Ghi nhận Gantt là IDLE (`pid = -1`), `currentTime++`. Quay lại Bước 1.
   - **Bước 3:** Nếu tìm thấy tiến trình `P`:
     - Nếu `P` chạy lần đầu (`startTime == -1`), gán `startTime = currentTime`.
     - Ghi nhận vào Gantt (gộp đoạn nếu trùng `pid`).
     - Giảm `remainingBurstTime` của `P` đi 1. Tăng `currentTime` lên 1.
   - **Bước 4:** Nếu `remainingBurstTime == 0`:
     - Tính `completionTime = currentTime`.
     - Tính `TAT`, `WT`, `RT`.
     - Thêm vào danh sách kết quả `ProcessResult`. `completed++`.
3. Kết thúc vòng lặp, trả về `ScheduleResult`.

### 4.2. Cách các thành phần tương tác
- **CSV → Scheduler:** Truyền `vector<Process>`. Scheduler không cần biết dữ liệu đến từ đâu.
- **Scheduler → CSV/UI:** Trả về `ScheduleResult` đã được tính toán đầy đủ. Các module khác chỉ việc lấy dữ liệu này để ghi file hoặc vẽ biểu đồ.
>>>>>>> main

## 5. Cấu trúc và thiết kế

### 5.1. Cấu trúc file
```
src/core/
  PreemptiveScheduler.h
  PreemptiveScheduler.cpp
```

<<<<<<< HEAD
### 5.2. Thiết kế Class (Tuân thủ tuyệt đối mục 4 - conventions.md)
=======
### 5.2. Thiết kế Class
>>>>>>> main
```cpp
class PreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
```
<<<<<<< HEAD

### 5.3. Quyết định thiết kế (Đã thống nhất theo conventions.md)
- **Hàm `static`:** Vì thuật toán không cần lưu trữ trạng thái (stateless), dùng hàm static giúp gọi hàm dễ dàng từ `main` hoặc từ test mà không cần khởi tạo object.
- **Không phụ thuộc Qt/File:** Module này hoàn toàn "trong sạch", chỉ nhận dữ liệu từ danh sách đầu vào và trả về kết quả, đảm bảo tính đóng gói và dễ test độc lập.
- **Không tự tính trung bình:** Scheduler chỉ tính toán các chỉ số của từng tiến trình. Việc tính điểm trung bình (Avg WT, Avg TAT, Avg RT) được giao lại hoàn toàn cho module `Metrics` để đảm bảo đúng phân vai (mục 6 - conventions.md).
- **Xử lý IDLE thông minh:** Theo quy tắc số 6, thay vì tăng thời gian lên từng đơn vị một khi không có tiến trình nào chạy, thuật toán sẽ nhảy cóc (`jump`) thời gian đến ngay thời điểm `arrivalTime` của tiến trình tiếp theo để tối ưu hiệu suất.
=======
- **Quyết định thiết kế:** Dùng hàm `static` vì Scheduler không cần lưu trữ trạng thái (stateless). Việc này giúp dễ gọi và test độc lập.

### 5.3. Điểm cần thống nhất với nhóm
1. Thuật toán xử lý tie-breaker: Nếu Priority bằng nhau, xét AT. Nếu AT bằng nhau, dựa vào thứ tự trong `vector<Process>` (tương ứng thứ tự dòng trong file CSV) hay dựa vào `pid`? (Theo quy ước `conventions.md` là dựa vào thứ tự trong vector).
2. Có cần cộng thêm thời gian chuyển ngữ cảnh (context switch) khi bị ngắt không? (Theo quy ước: Không có chi phí chuyển ngữ cảnh).
>>>>>>> main

## 6. Kế hoạch kiểm thử độc lập

Test đặt trong `tests/unit/PreemptiveScheduler_test.cpp`, sử dụng doctest.

| Trường hợp | Kỳ vọng |
|---|---|
<<<<<<< HEAD
| Danh sách rỗng | Trả về `ScheduleResult` rỗng. |
| Input bình thường (không bị ngắt) | Priority giảm dần theo thời gian, Gantt khớp, các chỉ số CT/TAT/WT/RT đúng. |
| Có bị ngắt quãng (Preemption) | Tiến trình đang chạy bị dừng, Gantt thể hiện rõ các đoạn bị cắt, kết quả TAT/WT khớp với tính toán bằng tay. |
| CPU rảnh (IDLE) | Gantt có đoạn `pid = -1`, thời gian `currentTime` nhảy đúng đến AT gần nhất. |
| Hòa Priority | Xét tiếp AT nhỏ hơn, nếu vẫn hòa thì theo thứ tự nhập ban đầu (thứ tự dòng CSV). |
=======
| Mảng rỗng | Trả về `ScheduleResult` rỗng. |
| Input bình thường (không bị ngắt) | Priority giảm dần theo thời gian, Gantt khớp, các chỉ số CT/TAT/WT/RT đúng. |
| Có bị ngắt quãng (Preemption) | Tiến trình đang chạy bị dừng, Gantt thể hiện rõ các đoạn bị cắt, kết quả TAT/WT khớp với tính toán bằng tay. |
| CPU rảnh (IDLE) | Gantt có đoạn `pid = -1`, thời gian chờ được cộng dồn chính xác. |
| Hòa Priority | Xét tiếp AT nhỏ hơn, nếu vẫn hòa thì theo thứ tự trong vector. |
>>>>>>> main

## 7. Tiến độ
- [x] Nghiên cứu thuật toán, hoàn thiện tài liệu và cập nhật đúng quy ước `conventions.md` (tuần này).
- [ ] Cài đặt code `PreemptiveScheduler.cpp` (tuần sau).
- [ ] Viết test độc lập bằng doctest.
- [ ] Tạo PR vào nhánh `main`.
<<<<<<< HEAD
=======
```
>>>>>>> main

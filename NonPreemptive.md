# Tài liệu module Non-preemptive Priority Scheduling

**Phần phụ trách:** Lập lịch CPU theo độ ưu tiên không ngắt.  
**Giai đoạn:** Viết tài liệu thiết kế 

## 1. Module này làm gì?

Module nhận danh sách các tiến trình (process) và quyết định **CPU chạy tiến trình nào, chạy từ lúc nào đến lúc nào**.

**Non-preemptive (không ngắt)** nghĩa là: khi một tiến trình đã được CPU chọn, nó sẽ chạy **đến khi xong**, dù trong lúc đó có tiến trình ưu tiên cao hơn xuất hiện.

Module cần:
- Chọn tiến trình đúng quy tắc ưu tiên.
- Ghi khoảng **IDLE** khi CPU không có việc để chạy.
- Tính các chỉ số thời gian của từng tiến trình và giá trị trung bình.
- Trả về các đoạn thời gian để phần giao diện vẽ **Gantt Chart**.

## 2. Quy tắc chọn tiến trình

Chỉ chọn trong các tiến trình **đã đến** (`AT <= thời điểm hiện tại`) và **chưa hoàn thành**.

1. **Priority nhỏ hơn → ưu tiên cao hơn**. Ví dụ: Priority 1 được chọn trước Priority 3.
2. Nếu cùng Priority, **Arrival Time (AT) nhỏ hơn** được chọn trước.
3. Nếu cả Priority và AT đều bằng nhau, chọn theo **thứ tự nhập**.
4. Tiến trình đã chạy thì **không bị ngắt**.
5. Nếu chưa có tiến trình sẵn sàng, CPU **IDLE** đến thời điểm có tiến trình tiếp theo.

**Starvation (bị bỏ đói):** tiến trình có độ ưu tiên thấp có thể chờ rất lâu nếu liên tục có nhiều tiến trình ưu tiên cao được chọn trước. Có thể dùng *aging* (tăng ưu tiên theo thời gian chờ) để hạn chế, nhưng chưa nằm trong phần cài đặt dự kiến.

## 3. Đầu vào, đầu ra và cách dùng

**Đầu vào:** danh sách `Process`, mỗi tiến trình có:

| Trường | Ý nghĩa |
|---|---|
| `pid` | Mã tiến trình (P1, P2...) |
| `arrivalTime` (AT) | Thời điểm đến |
| `burstTime` (BT) | Thời gian CPU cần xử lý |
| `priority` | Độ ưu tiên |
| `inputOrder` | Thứ tự nhập để xử lý khi hòa |

**Cách gọi hàm dự kiến (C++):**

```cpp
ScheduleResult nonPreemptive(const vector<Process>& processes);
```

**Đầu ra:** `ScheduleResult` dùng chung với module Preemptive, gồm:
- Kết quả mỗi tiến trình: `ST`, `CT`, `TAT`, `WT`, `RT`.
- Danh sách đoạn Gantt: tên tiến trình (hoặc `IDLE`), thời điểm bắt đầu và kết thúc.
- Thời gian chờ, quay vòng và phản hồi trung bình.

### Công thức

| Ký hiệu | Ý nghĩa / công thức |
|---|---|
| AT | Thời điểm đến |
| BT | Thời gian CPU cần |
| ST | Thời điểm bắt đầu chạy lần đầu |
| CT | Thời điểm hoàn thành |
| TAT | `CT - AT` |
| WT | `TAT - BT` |
| RT | `ST - AT` |

Trong bài toán Non-preemptive đơn CPU, không có I/O và chi phí chuyển ngữ cảnh: **WT = RT**.

## 4. Luồng hoạt động (Flow)

```text
Nhận danh sách Process
         |
         v
Đặt thời gian hiện tại = 0
         |
         v
Còn tiến trình chưa chạy? -- Không --> Tính trung bình --> Trả kết quả
         |
        Có
         v
Có tiến trình đã đến? -- Không --> Ghi IDLE, nhảy đến AT gần nhất --+
         |                                                           |
        Có                                                           |
         v                                                           |
Chọn theo Priority -> AT -> thứ tự nhập                              |
         |                                                           |
         v                                                           |
Chạy đến hết BT (không ngắt)                                         |
         |                                                           |
         v                                                           |
Ghi Gantt, tính CT/TAT/WT/RT, cập nhật thời gian --------------------+
         |
         +--> Lặp lại
```

**Các bước chính:**
1. Xét các tiến trình đã đến và chưa chạy.
2. Chọn tiến trình ưu tiên cao nhất theo quy tắc ở mục 2.
3. Cho chạy hết Burst Time; lưu kết quả và đoạn Gantt.
4. Nếu không có tiến trình, thêm đoạn IDLE và chuyển đến AT kế tiếp.
5. Lặp lại đến khi tất cả hoàn thành, rồi tính các giá trị trung bình.

## 5. Ví dụ minh họa

| Process | AT | BT | Priority |
|---|---:|---:|---:|
| P1 | 0 | 5 | 3 |
| P2 | 1 | 3 | 1 |
| P3 | 2 | 2 | 2 |
| P4 | 4 | 4 | 1 |

- Lúc 0 chỉ có P1 nên P1 chạy từ **0 đến 5**. P2 đến lúc 1 nhưng **không thể ngắt P1**.
- Lúc 5, P2 và P4 cùng Priority 1; P2 đến sớm hơn nên chạy trước.
- Sau đó chạy P4 rồi P3.

```text
0       5       8        12    14
|  P1   |  P2   |   P4   | P3 |
```

| Process | ST | CT | TAT | WT | RT |
|---|---:|---:|---:|---:|---:|
| P1 | 0 | 5 | 5 | 0 | 0 |
| P2 | 5 | 8 | 7 | 4 | 4 |
| P3 | 12 | 14 | 12 | 10 | 10 |
| P4 | 8 | 12 | 8 | 4 | 4 |

**Trung bình:** WT = **4,5**; TAT = **8**; RT = **4,5**.

## 6. Cấu trúc và thiết kế dự kiến

```text
include/
  Process.h          # Thông tin tiến trình (dùng chung)
  ScheduleResult.h   # Kiểu kết quả (dùng chung)
  NonPreemptive.h    # Khai báo hàm
src/
  NonPreemptive.cpp  # Xử lý thuật toán
tests/
  test_nonpreemptive.cpp  # Kiểm thử riêng
docs/
  NonPreemptive.md   # Tài liệu này
```

- **`Process`**: lưu `pid`, `arrivalTime`, `burstTime`, `priority`, `inputOrder`.
- **`ScheduleResult`**: chứa kết quả từng tiến trình, các đoạn Gantt và số liệu trung bình.
- **`nonPreemptive(...)`**: nhận danh sách `Process`, xử lý và trả về `ScheduleResult`.
- **Tương tác:** Phần nhập dữ liệu → module Non-preemptive → phần hiển thị Gantt và bảng so sánh.

**Lưu ý:** Cấu trúc và tên hàm là đề xuất. Nhóm phải thống nhất `Process` và `ScheduleResult` trước khi viết code.

## 7. Kiểm thử dự kiến

Khi bắt đầu lập trình, cần kiểm tra tối thiểu:

- Một tiến trình duy nhất.
- Tiến trình ưu tiên cao xuất hiện khi CPU đang bận (**không được ngắt**).
- Cùng Priority → chọn AT nhỏ hơn; vẫn hòa → theo thứ tự nhập.
- CPU IDLE lúc bắt đầu hoặc giữa hai tiến trình.
- Kết quả CT, TAT, WT, RT và Gantt khớp ví dụ ở mục 5.

**Kết quả tuần này:** hoàn thành tài liệu mô tả, chưa yêu cầu viết code hay chạy test thực tế.

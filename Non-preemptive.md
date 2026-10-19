# Non-preemptive Priority Scheduling — Tài liệu module

**Phạm vi:** Module `NonPreemptiveScheduler` trong dự án PriorityScheduling.  
**Trạng thái:** Tài liệu thiết kế; không khẳng định đã triển khai hoặc kiểm thử.  
**Căn cứ:** `docs/conventions.md` (quy ước chung của nhóm).

## 1. Cấu trúc thư mục

Các vị trí **theo quy ước dự án** có liên quan đến module:

```text
PriorityScheduling/
├── src/
│   └── core/
│       ├── Process.h
│       ├── GanttSegment.h
│       ├── ScheduleResult.h
│       ├── NonPreemptiveScheduler.h
│       ├── NonPreemptiveScheduler.cpp
│       ├── Metrics.h
│       └── Metrics.cpp
├── tests/
│   └── unit/                 # Test độc lập từng module
├── samples/
│   └── sample.csv            # Bộ dữ liệu mẫu dùng chung
├── output/                   # Kết quả xuất CSV
└── docs/
    ├── conventions.md
    └── NonPreemptive.md
```

Đây là đường dẫn đã quy định trong `conventions.md`, **không phải xác nhận rằng toàn bộ file đã tồn tại trên `main`**.

## 2. Kiểu dữ liệu dùng chung

Module **dùng nguyên các kiểu chung**; không tự thêm trường hay đổi kiểu dữ liệu.

| Kiểu | Trường | Vai trò |
|---|---|---|
| `Process` | `int pid`, `long long arrivalTime`, `long long burstTime`, `int priority` | Dữ liệu tiến trình đầu vào |
| `GanttSegment` | `int pid`, `long long startTime`, `long long endTime` | Một khoảng thời gian CPU chạy tiến trình hoặc IDLE |
| `ProcessResult` | `int pid`, `long long completionTime`, `long long turnaroundTime`, `long long waitingTime`, `long long responseTime` | Kết quả của từng tiến trình |
| `ScheduleResult` | `processes`, `gantt`, `averageWaitingTime`, `averageTurnaroundTime`, `averageResponseTime` | Kết quả lập lịch tổng hợp |

- `pid > 0` và không trùng; `arrivalTime >= 0`; `burstTime > 0`.
- `pid = -1` trong `GanttSegment` biểu thị **IDLE**.
- Thứ tự nhập được giữ bằng **vị trí phần tử trong `std::vector<Process>`**, **không có trường `inputOrder`**.
- Các thời gian dùng `long long`; các giá trị trung bình dùng `double`.

## 3. Quy tắc lập lịch Non-preemptive

**Non-preemptive** nghĩa là khi CPU đã chọn một tiến trình, nó chạy liên tục **hết `burstTime`**, không bị tiến trình mới đến ngắt.

Ở mỗi lần CPU rảnh, chỉ xét các tiến trình đã đến (`arrivalTime <= thời điểm hiện tại`) và chưa chạy xong. Chọn theo thứ tự:

1. `priority` **nhỏ hơn** được ưu tiên hơn.
2. Nếu cùng `priority`, chọn `arrivalTime` **nhỏ hơn**.
3. Nếu vẫn hòa, chọn tiến trình xuất hiện **trước trong vector đầu vào** (thứ tự dòng CSV).
4. Nếu chưa có tiến trình sẵn sàng, ghi đoạn **IDLE** đến thời điểm đến gần nhất.

Mô hình **không có I/O, không tính chi phí chuyển ngữ cảnh**. *Starvation/aging nằm ngoài phạm vi, không cài đặt* theo quy ước nhóm.

**Luồng xử lý:**

```text
Nhận vector<Process>
        |
        v
Thời gian hiện tại = 0; chưa tiến trình nào hoàn thành
        |
        v
Còn tiến trình chưa xử lý? -- Không --> Trả ScheduleResult
        |
       Có
        v
Có tiến trình đã đến? -- Không --> Ghi IDLE; nhảy đến AT gần nhất --+
        |                                                            |
       Có                                                            |
        v                                                            |
Chọn theo priority → arrivalTime → vị trí trong vector               |
        |                                                            |
        v                                                            |
Chạy hết burstTime (không ngắt)                                      |
        |                                                            |
        v                                                            |
Lưu ProcessResult và GanttSegment; cập nhật thời gian ----------------+
```

## 4. Giao diện scheduler (API)

Theo `conventions.md`, scheduler có **một class và một hàm public duy nhất**:

```cpp
// src/core/NonPreemptiveScheduler.h
class NonPreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
```

**Cách gọi (minh họa cách sử dụng API, không phải code đã triển khai):**

```cpp
std::vector<Process> processes = {
    {1, 0, 5, 3},
    {2, 1, 3, 1},
    {3, 2, 2, 2},
    {4, 4, 4, 1}
};
ScheduleResult result = NonPreemptiveScheduler::schedule(processes);
```

Hàm chỉ nhận `vector<Process>` và trả `ScheduleResult`: **không đọc/ghi CSV, không phụ thuộc Qt, không vẽ giao diện**.

## 5. File CSV đầu vào

CSV do `CsvHandler` đọc, sau đó truyền danh sách `Process` cho scheduler. Header chuẩn:

```csv
pid,arrival,burst,priority
```

- `pid`: số nguyên dương, duy nhất.
- `arrival`: số nguyên không âm; `burst`: số nguyên dương; `priority`: số nguyên.
- Giữ nguyên thứ tự dòng CSV khi tạo vector để xử lý hòa.
- `CsvHandler` chịu trách nhiệm kiểm tra dữ liệu, xử lý lỗi và báo lỗi tiếng Việt kèm số dòng. Scheduler **không** xử lý việc đọc CSV.

## 6. Phân vai tính toán và kết quả

**`NonPreemptiveScheduler` phụ trách:** chọn tiến trình, xác định thứ tự chạy, tạo `gantt` và `ProcessResult` cho từng tiến trình.

Các công thức của mỗi tiến trình:

| Ký hiệu | Ý nghĩa | Công thức |
|---|---|---|
| `AT` | Thời điểm đến | `arrivalTime` |
| `BT` | Thời gian xử lý | `burstTime` |
| `ST` | Lần đầu được chạy | Dùng để tính RT, không là trường của `ProcessResult` |
| `CT` | Thời điểm hoàn thành | Thời điểm kết thúc chạy |
| `TAT` | Thời gian quay vòng | `CT - AT` |
| `WT` | Thời gian chờ | `TAT - BT` |
| `RT` | Thời gian phản hồi | `ST - AT` |

Trong mô hình không ngắt, không có I/O: **WT = RT**.

**`Metrics` phụ trách:** tính ba trung bình `averageWaitingTime`, `averageTurnaroundTime`, `averageResponseTime` và số liệu so sánh hai thuật toán. Scheduler không tự triển khai lại các phép tính trung bình.

**Ví dụ minh họa thuật toán** (bộ dữ liệu giải thích, **không khẳng định là nội dung `samples/sample.csv`**):

| PID | AT | BT | Priority |
|---:|---:|---:|---:|
| 1 | 0 | 5 | 3 |
| 2 | 1 | 3 | 1 |
| 3 | 2 | 2 | 2 |
| 4 | 4 | 4 | 1 |

Tại `t = 0` chỉ có P1, nên P1 chạy hết từ 0 đến 5. Đến `t = 5`, P2 và P4 cùng priority 1, P2 đến trước nên chạy trước. Sau đó là P4 và P3.

```text
0       5       8        12    14
|  P1   |  P2   |   P4   | P3 |
```

| PID | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|
| 1 | 5 | 5 | 0 | 0 |
| 2 | 8 | 7 | 4 | 4 |
| 3 | 14 | 12 | 10 | 10 |
| 4 | 12 | 8 | 4 | 4 |

Kết quả trung bình từ các dòng trên (do `Metrics` tính): **WT = 4,50**, **TAT = 8,00**, **RT = 4,50**.

## 7. File CSV đầu ra

`CsvHandler` xuất dữ liệu nhận được, **không tính toán lại**. Các file liên quan tới Non-preemptive:

| File | Nội dung |
|---|---|
| `output/np_result.csv` | Kết quả từng tiến trình |
| `output/np_gantt.csv` | Các đoạn Gantt |
| `output/comparison.csv` | So sánh trung bình NP và P (do phần tích hợp cung cấp) |

Định dạng kết quả:

```csv
pid,CT,TAT,WT,RT
```

Định dạng Gantt:

```csv
pid,start,end
```

Ghi `pid = -1` cho IDLE. Các đoạn Gantt liền kề cùng `pid` cần được gộp trước khi ghi hoặc hiển thị. File so sánh có header `metric,np,p`, một dòng cho mỗi trung bình và hiển thị hai chữ số thập phân.

## 8. Kiểm thử module

Theo quy ước nhóm, sử dụng **doctest** tại `tests/third_party/doctest.h`; test độc lập đặt ở `tests/unit/<module>_test.cpp`, không cần Qt hay scheduler khác. Test phải đạt trước khi mở Pull Request.

Các trường hợp cần kiểm tra cho Non-preemptive:

1. Một tiến trình duy nhất.
2. Có tiến trình ưu tiên cao đến khi CPU đang bận, nhưng tiến trình đang chạy **không bị ngắt**.
3. Cùng priority: ưu tiên AT nhỏ hơn; cùng cả hai: giữ thứ tự vector.
4. IDLE lúc ban đầu hoặc giữa các tiến trình.
5. Các đoạn Gantt và CT/TAT/WT/RT đúng; so sánh **expected** với **actual**.

**Trạng thái tài liệu:** Đây là danh sách test cần thực hiện sau, chưa tuyên bố test đã chạy.

## 9. Giao diện hiển thị (UI)

Module không tự vẽ UI. Phần `ui/` sẽ nhận `ScheduleResult` để:

- Hiển thị PID số nguyên `1` thành `P1`; `-1` thành `IDLE`.
- Hiển thị bảng `CT/TAT/WT/RT`, các giá trị trung bình và Gantt Chart.
- Cho phép người dùng chạy thuật toán và xuất kết quả bằng phần chức năng tích hợp.

## 10. Benchmark hiệu năng

Module Non-preemptive được phần `benchmark/` gọi với các bộ dữ liệu chuẩn của nhóm:

| File | Số tiến trình |
|---|---:|
| `samples/sample.csv` | 3 |
| `samples/perf_small.csv` | 20 |
| `samples/perf_medium.csv` | 100 |
| `samples/perf_large.csv` | 1000 |

`benchmark/` đo thời gian chạy scheduler (ms); `Metrics` cung cấp WT/TAT/RT trung bình. Kết quả benchmark được lưu ở `report/` theo quy ước, không trộn với `output/` của chương trình.

---

**Tóm tắt trách nhiệm:** Người làm Non-preemptive tập trung vào `NonPreemptiveScheduler` (xếp lịch, `ProcessResult`, `gantt`) và test độc lập. Đọc CSV, tính trung bình, vẽ UI, xuất file và benchmark thuộc các module được phân công riêng.

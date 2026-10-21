
## 1. Cấu trúc thư mục

```
PriorityScheduling/
  CMakeLists.txt
  src/
    main.cpp
    core/     Process.h, GanttSegment.h, ScheduleResult.h,
              NonPreemptiveScheduler.{h,cpp}, PreemptiveScheduler.{h,cpp},
              Metrics.{h,cpp}
    io/       CsvHandler.{h,cpp}
    ui/       MainWindow.{h,cpp}, GanttChart.{h,cpp}
    benchmark/ Benchmark.{h,cpp}
  tests/
    unit/         test độc lập từng module
    integration/  test nối nhiều module
  samples/   file CSV mẫu chuẩn (dữ liệu dùng chung cho doc + test)
  input/     dữ liệu người dùng
  output/    kết quả chương trình ghi ra
  docs/      conventions.md + <Module>.md cho từng module
  build/     không commit
```


## 2. Kiểu dữ liệu dùng chung

```cpp
struct Process {
    int pid;                 // > 0, duy nhất
    long long arrivalTime;   // AT >= 0
    long long burstTime;     // BT > 0
    int priority;            // số nhỏ = ưu tiên cao
};

struct GanttSegment {
    int pid;                 // -1 = IDLE
    long long startTime;
    long long endTime;       // endTime > startTime
};

struct ProcessResult {
    int pid;
    long long completionTime;    // CT
    long long turnaroundTime;    // TAT = CT - AT
    long long waitingTime;       // WT = TAT - BT
    long long responseTime;      // RT = ST - AT
};

struct ScheduleResult {
    std::vector<ProcessResult> processes;
    std::vector<GanttSegment>  gantt;
    double averageWaitingTime;
    double averageTurnaroundTime;
    double averageResponseTime;
};
```

Quy tắc:

- **`pid` là `int`**, không bao giờ là chuỗi. Muốn hiển thị "P1" thì UI tự ghép tiền tố `"P"` (mục 9).
- **IDLE = `pid = -1`** trong mọi cấu trúc và file CSV; chữ `"IDLE"` chỉ xuất hiện ở giao diện.
- **Mọi trường thời gian là `long long`.** `pid`, `priority` là `int`. Ba trung bình là `double`.
- Cấu trúc này là **tuyệt đối**: thêm/bớt trường phải do Leader sửa quy ước trước.

## 3. Quy tắc lập lịch (mọi scheduler)

Áp dụng giống hệt nhau cho cả Non-preemptive và Preemptive:

1. **Priority nhỏ hơn = ưu tiên cao hơn.**
2. Hòa priority → **Arrival Time nhỏ hơn** được chọn trước.
3. Hòa cả hai → **thứ tự trong `vector`** (thứ tự dòng CSV).
4. Chỉ xét tiến trình **đã đến** (`AT <= thời điểm hiện tại`) và **chưa hoàn thành**.
5. Non-preemptive: đã chọn thì chạy **hết BT**, không bị ngắt.
6. Không có tiến trình sẵn sàng → ghi đoạn **IDLE** đến thời điểm AT gần nhất.
7. Không có I/O, không có chi phí chuyển ngữ cảnh.

*Starvation/aging: ngoài phạm vi đề tài — không cài đặt.*

## 4. Giao diện scheduler

Mỗi biến thể đúng 1 class, 1 hàm public duy nhất:

```cpp
// src/core/NonPreemptiveScheduler.h
class NonPreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
// src/core/PreemptiveScheduler.h — tương tự
```

- Không phụ thuộc Qt, không đọc file — chỉ nhận `vector<Process>` và trả `ScheduleResult`.
- Không tính trung bình tự tính nếu không cần; số liệu tập trung ở `Metrics` (mục 6).

## 5. File CSV đầu vào

Header chính thức:

```
pid,arrival,burst,priority
```

| Cột | Kiểu | Ràng buộc |
|---|---|---|
| `pid` | int | > 0, duy nhất |
| `arrival` | int | >= 0 |
| `burst` | int | > 0 |
| `priority` | int | số nhỏ = ưu tiên cao |

- Dấu phẩy phân cách; có 1 dòng header; hỗ trợ `\r\n`; bỏ qua dòng trống và khoảng trắng thừa.
- **Không sắp xếp lại** bất kỳ dữ liệu nào khi đọc/ghi.
- Lỗi: trả `bool` + tham số `std::string& error` (**không ném exception**), thông báo **tiếng Việt**, luôn kèm số dòng, ví dụ: `Dòng 3: burst phải > 0`.
- **Mọi lỗi đọc file phải hiển thị lên GUI** (dialog/thông báo trong `MainWindow`), không chỉ in ra console — đề bài bắt buộc hiển thị trạng thái hợp lệ/lỗi cho người dùng.


## 6. Phân vai tính toán

| Thành phần | Được làm | KHÔNG được làm |
|---|---|---|
| `NonPreemptiveScheduler` / `PreemptiveScheduler` | xếp lịch, điền `ProcessResult` + `gantt` | đọc/ghi file, vẽ UI, phụ thuộc Qt |
| `Metrics` | tính mọi trung bình, bảng so sánh | đọc/ghi file, xếp lịch |
| `CsvHandler` | đọc CSV đầu vào, **ghi** kết quả ra file | tính toán lại số liệu, phụ thuộc scheduler/Qt |
| `ui/` | vẽ Gantt/bảng, ghép tiền tố "P", hiển thị lỗi CSV | tính toán số liệu |
| `benchmark/` | chạy đo hiệu năng trên bộ data chuẩn (mục 10) | xếp lịch, vẽ UI |

`CsvHandler::writeComparison` **chỉ format** hai `ScheduleResult` cho sẵn ra file, không tự tính.

## 7. File CSV đầu ra

| Nội dung | Đường dẫn |
|---|---|
| Kết quả NP | `output/np_result.csv` |
| Gantt NP | `output/np_gantt.csv` |
| Kết quả P | `output/p_result.csv` |
| Gantt P | `output/p_gantt.csv` |
| So sánh 2 thuật toán | `output/comparison.csv` |

**`*_result.csv`** — 4 cột, 1 dòng header, mỗi tiến trình 1 dòng:

```
pid,CT,TAT,WT,RT
1,8,8,0,0
2,12,11,7,7
3,14,12,10,10
```

**`*_gantt.csv`**:

```
pid,start,end
1,0,8
2,8,12
3,12,14
```

- IDLE ghi `pid = -1`.
- **Gộp 2 đoạn liền nhau cùng pid** (kể cả 2 đoạn IDLE sát nhau) thành 1 đoạn trước khi ghi/hiển thị.

**`comparison.csv`** — 3 cột, trung bình ghi **1 dòng riêng cho mỗi chỉ số**, làm tròn **2 chữ số thập phân**:

```
metric,np,p
avg_WT,5.67,3.00
avg_TAT,10.33,7.67
avg_RT,5.67,1.00
```

## 8. Kiểm thử

- Framework: **doctest** (single header, đặt trong `tests/third_party/doctest.h`, thêm bằng `target_include_directories`) — chốt trước khi viết test, không đổi giữa chừng.
- Test đặt `tests/unit/<module>_test.cpp`, chạy được **độc lập** (không cần Qt, không cần scheduler khác).
- Mỗi module có test xanh **trước khi** tạo PR.
- Test cuối (Leader nghiệm thu): chạy `samples/sample.csv` qua cả 2 biến thể, so `output/*.csv` khớp bảng chuẩn ở mục 5.
- Bảng kiểm thử phải ghi rõ **expected vs actual** cho các nhóm case: bình thường, IDLE, tie (hòa priority/AT), preemption (riêng biến thể P), input lỗi.

## 9. Giao diện hiển thị (UI)

- Hiển thị PID dạng `"P" + pid` (`P1`, `P2`, ...); IDLE hiển thị chữ `IDLE`.
- MainWindow cần thấy: chọn file CSV → chạy từng/hoặc cả 2 thuật toán → bảng kết quả (CT/TAT/WT/RT + trung bình) → Gantt Chart → nút xuất `output/*.csv`.
- Thông báo lỗi CSV hiển thị ngay tại cửa sổ chính (mục 5).

## 10. Benchmark hiệu năng

| File | Số tiến trình | Ghi chú |
|---|---|---|
| `samples/sample.csv` | 3 | test chính xác số liệu |
| `samples/perf_small.csv` | 20 | sinh ngẫu nhiên, seed cố định |
| `samples/perf_medium.csv` | 100 | |
| `samples/perf_large.csv` | 1000 | |

- File perf sinh bằng script/seed cố định để mọi thành viên tái lập được.
- Benchmark đo: thời gian chạy của từng scheduler (ms) trên từng bộ data + bảng WT/TAT/RT trung bình → phục vụ báo cáo (cột "hiệu năng", "giới hạn").
- Kết quả benchmark ghi vào `report/` (CSV hoặc markdown), không trộn với `output/` của test chính xác.



---

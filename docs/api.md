

## 1. Nguyên tắc chung

1. API thuộc về **nơi sản xuất + nơi tiêu thụ chung**, không thuộc nơi tiêu thụ.
2. `ps_core` (core + io) **không được** include Qt.
3. **Không ném exception.** Lỗi I/O trả `bool` + `std::string& error`.
4. Không module nào tự định nghĩa lại struct dùng chung.
5. Include một kiểu duy nhất: `"core/..."`, `"io/..."`, `"ui/..."`, `"benchmark/..."`.
6. Mọi trường thời gian là `long long`; `pid`, `priority` là `int`.
7. Header dùng `#pragma once`. Không dùng namespace
8. Hàm public là `static` trong class, không dùng free function cho API chung.

### Quan hệ phụ thuộc (bắt buộc)

```
core/*      →  không include io/, ui/, benchmark/
io/*        →  được include core/, KHÔNG include ui/
benchmark/* →  được include core/, KHÔNG include ui/
ui/*        →  được include core/, io/, benchmark/
tests/*     →  được include tất cả (trừ ui nếu test không cần Qt)
```

---

## 2. Struct dùng chung — FROZEN

```cpp
// src/core/Process.h
#pragma once

struct Process {
    int pid;                 // > 0, duy nhất
    long long arrivalTime;   // AT >= 0
    long long burstTime;     // BT > 0
    int priority;            // số nhỏ = ưu tiên cao
};
```

```cpp
// src/core/GanttSegment.h
#pragma once

struct GanttSegment {
    int pid;                 // -1 = IDLE
    long long startTime;
    long long endTime;       // endTime > startTime
};
```

```cpp
// src/core/ScheduleResult.h
#pragma once

#include <vector>
#include "core/GanttSegment.h"

struct ProcessResult {
    int pid;
    long long completionTime;    // CT
    long long turnaroundTime;    // TAT = CT - AT
    long long waitingTime;       // WT = TAT - BT
    long long responseTime;      // RT = ST - AT
};

struct ScheduleResult {
    std::vector<ProcessResult> processes;  // thứ tự = thứ tự input
    std::vector<GanttSegment>  gantt;      // đoạn liền kề cùng pid đã gộp
    double averageWaitingTime    = 0.0;
    double averageTurnaroundTime = 0.0;
    double averageResponseTime   = 0.0;
};
```

### Bất biến (invariant) — mọi module phải giữ

| # | Bất biến |
|---|---|
| I1 | `ScheduleResult.processes` giữ **đúng thứ tự input**, không phải thứ tự hoàn thành |
| I2 | `gantt` phủ kín `[0, max(CT)]`, không có khoảng trống, không chồng lấn |
| I3 | `gantt` không có 2 đoạn liền kề cùng `pid` (IDLE cũng gộp) |
| I4 | IDLE biểu diễn bằng `pid = -1`, không dùng số 0 hay chuỗi |
| I5 | Scheduler **không** điền 3 trường `average*`; `Metrics` điền |
| I6 | Input rỗng → trả `ScheduleResult` rỗng, không crash, không log |
| I7 | `WT = TAT - BT`, `TAT = CT - AT`, `RT = ST - AT` với mọi phần tử |
| I8 | Không có context switch, không I/O |

---

## 3. Scheduler — FROZEN

```cpp
// src/core/NonPreemptiveScheduler.h
#pragma once
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class NonPreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
```

```cpp
// src/core/PreemptiveScheduler.h
#pragma once
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class PreemptiveScheduler {
public:
    static ScheduleResult schedule(const std::vector<Process>& processes);
};
```

Hợp đồng:

- Thuần logic: không đọc/ghi file, không Qt, không in ra màn hình, không `cout` debug.
- Không sửa `processes` đầu vào (nhận `const&`).
- Tie-break: **priority nhỏ hơn → AT nhỏ hơn → thứ tự trong vector**. Không xét `pid`.
- Non-preemptive: đã chọn chạy hết BT.
- Preemptive: mỗi 1 đơn vị thời gian kiểm tra lại; gộp đoạn Gantt liền kề.
- Trả kết quả theo I1–I8.

---

## 4. CsvHandler — FROZEN

```cpp
// src/io/CsvHandler.h
#pragma once
#include <string>
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class CsvHandler {
public:
    // Đọc + validate. Thành công: true, out có dữ liệu theo đúng thứ tự dòng.
    // Thất bại: false, error chứa thông báo (tiếng Việt, kèm số dòng).
    static bool readProcesses(const std::string& path,
                              std::vector<Process>& out,
                              std::string& error);

    // Ghi kết quả từng tiến trình: pid,CT,TAT,WT,RT
    static bool writeProcessResults(const std::string& path,
                                    const ScheduleResult& result,
                                    std::string& error);

    // Ghi Gantt: pid,start,end (IDLE = -1)
    static bool writeGantt(const std::string& path,
                           const ScheduleResult& result,
                           std::string& error);

    // Ghi bảng so sánh: metric,np,p — chỉ format, không tính toán
    static bool writeComparison(const std::string& path,
                                const ScheduleResult& np,
                                const ScheduleResult& p,
                                std::string& error);
};
```

### Quy tắc đọc

| Kiểm tra | Lỗi trả về (mẫu) |
|---|---|
| Không mở được file | `Không mở được file: <path>` |
| Sai header | `Dòng 1: header phải là pid,arrival,burst,priority` |
| Thiếu/thừa cột | `Dòng N: cần 4 cột, có <k>` |
| Không phải số nguyên | `Dòng N: <tên cột> không phải số nguyên` |
| `arrival < 0` | `Dòng N: arrival phải >= 0` |
| `burst <= 0` | `Dòng N: burst phải > 0` |
| `pid` trùng | `Dòng N: pid <x> bị trùng` |
| File chỉ có header | `File không có tiến trình nào` |

- Bỏ qua dòng trống và khoảng trắng thừa; hỗ trợ `\r\n`.
- **Không sắp xếp lại** dữ liệu.
- `error` rỗng khi thành công.

### Quy tắc ghi

- `writeProcessResults`: đúng 4 dòng dữ liệu cho sample, header `pid,CT,TAT,WT,RT`.
- `writeGantt`: header `pid,start,end`; IDLE ghi `-1`.
- `writeComparison`: header `metric,np,p`; 3 dòng `avg_WT`, `avg_TAT`, `avg_RT`;
  số làm tròn **2 chữ số thập phân** (`std::fixed << setprecision(2)`).
- `writeComparison` đọc 3 trường `average*` đã được `Metrics` điền; **không tự tính**.
- Không hard-code đường dẫn; nơi gọi truyền vào.
- Mỗi file kết thúc bằng một ký tự xuống dòng (`\n`).

---

## 5. Metrics — FROZEN

```cpp
// src/core/Metrics.h
#pragma once
#include "core/ScheduleResult.h"

struct ComparisonRow {
    double np;
    double p;
};

struct Comparison {
    ComparisonRow waitingTime;
    ComparisonRow turnaroundTime;
    ComparisonRow responseTime;
};

class Metrics {
public:
    // Điền 3 trường average* từ result.processes.
    // Danh sách rỗng -> cả 3 = 0.0. Gọi lại nhiều lần cho cùng kết quả.
    static void fillAverages(ScheduleResult& result);

    // Bảng so sánh 2 biến thể; dùng cho GUI và báo cáo.
    static Comparison compare(const ScheduleResult& np,
                              const ScheduleResult& p);
};
```

Hợp đồng:

- Đây là **nơi duy nhất** tính trung bình và so sánh.
- Không đọc/ghi file, không Qt.
- Trung bình = tổng / số tiến trình; danh sách rỗng → `0.0` (không chia cho 0).

**Quyết định đã chốt:** `CsvHandler::writeComparison` nhận 2 `ScheduleResult` đã có
average, tự format. Không truyền `Comparison` vào CSV.

---

## 6. Benchmark — FROZEN

```cpp
// src/benchmark/Benchmark.h
#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "core/Process.h"

struct BenchmarkResult {
    std::string label;          // tên bộ dữ liệu, ví dụ "perf_medium"
    std::size_t processCount;   // số tiến trình
    double npMilliseconds;      // thời gian chạy NP
    double pMilliseconds;       // thời gian chạy P
};

class Benchmark {
public:
    // Chạy cả 2 scheduler trên cùng input, đo bằng std::chrono::steady_clock.
    static BenchmarkResult run(const std::string& label,
                               const std::vector<Process>& processes);

    // Ghi report/benchmark.csv
    static bool writeReport(const std::string& path,
                            const std::vector<BenchmarkResult>& results,
                            std::string& error);
};
```

- Không sửa scheduler, không tự tính lại CT/TAT/WT/RT.
- Bộ kích thước: 1, 10, 50, 100, 150, 200, 500, 1.000, 5.000, 10.000, 50.000,
  100.000, 500.000, 1.000.000 tiến trình (sinh bằng `tools/generate_perf.cpp`).
- Đo tối thiểu 3 lần mỗi bộ, lấy trung bình (ghi rõ trong báo cáo).
- Kết quả ghi `report/`, không trộn với `output/`.

---

## 7. UI — hợp đồng tối thiểu (nội bộ không freeze)

```cpp
// src/ui/GanttChart.h
#pragma once
#include <vector>
#include "core/GanttSegment.h"

class GanttChart {
public:
    void setData(const std::vector<GanttSegment>& segments);  // pid = -1 vẽ IDLE
};
```

Trách nhiệm `MainWindow`:

1. Chọn file → `CsvHandler::readProcesses`.
2. Lỗi → hiện `error` ngay trên cửa sổ (không chỉ console), không crash.
3. Chạy → 2 scheduler → `Metrics::fillAverages` → hiện bảng + Gantt.
4. Xuất → `CsvHandler::write*` vào `output/`.
5. Hiển thị pid dạng `"P" + pid`; IDLE hiện chữ `IDLE`.

UI **không được**: tự tính CT/TAT/WT/RT, tự parse CSV, tự định nghĩa struct dữ liệu.

---

## 8. Bảng dữ liệu chuẩn (kết quả chuẩn) — dùng cho test

Nguồn: `samples/sample.csv` (10 process)

```
pid,arrival,burst,priority
1,0,5,3
2,1,3,2
3,2,8,1
4,4,2,2
5,6,4,4
6,9,1,1
7,12,3,3
8,15,2,2
9,18,5,5
10,21,2,1
```

### Non-preemptive

Gantt: `1[0-5] → 3[5-13] → 6[13-14] → 2[14-17] → 4[17-19] → 8[19-21] → 10[21-23] → 7[23-26] → 5[26-30] → 9[30-35]`

| pid | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|
| 1 | 5 | 5 | 0 | 0 |
| 2 | 17 | 16 | 13 | 13 |
| 3 | 13 | 11 | 3 | 3 |
| 4 | 19 | 15 | 13 | 13 |
| 5 | 30 | 24 | 20 | 20 |
| 6 | 14 | 5 | 4 | 4 |
| 7 | 26 | 14 | 11 | 11 |
| 8 | 21 | 6 | 4 | 4 |
| 9 | 35 | 17 | 12 | 12 |
| 10 | 23 | 2 | 0 | 0 |

Trung bình: WT **8.00**, TAT **11.50**, RT **8.00**

### Preemptive

Gantt: `1[0-1] → 2[1-2] → 3[2-10] → 6[10-11] → 2[11-13] → 4[13-15] → 8[15-17] → 1[17-21] → 10[21-23] → 7[23-26] → 5[26-30] → 9[30-35]`

| pid | CT | TAT | WT | RT |
|---:|---:|---:|---:|---:|
| 1 | 21 | 21 | 16 | 0 |
| 2 | 13 | 12 | 9 | 0 |
| 3 | 10 | 8 | 0 | 0 |
| 4 | 15 | 11 | 9 | 9 |
| 5 | 30 | 24 | 20 | 20 |
| 6 | 11 | 2 | 1 | 1 |
| 7 | 26 | 14 | 11 | 11 |
| 8 | 17 | 2 | 0 | 0 |
| 9 | 35 | 17 | 12 | 12 |
| 10 | 23 | 2 | 0 | 0 |

Trung bình: WT **7.80**, TAT **11.30**, RT **5.30**

### comparison.csv

```
metric,np,p
avg_WT,8.00,7.80
avg_TAT,11.50,11.30
avg_RT,8.00,5.30
```

### File kết quả chuẩn

Bản gốc chia theo thành viên trong `tests/data/ket_qua/`:

| Folder | Dùng cho |
|---|---|
| `ket_qua/csv/` | C16–C20, C25 |
| `ket_qua/non-preemptive/` | N01, N12 |
| `ket_qua/preemptive/` | P01, P18 |
| `ket_qua/gui/` | U03, U05, U06, U13 |
| `ket_qua/leader/` | M01–M02, I01–I03, I08 |

Ví dụ folder leader:

```
tests/data/ket_qua/leader/sample_np_result.csv
tests/data/ket_qua/leader/sample_np_gantt.csv
tests/data/ket_qua/leader/sample_p_result.csv
tests/data/ket_qua/leader/sample_p_gantt.csv
tests/data/ket_qua/leader/sample_comparison.csv
tests/data/ket_qua/leader/sample_20_np_result.csv
tests/data/ket_qua/leader/sample_20_np_gantt.csv
tests/data/ket_qua/leader/sample_20_p_result.csv
tests/data/ket_qua/leader/sample_20_p_gantt.csv
tests/data/ket_qua/leader/sample_20_comparison.csv
```





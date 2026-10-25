# TÀI LIỆU MODULE CSV – PRIORITY SCHEDULING

Nhánh: `feature/csv` · File phụ trách: `src/io/CsvHandler.h`, `src/io/CsvHandler.cpp` · Test: `tests/unit/csv/csv_handler_test.cpp`

## 1. Nghiên cứu về phần mình đảm nhiệm

### 1.1. Vai trò của module CSV

Module CSV là **cổng vào và cổng ra dữ liệu** của chương trình:

- **Đầu vào:** đọc file CSV chứa danh sách tiến trình và chuyển thành `std::vector<Process>` để hai thuật toán (Non-preemptive, Preemptive) sử dụng.
- **Đầu ra:** ghi `ScheduleResult` (kết quả từng tiến trình, các đoạn Gantt) và bảng so sánh hai thuật toán ra file CSV trong thư mục `output/`.

Module **không** xếp lịch, **không** tính toán lại số liệu và **không** phụ thuộc scheduler hay Qt. Nhờ vậy hai thuật toán luôn chạy trên **cùng một bộ dữ liệu** đọc từ một file duy nhất, đúng yêu cầu của đề tài (so sánh hai biến thể trên cùng dữ liệu).

### 1.2. Quy ước chung của nhóm ảnh hưởng đến module

| Quy ước | Ảnh hưởng đến CSV |
|---|---|
| Priority nhỏ hơn = ưu tiên cao hơn | Cột `priority` là số nguyên; module không đổi nghĩa giá trị này. |
| Hòa priority: AT nhỏ hơn trước; hòa cả hai: theo thứ tự dòng trong `vector` | Module phải **giữ nguyên thứ tự dòng** của file, không sắp xếp lại bất kỳ dữ liệu nào khi đọc/ghi. |
| Không có tiến trình sẵn sàng: CPU ở trạng thái IDLE | IDLE được ghi là `pid = -1` trong mọi file CSV; chữ "IDLE" chỉ xuất hiện ở giao diện. |
| `pid` luôn là `int`; UI tự ghép tiền tố "P" | File CSV chỉ ghi số (`1`, `2`, ...), không ghi `P1`. |
| Mọi trường thời gian là `long long` | Khi đọc dùng `std::from_chars` (không ném exception); `arrival` và `burst` tối đa 10^12 để scheduler cộng dồn không tràn; khi ghi giữ nguyên số nguyên. |
| Không I/O, không chi phí chuyển ngữ cảnh, starvation/aging ngoài phạm vi | Không ảnh hưởng đến CSV. |
| Mọi lỗi đọc file phải hiển thị lên GUI | CsvHandler trả thông báo lỗi; `MainWindow` chịu trách nhiệm hiển thị. |
| Mỗi module có test xanh trước khi tạo PR vào `main` | Có kế hoạch kiểm thử riêng cho CSV (mục 6). |

### 1.3. Phân vai tính toán

| Thành phần | Được làm | KHÔNG được làm |
|---|---|---|
| `CsvHandler` | Đọc CSV đầu vào, ghi kết quả ra file | Tính toán lại số liệu, phụ thuộc scheduler hoặc Qt |
| Scheduler (NP/P) | Xếp lịch, điền `ProcessResult` và `gantt` | Đọc/ghi file, vẽ UI |
| `Metrics` | Tính mọi giá trị trung bình, bảng so sánh | Đọc/ghi file, xếp lịch |
| `ui/` | Vẽ Gantt/bảng, ghép tiền tố "P", hiển thị lỗi CSV | Tính toán số liệu |

`CsvHandler::writeComparison` chỉ **định dạng** hai `ScheduleResult` cho sẵn ra file, không tự tính.

### 1.4. Các vấn đề kỹ thuật cần xử lý khi đọc CSV

- Dòng tiêu đề (header), dòng trống, khoảng trắng thừa quanh giá trị, ký tự xuống dòng kiểu Windows (`\r\n`).
- Dòng thiếu cột hoặc thừa cột.
- Giá trị không phải số nguyên (ví dụ `abc`, `3.5`, để trống).
- Giá trị không hợp lệ về ý nghĩa: `pid <= 0`, `arrival < 0`, `burst <= 0`.
- `pid` bị trùng.
- File không tồn tại hoặc không mở được.
- Thư viện chuẩn C++17 (`std::ifstream`, `std::getline`, `std::from_chars`) là đủ, không cần thư viện ngoài.

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần

| Thành phần | Nhiệm vụ |
|---|---|
| `CsvHandler.h` | Khai báo giao diện (hàm, kiểu trả về) mà các module khác gọi. |
| `CsvHandler.cpp` | Cài đặt: mở file, tách dòng và cột, kiểm tra hợp lệ, tạo `Process`, ghi kết quả. |
| `Process` (`core/Process.h`) | Cấu trúc đầu vào dùng chung: `pid`, `arrivalTime`, `burstTime`, `priority`. |
| `GanttSegment` (`core/GanttSegment.h`) | Một đoạn chạy: `pid` (-1 là IDLE), `startTime`, `endTime`. |
| `ProcessResult` (`core/ScheduleResult.h`) | Kết quả một tiến trình: `pid`, `completionTime`, `turnaroundTime`, `waitingTime`, `responseTime`. |
| `ScheduleResult` (`core/ScheduleResult.h`) | Kết quả một thuật toán: `processes`, `gantt`, `averageWaitingTime`, `averageTurnaroundTime`, `averageResponseTime`. |

Các kiểu dữ liệu chung (do Leader quản lý, không tự ý sửa):

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

### Chức năng chính của CsvHandler

1. **Đọc tiến trình** (`readProcesses`): đọc `input/*.csv` hoặc `samples/*.csv`, kiểm tra, trả về danh sách `Process` theo đúng thứ tự dòng.
2. **Ghi kết quả từng tiến trình** (`writeProcessResults`): ghi `pid, CT, TAT, WT, RT`.
3. **Ghi các đoạn Gantt** (`writeGantt`): ghi `pid, start, end` đúng như nhận được (scheduler đã gộp sẵn các đoạn liền kề cùng `pid`, kể cả IDLE).
4. **Ghi bảng so sánh** (`writeComparison`): ghi `metric, np, p`, làm tròn 2 chữ số thập phân.
5. **Báo lỗi rõ ràng**: mọi lỗi bằng tiếng Việt, kèm số dòng, trả qua tham số `error` để GUI hiển thị.

## 3. Hướng dẫn sử dụng

> Lưu ý: định dạng file, cách báo lỗi, kiểu dữ liệu, tên hàm và chữ ký của `CsvHandler` đã thống nhất và cố định theo `docs/api.md` (FROZEN). Các ví dụ dưới đây dùng bộ dữ liệu rút gọn 3 tiến trình để dễ đọc; bộ dữ liệu chuẩn của nhóm là `samples/sample.csv` (10 tiến trình).

### 3.1. Định dạng file CSV đầu vào

Header chính thức, một dòng, phân cách bằng dấu phẩy, mỗi dòng một tiến trình. Ví dụ rút gọn (file chuẩn đầy đủ: `samples/sample.csv`, 10 tiến trình):

```
pid,arrival,burst,priority
1,0,8,3
2,1,4,1
3,2,2,2
```

| Cột | Kiểu | Ràng buộc | Ánh xạ vào `Process` |
|---|---|---|---|
| `pid` | số nguyên | > 0, duy nhất | `pid` |
| `arrival` | số nguyên | >= 0 | `arrivalTime` |
| `burst` | số nguyên | > 0 | `burstTime` |
| `priority` | số nguyên | số nhỏ = ưu tiên cao | `priority` |

Hỗ trợ `\r\n`, bỏ qua dòng trống và khoảng trắng thừa. Thứ tự dòng trong file chính là **thứ tự nhập** dùng để hòa cuối cùng.

Vị trí file: `samples/` chứa file mẫu chuẩn (dữ liệu dùng chung cho tài liệu và test), `input/` chứa dữ liệu người dùng.

### 3.2. Cách gọi hàm đọc

```cpp
#include "CsvHandler.h"

std::string error;
std::vector<Process> processes;

if (!CsvHandler::readProcesses("samples/sample.csv", processes, error)) {
    // MainWindow hiển thị error lên dialog; ví dụ: "Dòng 3: burst phải > 0"
    return;
}
// processes giờ có thể truyền cho cả hai scheduler
```

- **Đầu vào:** đường dẫn file.
- **Đầu ra:** trả về `true` nếu thành công; `processes` chứa dữ liệu theo đúng thứ tự dòng. Trả về `false` nếu lỗi; `error` chứa thông báo tiếng Việt kèm số dòng. Không ném exception.

### 3.3. Ví dụ file sai và phản ứng của chương trình

| Nội dung dòng | Thông báo lỗi |
|---|---|
| `2,1,4` | "Dòng N: thiếu cột (cần 4 cột)" |
| `2,1,-4,1` | "Dòng N: burst phải > 0" |
| `2,-1,4,1` | "Dòng N: arrival phải >= 0" |
| `2,x,4,1` | "Dòng N: arrival không phải số nguyên" |
| `0,1,4,1` | "Dòng N: pid phải > 0" |
| `1,0,8,3` xuất hiện hai lần | "Dòng N: pid 1 bị trùng" |
| file không tồn tại | "Không mở được file ..." |
| file chỉ có header | "File không có tiến trình nào" |

### 3.4. Cách gọi hàm ghi và định dạng file đầu ra

Tất cả file ghi vào `output/`:

| Nội dung | Đường dẫn |
|---|---|
| Kết quả Non-preemptive | `output/np_result.csv` |
| Gantt Non-preemptive | `output/np_gantt.csv` |
| Kết quả Preemptive | `output/p_result.csv` |
| Gantt Preemptive | `output/p_gantt.csv` |
| So sánh hai thuật toán | `output/comparison.csv` |

```cpp
CsvHandler::writeProcessResults("output/np_result.csv", resultNP, error);
CsvHandler::writeGantt("output/np_gantt.csv", resultNP, error);
CsvHandler::writeProcessResults("output/p_result.csv", resultP, error);
CsvHandler::writeGantt("output/p_gantt.csv", resultP, error);
CsvHandler::writeComparison("output/comparison.csv", resultNP, resultP, error);
```

**Kết quả từng tiến trình** (`np_result.csv`, 5 cột, một dòng header, mỗi tiến trình một dòng):

```
pid,CT,TAT,WT,RT
1,8,8,0,0
2,12,11,7,7
3,14,12,10,10
```

**Đoạn Gantt** (`np_gantt.csv`), IDLE ghi `pid = -1`:

```
pid,start,end
1,0,8
2,8,12
3,12,14
```

**So sánh** (`comparison.csv`), mỗi chỉ số một dòng, làm tròn 2 chữ số thập phân:

```
metric,np,p
avg_WT,5.67,3.00
avg_TAT,10.33,7.67
avg_RT,5.67,1.00
```

### 3.5. Ví dụ minh họa đầy đủ

Dữ liệu vào (ví dụ rút gọn): P1 (AT 0, BT 8, Pri 3), P2 (AT 1, BT 4, Pri 1), P3 (AT 2, BT 2, Pri 2).

- Non-preemptive: P1 chạy 0–8, P2 8–12, P3 12–14.
- Preemptive: P1 chạy 0–1, P2 1–5, P3 5–7, P1 7–14.

`output/p_result.csv` và `output/p_gantt.csv` của Preemptive:

```
pid,CT,TAT,WT,RT
1,14,14,6,0
2,5,4,0,0
3,7,5,3,3
```

```
pid,start,end
1,0,1
2,1,5
3,5,7
1,7,14
```

Cả hai chạy trên cùng danh sách `processes` do CSV đọc ra, rồi kết quả được ghi thành các file trên để so sánh.

## 4. Flow hoạt động

### 4.1. Luồng đọc (đầu vào)

1. Người dùng chọn file CSV trên `MainWindow` (hoặc chương trình nhận đường dẫn).
2. `readProcesses` mở file; nếu không mở được thì trả `false` kèm lỗi và dừng.
3. Đọc dòng đầu tiên là header, kiểm tra đúng `pid,arrival,burst,priority`.
4. Với mỗi dòng còn lại: bỏ qua dòng trống, cắt khoảng trắng, tách theo dấu phẩy.
5. Kiểm tra số cột, chuyển sang số nguyên, kiểm tra ràng buộc (pid > 0 và duy nhất, arrival >= 0, burst > 0).
6. Tạo `Process` và thêm vào cuối `vector` (giữ thứ tự nhập).
7. Hết file: kiểm tra có ít nhất một tiến trình, trả `vector` cho nơi gọi.
8. Nếu có lỗi, `MainWindow` hiển thị thông báo lỗi tại cửa sổ chính.

### 4.2. Luồng tổng thể của chương trình

```
samples/sample.csv hoặc input/*.csv
   │  readProcesses
   ▼
vector<Process> ──┬──► NonPreemptiveScheduler::schedule ──► ScheduleResult (NP) ──┐
                  │                                                                ├─► Metrics ──► MainWindow / GanttChart
                  └──► PreemptiveScheduler::schedule    ──► ScheduleResult (P)  ──┘
                                                                  │
       writeProcessResults / writeGantt / writeComparison ◄──────┘
                                  │
                                  ▼
              output/np_*.csv, output/p_*.csv, output/comparison.csv
```

### 4.3. Luồng ghi (đầu ra)

1. Người dùng bấm nút xuất kết quả trên `MainWindow`.
2. `MainWindow` gọi các hàm ghi với `ScheduleResult` đã có.
3. Mỗi hàm mở file trong `output/`, ghi header rồi từng dòng; `writeGantt` ghi nguyên các đoạn đã gộp sẵn từ scheduler; `writeComparison` làm tròn 2 chữ số thập phân.
4. Lỗi ghi file (không mở được, thư mục không tồn tại) trả `false` kèm thông báo.

### 4.4. Cách các thành phần tương tác

- **CSV → Scheduler:** truyền `std::vector<Process>`; scheduler không biết dữ liệu đến từ file.
- **Scheduler → CSV:** trả `ScheduleResult`; CSV chỉ đọc kết quả để ghi, không tính toán lại.
- **Metrics → CSV:** các giá trị trung bình đã nằm sẵn trong `ScheduleResult`; CSV chỉ định dạng.
- **CSV ↔ UI:** `MainWindow` gọi hàm CSV và hiển thị lỗi; CSV không phụ thuộc Qt.
- **Benchmark:** dùng các file trong `samples/` (`perf_<n>.csv`); kết quả benchmark ghi riêng vào `report/`, không trộn với `output/`.

## 5. Cấu trúc và thiết kế

### 5.1. Cấu trúc thư mục liên quan

```
PriorityScheduling/
  src/
    core/   Process.h, GanttSegment.h, ScheduleResult.h, scheduler, Metrics
    io/     CsvHandler.h, CsvHandler.cpp          ← phần của tôi
  tests/
    unit/csv/  csv_handler_test.cpp
    third_party/doctest.h
  samples/  sample.csv, sample_20.csv, perf_<n>.csv (sinh bằng tools/generate_perf.cpp)
  input/    dữ liệu người dùng
  output/   np_result.csv, np_gantt.csv, p_result.csv, p_gantt.csv, comparison.csv
  docs/     tài liệu
```

### 5.2. Hàm quan trọng (theo `docs/api.md`)

| Hàm | Đầu vào | Đầu ra | Vai trò |
|---|---|---|---|
| `readProcesses(path, out, error)` | đường dẫn file | `bool`, `vector<Process>`, lỗi | Đọc và kiểm tra file đầu vào |
| `writeProcessResults(path, result, error)` | đường dẫn, `ScheduleResult` | `bool`, lỗi | Ghi `pid,CT,TAT,WT,RT` |
| `writeGantt(path, result, error)` | đường dẫn, `ScheduleResult` | `bool`, lỗi | Ghi `pid,start,end` đúng như nhận được |
| `writeComparison(path, np, p, error)` | hai `ScheduleResult` | `bool`, lỗi | Ghi `metric,np,p` từ số liệu có sẵn |
| hàm nội bộ `splitLine`, `trim`, `parseInt` | chuỗi | các trường / số nguyên | Hỗ trợ tách và chuyển đổi, không public |

### 5.3. Quyết định thiết kế

- **Trả về `bool` + tham số `error`** thay vì ném exception: đúng quy ước nhóm, dễ hiển thị lỗi lên GUI và dễ kiểm thử.
- **Không sắp xếp lại dữ liệu**: bảo toàn thứ tự nhập để tie-breaking đúng.
- **Chỉ phụ thuộc `Process`, `ScheduleResult`, `GanttSegment`**: không phụ thuộc scheduler hay Qt, nên test độc lập được.
- **Chỉ định dạng, không tính toán**: mọi số liệu do scheduler và `Metrics` cung cấp.
- **Kiểm tra dữ liệu ngay khi đọc**: scheduler có thể tin rằng đầu vào đã hợp lệ.
- **`pid = -1` cho IDLE trong file**: UI mới chuyển thành chữ "IDLE" và ghép tiền tố "P".

### 5.4. Các điểm đã chốt theo `docs/api.md`

1. Tên hàm và chữ ký: `class CsvHandler` với bốn hàm `static` (`readProcesses`, `writeProcessResults`, `writeGantt`, `writeComparison`), trả `bool` kèm `std::string& error`.
2. Gộp các đoạn Gantt liền kề cùng `pid` là việc của scheduler; `writeGantt` chỉ ghi lại.
3. Làm tròn 2 chữ số thập phân khi ghi `comparison.csv` do `writeComparison` đảm nhiệm, dựa trên ba trường `average*` mà `Metrics` đã điền.
4. Còn lại: tên file tài liệu theo quy ước `docs/<Module>.md` (hiện là `csv-handler.md`), cần hỏi nhóm có đổi thành `CsvHandler.md` hay không.

## 6. Kế hoạch kiểm thử độc lập

- **Framework:** doctest (single header), đặt tại `tests/third_party/doctest.h`, thêm vào CMake bằng `target_include_directories`.
- **Vị trí:** `tests/unit/csv/csv_handler_test.cpp`, chạy độc lập, không cần Qt hay scheduler.
- **Điều kiện PR:** test xanh trước khi tạo PR vào `main`.
- **Nghiệm thu:** chạy `samples/sample.csv` qua hai biến thể, so kết quả ghi ra khớp file chuẩn trong `tests/data/ket_qua/csv/`.

Bảng kiểm thử (cột "Thực tế" được điền khi chạy test):

| Nhóm | Trường hợp | Kỳ vọng | Thực tế |
|---|---|---|---|
| Bình thường | `tests/data/csv/sample.csv` 10 tiến trình (C01) | `true`, 10 `Process` đúng giá trị và thứ tự | Đạt |
| Bình thường | Dòng trống, khoảng trắng thừa, `\r\n` | Bỏ qua dòng trống, giá trị vẫn đúng | Đạt |
| Bình thường | Ghi `np_result.csv` rồi đọc lại | Khớp `ScheduleResult` ban đầu | Đạt |
| IDLE | Gantt có đoạn `pid = -1` | File ghi đúng `-1`, không ghi chữ "IDLE" | Đạt |
| Hòa (tie) | Hai tiến trình cùng priority và AT | Thứ tự dòng trong `vector` được giữ nguyên | Đạt |
| Preemption | Gantt có cùng `pid` xuất hiện nhiều đoạn không liền nhau | Ghi đủ các đoạn đúng thứ tự, không gộp | Đạt |
| Input lỗi | Thiếu / thừa cột | `false`, lỗi nêu đúng số dòng | Đạt |
| Input lỗi | Giá trị không phải số | `false`, lỗi nêu đúng số dòng | Đạt |
| Input lỗi | `burst <= 0`, `arrival < 0`, `pid <= 0` | `false`, lỗi nêu đúng số dòng | Đạt |
| Input lỗi | `pid` trùng | `false` | Đạt |
| Input lỗi | File không tồn tại, file rỗng, chỉ có header | `false` | Đạt |
| Đầu ra | `comparison.csv` | `metric,np,p`, làm tròn 2 chữ số (`5.67`, `10.33`, `3.00`) | Đạt |

## 7. Tiến độ

- [x] Nghiên cứu phần phụ trách, thiết kế giao diện, viết tài liệu (tuần này).
- [x] Cập nhật tài liệu theo quy ước thống nhất của nhóm.
- [x] Cài đặt `CsvHandler` (`src/io/CsvHandler.h`, `src/io/CsvHandler.cpp`).
- [x] Viết test độc lập bằng doctest (44 test case, tất cả pass).
- [ ] Chạy bộ test của Leader và tạo PR vào `main`.

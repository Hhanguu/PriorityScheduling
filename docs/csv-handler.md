# TÀI LIỆU MODULE CSV – PRIORITY SCHEDULING

Nhánh: `feature/csv` · File phụ trách: `src/io/CsvHandler.h`, `src/io/CsvHandler.cpp`

## 1. Nghiên cứu về phần mình đảm nhiệm

### 1.1. Vai trò của module CSV

Module CSV là **cổng vào và cổng ra dữ liệu** của chương trình:

- **Đầu vào:** đọc file CSV chứa danh sách tiến trình và chuyển thành `std::vector<Process>` để hai thuật toán (Non-preemptive, Preemptive) sử dụng.
- **Đầu ra:** ghi `ScheduleResult` (kết quả từng tiến trình, các đoạn Gantt, giá trị trung bình) ra file CSV để lưu trữ, so sánh và vẽ biểu đồ.

Module **không** cài đặt thuật toán lập lịch và **không** vẽ giao diện. Nhờ vậy hai thuật toán luôn được chạy trên **cùng một bộ dữ liệu** đọc từ một file duy nhất, đúng yêu cầu của đề tài (so sánh hai biến thể trên cùng dữ liệu).

### 1.2. Quy ước chung của nhóm ảnh hưởng đến module

| Quy ước | Ảnh hưởng đến CSV |
|---|---|
| Priority càng nhỏ thì ưu tiên càng cao | Cột `priority` là số nguyên; module không đổi nghĩa giá trị này. |
| Cùng priority: AT nhỏ hơn được chọn trước; nếu vẫn bằng nhau thì theo **thứ tự nhập** | Module phải **giữ nguyên thứ tự các dòng** trong file, tuyệt đối không sắp xếp lại. |
| Không có tiến trình sẵn sàng thì CPU ở trạng thái IDLE | Khi ghi Gantt ra CSV, đoạn IDLE phải được biểu diễn được (xem mục 3.4). |
| Hai thuật toán dùng chung `Process` và trả về `ScheduleResult` | Module chỉ làm việc với hai cấu trúc này, không định nghĩa cấu trúc riêng. |
| Mỗi module có test độc lập trước khi tạo PR vào main | Có kế hoạch kiểm thử riêng cho CSV (mục 6). |

### 1.3. Các vấn đề kỹ thuật cần xử lý khi đọc CSV

- Dòng tiêu đề (header), dòng trống, khoảng trắng thừa quanh giá trị, ký tự xuống dòng kiểu Windows (`\r\n`).
- Dòng thiếu cột hoặc thừa cột.
- Giá trị không phải số nguyên (ví dụ `abc`, `3.5`, để trống).
- Giá trị không hợp lệ về ý nghĩa: `arrivalTime < 0`, `burstTime <= 0`.
- `pid` bị trùng.
- File không tồn tại hoặc không mở được.
- Thư viện chuẩn C++17 (`std::ifstream`, `std::getline`, `std::stringstream`, `std::stoll`) là đủ, không cần thư viện ngoài.

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần

| Thành phần | Nhiệm vụ |
|---|---|
| `CsvHandler.h` | Khai báo giao diện (hàm, kiểu trả về) mà các module khác gọi. |
| `CsvHandler.cpp` | Cài đặt: mở file, tách dòng và cột, kiểm tra hợp lệ, tạo `Process`, ghi kết quả. |
| `Process` (`core/Process.h`) | Cấu trúc đầu vào dùng chung: `pid`, `arrivalTime`, `burstTime`, `priority`. Do nhóm định nghĩa, CSV chỉ sử dụng. |
| `ScheduleResult` (`core/ScheduleResult.h`) | Cấu trúc đầu ra dùng chung: `processes` (danh sách `ProcessResult`), `gantt` (danh sách `GanttSegment`), `averageWaitingTime`, `averageTurnaroundTime`. |
| `GanttSegment` (`core/GanttSegment.h`) | Một đoạn chạy trên biểu đồ Gantt: `pid`, `startTime`, `endTime`. |

### Chức năng chính của CsvHandler

1. **Đọc tiến trình** (`readProcesses`): đọc file CSV, kiểm tra, trả về danh sách `Process` theo đúng thứ tự dòng.
2. **Ghi kết quả từng tiến trình** (`writeProcessResults`): ghi CT, TAT, WT, RT của từng tiến trình.
3. **Ghi các đoạn Gantt** (`writeGantt`): ghi các đoạn `pid, start, end`.
4. **Ghi bảng so sánh** (`writeComparison`): ghi các giá trị trung bình của hai thuật toán cạnh nhau (phục vụ phần so sánh và vẽ biểu đồ).
5. **Báo lỗi rõ ràng**: mọi lỗi kèm số dòng và nguyên nhân.

## 3. Hướng dẫn sử dụng

> Lưu ý: tên hàm và chữ ký dưới đây là **thiết kế đề xuất** (chưa cài đặt). Nhóm có thể điều chỉnh khi chốt giao diện chung.

### 3.1. Định dạng file CSV đầu vào

Có dòng tiêu đề, phân cách bằng dấu phẩy, mỗi dòng một tiến trình:

```
pid,arrival,burst,priority
1,0,8,3
2,1,4,1
3,2,2,2
```

| Cột | Kiểu | Ràng buộc | Ánh xạ vào `Process` |
|---|---|---|---|
| `pid` | số nguyên | duy nhất, > 0 | `pid` |
| `arrival` | số nguyên | >= 0 | `arrivalTime` |
| `burst` | số nguyên | > 0 | `burstTime` |
| `priority` | số nguyên | số nhỏ = ưu tiên cao | `priority` |

Thứ tự dòng trong file chính là **thứ tự nhập** dùng để hòa cuối cùng.

### 3.2. Cách gọi hàm đọc

```cpp
#include "CsvHandler.h"

std::string error;
std::vector<Process> processes;

if (!CsvHandler::readProcesses("data/sample.csv", processes, error)) {
    std::cerr << error << '\n';   // ví dụ: "Dòng 3: burst phải > 0"
    return 1;
}
// processes giờ có thể truyền cho cả hai scheduler
```

- **Đầu vào:** đường dẫn file.
- **Đầu ra:** trả về `true` nếu thành công; `processes` chứa dữ liệu theo đúng thứ tự dòng. Trả về `false` nếu lỗi; `error` chứa thông báo.

### 3.3. Ví dụ file sai và phản ứng của chương trình

| Nội dung dòng | Kết quả |
|---|---|
| `2,1,4` | Lỗi: "Dòng N: thiếu cột (cần 4 cột)" |
| `2,1,-4,1` | Lỗi: "Dòng N: burst phải > 0" |
| `2,x,4,1` | Lỗi: "Dòng N: arrival không phải số nguyên" |
| `1,0,8,3` xuất hiện hai lần | Lỗi: "Dòng N: pid 1 bị trùng" |
| file không tồn tại | Lỗi: "Không mở được file ..." |
| file chỉ có header | Lỗi: "File không có tiến trình nào" |

### 3.4. Cách gọi hàm ghi và định dạng file đầu ra

```cpp
CsvHandler::writeProcessResults("out/np_result.csv", resultNonPreemptive, error);
CsvHandler::writeGantt("out/np_gantt.csv", resultNonPreemptive, error);
```

**Kết quả từng tiến trình** (`np_result.csv`):

```
pid,CT,TAT,WT,RT
1,8,8,0,0
2,12,11,7,7
3,14,12,10,10
average_WT,5.67
average_TAT,10.33
```

**Đoạn Gantt** (`np_gantt.csv`):

```
pid,start,end
1,0,8
2,8,12
3,12,14
```

Đoạn CPU rảnh (IDLE) được ghi với `pid = -1` (quy ước đề xuất, cần nhóm xác nhận cùng người cài đặt scheduler và vẽ Gantt).

### 3.5. Ví dụ minh họa đầy đủ

Dữ liệu vào (`sample.csv`): P1 (AT 0, BT 8, Pri 3), P2 (AT 1, BT 4, Pri 1), P3 (AT 2, BT 2, Pri 2).

- Non-preemptive: P1 chạy 0–8, P2 8–12, P3 12–14.
- Preemptive: P1 chạy 0–1, P2 1–5, P3 5–7, P1 7–14.

Cả hai chạy trên cùng danh sách `processes` do CSV đọc ra; kết quả được ghi thành hai bộ file riêng để so sánh.

## 4. Flow hoạt động

### 4.1. Luồng đọc (đầu vào)

1. Người dùng chọn hoặc truyền đường dẫn file CSV.
2. `readProcesses` mở file; nếu không mở được thì báo lỗi và dừng.
3. Đọc dòng đầu tiên là header, kiểm tra đúng các cột mong đợi.
4. Với mỗi dòng còn lại: bỏ qua dòng trống, cắt khoảng trắng, tách theo dấu phẩy.
5. Kiểm tra số cột, chuyển sang số nguyên, kiểm tra ràng buộc (AT >= 0, BT > 0, pid không trùng).
6. Tạo `Process` và thêm vào cuối `vector` (giữ thứ tự nhập).
7. Hết file: kiểm tra có ít nhất một tiến trình, trả `vector` cho nơi gọi.

### 4.2. Luồng tổng thể của chương trình

```
sample.csv
   │  readProcesses
   ▼
vector<Process> ──┬──► NonPreemptiveScheduler ──► ScheduleResult (NP) ──┐
                  │                                                     ├─► Metrics / so sánh
                  └──► PreemptiveScheduler    ──► ScheduleResult (P)  ──┘
                                                      │
              writeProcessResults / writeGantt / writeComparison
                                                      ▼
                                       các file CSV kết quả ──► GanttChart / MainWindow
```

### 4.3. Cách các thành phần tương tác

- **CSV → Scheduler:** truyền `std::vector<Process>`; scheduler không biết dữ liệu đến từ file.
- **Scheduler → CSV:** trả `ScheduleResult`; CSV chỉ đọc kết quả để ghi, không tính toán lại.
- **CSV → UI/Benchmark:** `MainWindow` có thể gọi `readProcesses` khi người dùng chọn file; `Benchmark` có thể dùng CSV để nạp bộ dữ liệu mẫu.

## 5. Cấu trúc và thiết kế

### 5.1. Cấu trúc thư mục liên quan

```
src/
  core/   Process.h, ScheduleResult.h, GanttSegment.h, (scheduler, Metrics)
  io/     CsvHandler.h, CsvHandler.cpp      ← phần của tôi
tests/
  unit/   test độc lập cho CsvHandler
data/     file CSV mẫu (đề xuất)
docs/     tài liệu
```

### 5.2. Hàm quan trọng (thiết kế đề xuất)

| Hàm | Đầu vào | Đầu ra | Vai trò |
|---|---|---|---|
| `readProcesses(path, out, error)` | đường dẫn file | `bool`, danh sách `Process`, thông báo lỗi | Đọc và kiểm tra file đầu vào |
| `writeProcessResults(path, result, error)` | đường dẫn, `ScheduleResult` | `bool`, thông báo lỗi | Ghi CT/TAT/WT/RT và trung bình |
| `writeGantt(path, result, error)` | đường dẫn, `ScheduleResult` | `bool`, thông báo lỗi | Ghi các đoạn Gantt |
| `writeComparison(path, np, p, error)` | hai `ScheduleResult` | `bool`, thông báo lỗi | Ghi bảng so sánh hai biến thể |
| hàm nội bộ `splitLine`, `trim`, `parseInt` | chuỗi | các trường / số nguyên | Hỗ trợ tách và chuyển đổi, không public |

### 5.3. Quyết định thiết kế

- **Trả về `bool` + tham số `error`** thay vì ném ngoại lệ: dễ hiển thị lỗi lên giao diện Qt và dễ kiểm thử.
- **Không sắp xếp lại dữ liệu**: bảo toàn thứ tự nhập để tie-breaking đúng.
- **Chỉ phụ thuộc `Process`, `ScheduleResult`**: không phụ thuộc scheduler hay Qt, nên test độc lập được.
- **Kiểm tra dữ liệu ngay khi đọc**: scheduler có thể tin rằng đầu vào đã hợp lệ.

### 5.4. Điểm cần thống nhất với nhóm

1. Header CSV chính thức và thứ tự cột.
2. Quy ước `pid = -1` cho IDLE trong Gantt.
3. `ScheduleResult` hiện chỉ có trung bình WT và TAT, chưa có trung bình RT; cần quyết định có thêm không.
4. Có ghi bảng so sánh hai thuật toán trong module CSV hay để module khác đảm nhiệm.

## 6. Kế hoạch kiểm thử độc lập

Test đặt trong `tests/unit/`, chạy được mà không cần scheduler hay giao diện:

| Trường hợp | Kỳ vọng |
|---|---|
| File hợp lệ 3 tiến trình | Trả `true`, đúng 3 `Process`, đúng giá trị và thứ tự |
| Có dòng trống và khoảng trắng thừa | Bỏ qua dòng trống, giá trị vẫn đúng |
| File kết thúc `\r\n` | Đọc đúng |
| Thiếu cột / thừa cột | `false`, lỗi nêu số dòng |
| Giá trị không phải số | `false` |
| `burst <= 0` hoặc `arrival < 0` | `false` |
| `pid` trùng | `false` |
| File không tồn tại, file rỗng, chỉ có header | `false` |
| Ghi rồi đọc lại file kết quả | Nội dung khớp `ScheduleResult` ban đầu |

## 7. Tiến độ

- [x] Nghiên cứu phần phụ trách, thiết kế giao diện, viết tài liệu (tuần này).
- [ ] Cài đặt `CsvHandler` (tuần sau).
- [ ] Viết test độc lập và tạo PR vào `main`.

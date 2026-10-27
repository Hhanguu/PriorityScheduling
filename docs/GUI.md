# GUI module — Priority Scheduling

**Phạm vi:** `src/main.cpp`, `src/ui/MainWindow.*`, `src/ui/GanttChart.*`
**Công nghệ:** Qt6 Widgets, C++17
**Trạng thái:** đã nối `QApplication` → `MainWindow` → `GanttChart`; chọn chạy riêng
NP, riêng P hoặc cả hai, hiện averages/bảng so sánh và xuất file. Có 9 test GUI (96 assertion).

## 1. Vai trò

GUI là lớp trình bày, không tự tính CT/TAT/WT/RT:

1. Chọn và đọc file CSV qua `CsvHandler`.
2. Hiển thị danh sách tiến trình đầu vào.
3. Gọi `NonPreemptiveScheduler` hoặc `PreemptiveScheduler`.
4. Gọi `Metrics::fillAverages` để điền ba giá trị trung bình.
5. Hiển thị bảng kết quả, Gantt và bảng so sánh.
6. Xuất kết quả CSV qua `CsvHandler`.

## 2. Thành phần

| File | Nhiệm vụ |
|---|---|
| `src/main.cpp` | Điểm vào ứng dụng; tạo `QApplication` và cửa sổ chính |
| `src/ui/MainWindow.h/.cpp` | Toolbar, bảng input/result, nhãn averages, chọn thuật toán, đọc/ghi file |
| `src/ui/GanttChart.h/.cpp` | Widget vẽ các đoạn Gantt từ dữ liệu đầu vào |

`MainWindow` đang có:

- `tableInput`: PID, AT, BT, priority.
- `tableResult`: PID, AT, BT, priority, CT, TAT, WT, RT.
- `tableComparison`: averages WT/TAT/RT của NP và P.
- `ganttChart`: biểu đồ Gantt, có nhãn `P<pid>` và `IDLE`.
- `comboMode`: chọn **Chỉ NP**, **Chỉ P** hoặc **Cả hai + so sánh**.
- `comboAlgorithm`: chọn thuật toán đang hiển thị khi chạy cả hai; bị khóa khi chạy riêng.
- `lblAvgWaiting`, `lblAvgTurnaround`, `lblAvgResponse`.
- `btnOpen`, `btnRun`, `btnExport`.

## 3. Luồng sử dụng

```text
Chọn CSV
  -> CsvHandler::readProcesses
  -> hiển thị tableInput
  -> Chọn thuật toán
  -> scheduler::schedule
  -> Metrics::fillAverages
  -> tableResult + averages + Gantt
  -> Xuất CSV
```

Khi đọc file lỗi, GUI hiển thị nguyên chuỗi `error` do `CsvHandler` trả về và giữ dữ liệu cũ.
Khi chưa có dữ liệu mà bấm chạy, GUI hiển thị cảnh báo và không gọi scheduler.

## 4. Quy ước hiển thị

- PID trên giao diện có tiền tố `P`, ví dụ `P1`; CSV chỉ ghi số.
- `pid = -1` trong Gantt hiển thị là `IDLE`.
- Ba giá trị trung bình hiển thị 2 chữ số sau dấu phẩy.
- Thời gian dùng `long long`; widget Gantt phải giữ đúng kiểu này.
- Không sắp xếp lại bảng input; giữ thứ tự dòng CSV.
- Bảng chỉ hiển thị tối đa 10.000 dòng; CSV xuất vẫn đủ. Có nhãn cảnh báo khi bị cắt.
- Gantt có nhiều đoạn hơn số pixel sẽ vẽ thu gọn theo từng cột pixel, tránh treo.

## 5. Xuất CSV

Theo `docs/test-cases/gui.md`, một lần xuất đủ 5 file:

- `np_result.csv`
- `np_gantt.csv`
- `p_result.csv`
- `p_gantt.csv`
- `comparison.csv`

Chế độ **Cả hai** ghi đủ 5 file. Chế độ chạy riêng chỉ ghi 2 file tương ứng; bảng so sánh
hiện `—` ở cột thuật toán chưa chạy.

## 6. Test GUI

Đặc tả: `docs/test-cases/gui.md` (U01–U13, B01–B06).
Dữ liệu: `tests/data/gui/`, kết quả chuẩn: `tests/data/ket_qua/gui/`.

`tests/gui/gui_test.cpp` có 9 test / 96 assertion: nạp CSV, lỗi giữ dữ liệu cũ, chạy
hai thuật toán, chạy riêng NP/P, averages/so sánh, xuất file khớp expected, input rỗng,
file 500 dòng và giới hạn bảng/Gantt thu gọn với 10k dòng.
Chạy headless bằng `QT_QPA_PLATFORM=offscreen` hoặc qua `ctest` (đã đặt biến môi trường).

## 7. Build và chạy

```bash
cmake -S . -B build
cmake --build build
./build/PriorityScheduling
```

Trên Linux không có màn hình có thể dùng `QT_QPA_PLATFORM=offscreen` khi chạy smoke test.

## 8. Việc còn lại

- Kiểm tra thủ công U04 (IDLE), U10 (resize) và U11 (cuộn 500 dòng) trên máy có màn hình.
- Bổ sung test tự động cho nội dung Gantt nếu cần.
- Chụp ảnh/video demo GUI để đưa vào báo cáo.

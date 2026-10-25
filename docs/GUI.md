# GUI module — Priority Scheduling

**Phạm vi:** `src/main.cpp`, `src/ui/MainWindow.*`, `src/ui/GanttChart.*`
**Công nghệ:** Qt6 Widgets, C++17
**Trạng thái:** skeleton đã compile; ứng dụng chưa mở cửa sổ, Gantt chưa được nối vào `MainWindow`.

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

`MainWindow` đang có sẵn:

- `tableInput`: PID, AT, BT, priority.
- `tableResult`: PID, AT, BT, priority, CT, TAT, WT, RT.
- `comboAlgorithm`: chọn Non-preemptive hoặc Preemptive.
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

## 5. Xuất CSV

Theo `docs/test-cases/gui.md`, một lần xuất đủ 5 file:

- `np_result.csv`
- `np_gantt.csv`
- `p_result.csv`
- `p_gantt.csv`
- `comparison.csv`

Hiện `MainWindow` mới ghi 2 file của thuật toán đang chọn; phần comparison và Gantt
trong cửa sổ còn thiếu.

## 6. Test GUI

Đặc tả: `docs/test-cases/gui.md` (U01–U13, B01–B06).
Dữ liệu: `tests/data/gui/`, kết quả chuẩn: `tests/data/ket_qua/gui/`.

`tests/unit/gui/` hiện chỉ có `.gitkeep`; chưa có test GUI tự động.

## 7. Build và chạy

```bash
cmake -S . -B build
cmake --build build
./build/PriorityScheduling
```

Trên Linux không có màn hình có thể dùng `QT_QPA_PLATFORM=offscreen` khi chạy smoke test.

## 8. Việc còn lại

- Nối `QApplication` + `MainWindow` trong `src/main.cpp`.
- Thêm `GanttChart` vào layout và chuyển `ScheduleResult::gantt` sang `GanttBlock`.
- Đổi `GanttBlock` sang `long long`, xử lý IDLE và nhãn `P<pid>`.
- Thêm bảng so sánh và gọi `CsvHandler::writeComparison`.
- Bổ sung test GUI theo U01–U13.

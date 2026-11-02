# Kế hoạch 8 tuần — Priority Scheduling

**Cập nhật:** 2026-11-02
**Mục tiêu:** hoàn thiện ứng dụng Qt6 so sánh Non-preemptive và Preemptive Priority
trên cùng dữ liệu CSV, có test, benchmark và tài liệu nghiệm thu.

## 1. Mốc công việc

| Tuần | Nội dung | Sản phẩm | Trạng thái |
|---|---|---|---|
| 1 | Khung repo, CMake, quy ước, hợp đồng API | `CMakeLists.txt`, `docs/conventions.md`, `docs/api.md` | Xong |
| 2 | Module CSV | `CsvHandler`, test C01–C25 | Xong (PR #39: gom mọi lỗi theo dòng, test đầu-cuối) |
| 3 | Non-preemptive | scheduler, test N01–N12 | Xong (PR #38: 1M còn 242 ms) |
| 4 | Preemptive | scheduler, test P01–P18 | Xong (PR #37: min-heap + event-based) |
| 5 | GUI skeleton + tích hợp | `MainWindow`, `GanttChart`, test U01–U13 | Xong |
| 6 | Hoàn thiện GUI | Gantt, bảng so sánh, xuất đủ 5 file | Xong (PR #40: bảng ảo, bỏ cắt 10.000 dòng) |
| 7 | Benchmark, tối ưu | `benchmark_runner`, `benchmark_np`, `generate_perf`, `report/` | Xong |
| 8 | Nghiệm thu, tài liệu, demo | `ctest`, README, báo cáo, video | Code xong; còn báo cáo cuối và video |

## 2. Trạng thái hiện tại

Đã có trên `main` (sau khi merge #37, #38, #39; #40 đang chờ review):

- Core không phụ thuộc Qt: `Process`, `GanttSegment`, `ScheduleResult`, `Metrics`.
- `NonPreemptiveScheduler` (min-heap) và `PreemptiveScheduler` (min-heap + mô phỏng theo sự kiện).
- `CsvHandler` đọc/ghi CSV, gom mọi lỗi theo dòng (tối đa 100 dòng) thay vì dừng ở lỗi đầu.
- GUI đã nối: nạp CSV, chọn chạy riêng NP/P hoặc cả hai, vẽ Gantt, hiện averages/so sánh,
  xuất file tương ứng. Hai bảng lớn dùng `QTableView` + `ProcessTableModel` nên hiển thị
  đủ mọi dòng, kể cả 1.000.000.
- `tests/unit/` (CSV, NP, P, leader), `tests/integration/` (pipeline thật, không còn stub),
  `tests/gui/`.
- CSV mẫu trong `input/` và `output/`; pseudocode/flowchart trong `docs/algorithms.md`.

### Số đo hiện tại

Máy: i5-11400F, 15 GiB, Ubuntu 24.04, GCC 13.3.0. Bản Release (`-O2 -DNDEBUG`), dữ liệu sinh
bằng `generate_perf --seed 42`. `benchmark_np` lấy trung bình 5 lượt; `benchmark_runner`
đo 1 lượt.

| Số tiến trình | Non-preemptive | Preemptive |
|---|---|---|
| 100.000 | 13,7 ms | 27 ms |
| 200.000 | — | 64 ms |
| 500.000 | — | 242 ms |
| 1.000.000 | 242 ms | 572 ms |
| 2.000.000 | 577 ms | — |

Peak RSS: NP 1M = 106 MiB, NP 2M = 255 MiB.

GUI với file 1.000.000 dòng (offscreen 1180×900): nạp CSV 272 ms, chạy cả hai thuật toán
835 ms, vẽ lại cửa sổ 21 ms, cuộn xuống dòng cuối 1 ms, peak RSS 247 MiB.

### Kiểm thử

| Bộ | Case | Assertion |
|---|---|---|
| `unit_tests` | 79 | 112.207 |
| `integration_tests` | 10 | 318 |
| `gui_tests` | 10 | 208 |
| `ctest` | 3/3 bộ pass | — |

## 3. Việc còn lại

1. Review + merge PR #40 (GUI bỏ giới hạn 10.000 dòng).
2. Báo cáo cuối và video demo.

Đã xử lý trong cùng PR #40: `src/benchmark_main.cpp` không còn đường dẫn mặc định sai
(in usage khi thiếu tham số), và `tests/unit/non-preemptive/np_edge_test.cpp` (N13–N18,
6 case / 226 assertion) đã được commit.

## 4. Điều kiện nghiệm thu

- `cmake --build build` không lỗi, không warning mới.
- `ctest --test-dir build --output-on-failure` chạy `unit_tests`, `integration_tests`, `gui_tests`, không skip.
- `samples/sample.csv` cho đúng kết quả chuẩn NP và P trong `docs/api.md`.
- GUI mở được, nạp CSV, chọn chạy riêng NP/P hoặc cả hai, vẽ Gantt, hiện averages;
  chế độ cả hai xuất đủ 5 file, chế độ riêng xuất 2 file.
- Benchmark ghi `report/` đúng thứ tự; case 1 triệu hoặc chạy xong hoặc báo không đạt, không treo.
- Mỗi PR chỉ chứa file của module, build/test xanh trước khi review.

## 5. Rủi ro

| Rủi ro | Ảnh hưởng | Cách xử lý |
|---|---|---|
| Preemptive `O(n²)` | Không đạt benchmark 100k–1M | Đã xử lý bằng min-heap + event-based: 1M còn 572 ms (cũ: 311 giây cho 100k) |
| `QTableWidget` với dữ liệu lớn | 8 triệu đối tượng cho 1M dòng, treo máy | Đã xử lý bằng `QTableView` + model ảo (PR #40) |
| Test preemptive quá mỏng | Lỗi lọt vào `main` | Đã bù: so full Gantt + averages, P01–P18 |
| Benchmark chạy trên máy khác nhau | Số liệu lệch giữa các thành viên | Ghi rõ cấu hình máy và cờ biên dịch trong mọi báo cáo |

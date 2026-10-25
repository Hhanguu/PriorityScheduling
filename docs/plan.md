# Kế hoạch 8 tuần — Priority Scheduling

**Cập nhật:** 2026-10-25
**Mục tiêu:** hoàn thiện ứng dụng Qt6 so sánh Non-preemptive và Preemptive Priority
trên cùng dữ liệu CSV, có test, benchmark và tài liệu nghiệm thu.

## 1. Mốc công việc

| Tuần | Nội dung | Sản phẩm | Trạng thái |
|---|---|---|---|
| 1 | Khung repo, CMake, quy ước, hợp đồng API | `CMakeLists.txt`, `docs/conventions.md`, `docs/api.md` | Xong |
| 2 | Module CSV | `CsvHandler`, test C01–C25 | Xong |
| 3 | Non-preemptive | scheduler, test N01–N12 | Xong |
| 4 | Preemptive | scheduler, test P01–P18 | Logic xong; test mới có 1 case, chưa đúng fixture |
| 5 | GUI skeleton + tích hợp | `MainWindow`, `GanttChart`, test U01–U13 | Một phần |
| 6 | Hoàn thiện GUI | Gantt, bảng so sánh, xuất đủ 5 file | Chưa |
| 7 | Benchmark, tối ưu | runner đo, `report/`, dãy 1 → 1.000.000 | Chưa |
| 8 | Nghiệm thu, tài liệu, demo | `ctest`, README, báo cáo, dữ liệu mẫu | Một phần |

## 2. Trạng thái hiện tại

Đã có:

- Core không phụ thuộc Qt: `Process`, `GanttSegment`, `ScheduleResult`, `Metrics`.
- `NonPreemptiveScheduler` và `PreemptiveScheduler`.
- `CsvHandler` đọc/ghi CSV.
- Unit test CSV và Non-preemptive; preemptive mới có một case.
- GUI skeleton compile được.

Còn thiếu:

- `src/main.cpp` chưa tạo `QApplication`/`MainWindow`; ứng dụng hiện là CLI demo.
- `MainWindow` chưa có `GanttChart`, bảng so sánh, xuất đủ 5 file.
- `tests/unit/gui/` chưa có test; `tests/integration/` còn stub.
- `src/benchmark/Benchmark.cpp/.h` rỗng; chưa có runner và `report/`.
- Preemptive quét tuần tự `O(n²)`, chưa đáp ứng dãy benchmark lớn.
- PR #33 (`feature/gui`) không build sạch; cần làm lại từ `main`.

## 3. Thứ tự ưu tiên

1. Hoàn thiện test preemptive theo P01–P18 và test tích hợp I01–I08.
2. Nối GUI từ `main`: `main.cpp` → `MainWindow` → `GanttChart` → comparison.
3. Viết benchmark runner, đo dãy nhỏ trước, tối ưu preemptive nếu cần.
4. Dọn test trùng, cập nhật README/docs, kiểm tra build/test trước khi tạo PR.

## 4. Điều kiện nghiệm thu

- `cmake --build build` không lỗi, không warning mới.
- `ctest --test-dir build --output-on-failure` chạy `unit_tests` và `integration_tests`, không skip.
- `samples/sample.csv` cho đúng kết quả chuẩn NP và P trong `docs/api.md`.
- GUI mở được, nạp CSV, chạy cả hai thuật toán, vẽ Gantt, hiện averages và xuất đủ 5 file.
- Benchmark ghi `report/` đúng thứ tự; case 1 triệu hoặc chạy xong hoặc báo không đạt, không treo.
- Mỗi PR chỉ chứa file của module, build/test xanh trước khi review.

## 5. Rủi ro

| Rủi ro | Ảnh hưởng | Cách xử lý |
|---|---|---|
| Preemptive `O(n²)` | Không đạt benchmark 100k–1M | Dùng heap/priority queue hoặc event-based |
| PR #33 chứa file rỗng | Không merge được | Đóng PR, tạo nhánh mới từ `main` |
| Test preemptive quá mỏng | Lỗi lọt vào `main` | Đọc fixture, so full Gantt và averages |
| GUI chưa nối | Không demo được | Ưu tiên sau test core |

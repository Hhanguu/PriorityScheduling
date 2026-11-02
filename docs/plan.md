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
| 4 | Preemptive | scheduler, test P01–P18 | Xong (min-heap + event-based, đủ P01–P18) |
| 5 | GUI skeleton + tích hợp | `MainWindow`, `GanttChart`, test U01–U13 | Xong (local) |
| 6 | Hoàn thiện GUI | Gantt, bảng so sánh, xuất đủ 5 file | Xong (local) |
| 7 | Benchmark, tối ưu | runner đo, `report/`, dãy 1 → 1.000.000 | Chưa |
| 8 | Nghiệm thu, tài liệu, demo | `ctest`, README, báo cáo, dữ liệu mẫu | Một phần |

## 2. Trạng thái hiện tại

Đã có:

- Core không phụ thuộc Qt: `Process`, `GanttSegment`, `ScheduleResult`, `Metrics`.
- `NonPreemptiveScheduler` và `PreemptiveScheduler`.
- `CsvHandler` đọc/ghi CSV.
- Unit test CSV, Non-preemptive và Preemptive đầy đủ P01–P18 (kiểm tra toàn bộ Gantt và averages).
- GUI đã nối: mở cửa sổ, nạp CSV, chọn chạy riêng NP/P hoặc cả hai, vẽ Gantt,
  hiện averages/so sánh, xuất file tương ứng.
- `tests/gui/`: 9 test / 96 assertion, `ctest` có 3 bộ test.
- CSV mẫu trong `input/` và `output/`; pseudocode/flowchart trong `docs/algorithms.md`.

Còn thiếu:

- `tests/integration/` vẫn là stub.
- `src/benchmark/Benchmark.cpp/.h` rỗng; đã có `benchmark_runner` nhưng chưa ghi `report/`.
- PR #33 (`feature/gui`) không build sạch; GUI đang sửa trên nhánh local `feature/gui-fixed`.
- Preemptive đã tối ưu: min-heap + event-based, 100k process ~0,3–0,5 giây (bản Debug).

## 3. Thứ tự ưu tiên

1. Test preemptive P01–P18 đã xong; còn test tích hợp I01–I08.
2. Kiểm tra thủ công GUI U04/U10/U11 trên máy có màn hình; chụp ảnh/video demo.
3. `benchmark_runner` đã có và preemptive đã tối ưu; còn ghi kết quả vào `report/`.
4. Cập nhật README/docs, kiểm tra build/test rồi tạo PR từ nhánh GUI mới.

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
| Preemptive `O(n²)` | Không đạt benchmark 100k–1M | Đã xử lý: min-heap + event-based (`O(n log n + g)`), 100k process ~0,4 giây |
| PR #33 chứa file rỗng | Không merge được | Đóng PR, dùng nhánh local `feature/gui-fixed` |
| Test preemptive quá mỏng | Lỗi lọt vào `main` | Đọc fixture, so full Gantt và averages |
| GUI chưa nối | Không demo được | Ưu tiên sau test core |

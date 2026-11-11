
# Báo cáo Checkpoint 02 — Tiến độ của nhóm & So sánh với Checkpoint 01

## Mô phỏng lập lịch CPU theo độ ưu tiên

### Nhóm 10 — Mã học phần: 012012500105
### Giảng viên: Mai Ngọc Châu
### Ngày: 11/11/2026

---

## 1) Tóm tắt điều hành

Tài liệu này tổng hợp tiến độ của nhóm từ **Checkpoint 01 → Checkpoint 02** cho đồ án Hệ Điều Hành “Priority Scheduling”.

So với Checkpoint 01, Checkpoint 02 đạt được:

- **Hoàn thiện kiểm chứng đúng đắn**: đã có unit + integration + GUI tests và chạy bằng `ctest` (3/3 bộ).
- **Cải thiện hiệu năng lớn**: thuật toán **Preemptive Priority** chuyển sang mô phỏng **event-based**, chạy hiệu quả đến **1.000.000 tiến trình**.
- **Tăng khả năng mở rộng của GUI**: bảng và Gantt được tối ưu để làm việc với dữ liệu rất lớn (kèm **cuộn ngang** khi số segment dày).

---

## 2) So sánh với Checkpoint 01

### 2.1 Checkpoint 01 (mốc baseline)

Checkpoint 01 báo cáo các trạng thái sau (trích từ `CheckPoint/Checkpoint01_10/Checkpoint01_10_Report.pdf`):

**Phạm vi chức năng (đã demo được):**

- Nạp CSV hợp lệ; chạy **Non-preemptive** và **Preemptive** (NP-only, P-only hoặc cả hai).
- Hiển thị CT/TAT/WT/RT; có averages và so sánh NP/P.
- Vẽ **Gantt chart** (kèm IDLE) và xuất file CSV kết quả.

**Kiểm thử:**

- `ctest`: **3/3 bộ pass**.
- **63 test cases, 3408 assertions** tổng cộng.
  - `unit_tests`: 53 case / 3311 assertions (Pass)
  - `integration_tests`: 1 case / 1 assertion (Pass, **stub**)
  - `gui_tests`: 9 case / 96 assertions (Pass)

**Benchmark (tham khảo, trên máy i5-11400F của nhóm, Release O2):**

- Non-preemptive:
  - 100.000: **29 ms**
  - 1.000.000: **0,86 s**
  - 2.000.000: **2,00 s**
- Preemptive:
  - 100.000: **311,35 s**
  - 1.000.000 / 2.000.000: **chưa đo** (ước tính vài giờ)
  - Vấn đề độ phức tạp được nêu: **O(n × tổng burst)** cho Preemptive.

**Giới hạn của GUI:**

- Bảng kết quả trong GUI bị giới hạn **10.000 dòng**.
- Với dữ liệu cỡ **~1.000.000 tiến trình**, GUI không hiển thị đầy đủ; **Gantt bị nén**.

**Các vấn đề còn lại / kế hoạch thực hiện (Checkpoint 01 → 02):**

- Đổi Preemptive sang mô phỏng **event-based + heap**.
- Bổ sung **benchmark runner** (xuất CSV + log có thể tái lập).
- Nâng cấp GUI để mở rộng tốt hơn (bảng ảo + Gantt theo viewport).
- Tăng độ mạnh integration test để xác nhận luồng **CSV → scheduler → output** end-to-end.

---

### 2.2 Checkpoint 02 (trạng thái hiện tại)

Checkpoint 02 đã được hoàn thiện trong repository và được kiểm chứng bởi bộ kiểm thử tự động.

**Kiểm thử (đã tích hợp đầy đủ):**

- `ctest`: **3/3 bộ pass**.
- Tổng: **99 test cases / 112.740 assertions**
  - `unit_tests`: 79 / 112.207
  - `integration_tests`: 10 / 318
  - `gui_tests`: 10 / 215

**Benchmark (cấu hình Release):**

- **Non-preemptive (NP):**
  - 100.000: **13,7 ms**
  - 1.000.000: **242 ms**
  - 2.000.000: **577 ms**
- **Preemptive (P):**
  - 100.000: **27 ms**
  - 500.000: **242 ms**
  - 1.000.000: **572 ms**

**Phương pháp đo benchmark:**

- NP: đo trung bình/median qua 5 lượt chạy.
- P: đo với các cỡ input đại diện (chạy 1 lượt trong runner) sau khi chuyển sang event-based.

**Mở rộng GUI & cải tiến Gantt (tệp ~1.000.000 dòng):**

- `loadCsv`: **272 ms**
- Chạy NP only: **251 ms**
- Chạy cả NP+P: **835 ms**
- Peak RSS: **247 MiB**
- Rendering Gantt chỉ vẽ các segment nhìn thấy và hỗ trợ **cuộn ngang** khi số segment lớn.

---

### 2.3 Bảng tổng kết — Điểm cải thiện chính

| Hạng mục | Checkpoint 01 | Checkpoint 02 |
|---|---|---|
| Mức độ bao phủ kiểm thử | 63 case / 3408 assertions (integration end-to-end chủ yếu còn stub) | 99 case / 112.740 assertions (integration xác thực CSV → scheduler → export) |
| Hiệu năng Preemptive | 100k ~311,35 s; lo ngại độ phức tạp O(n × tổng burst) | Event-based: 1.000.000 ~572 ms |
| Khả năng mở rộng GUI | Bảng giới hạn ~10.000 dòng; không hiển thị được 1M; Gantt bị nén | Bảng ảo `QTableView` + Gantt theo viewport; xem được 1M; cuộn ngang cho dữ liệu dày |
| Tính tái lập benchmark | Đo tay / công cụ tạm | Có `benchmark_runner` + `benchmark_np` + log đưa vào báo cáo |
| Xử lý lỗi CSV | Hiển thị lỗi sớm / ít thông tin | Thu thập lỗi theo dòng (trong giới hạn cho phép) |

---

## 3) Tiến độ theo tuần (mốc công việc)

| Tuần | Hạng mục | Đầu ra | Trạng thái |
|---|---|---|---|
| 1 | Nền tảng dự án | CMake, quy ước, frozen API contract | Done |
| 2 | Module CSV | `CsvHandler`, validate + unit tests (C01–C25) | Done |
| 3 | Non-preemptive Priority | `NonPreemptiveScheduler` + tests (N01–N18) | Done |
| 4 | Preemptive Priority | `PreemptiveScheduler` + tests (P01–P18) | Done |
| 5 | Skeleton GUI + tích hợp | `MainWindow`, `GanttChart`, wiring GUI + tests (U01–U13) | Done |
| 6 | Scalability GUI | Bảng ảo `QTableView` + Gantt cuộn ngang | Done (merge PR #40) |
| 7 | Benchmark & dữ liệu báo cáo | `benchmark_runner`, `benchmark_np`, `generate_perf` | Done |
| 8 | Xác nhận cuối + hoàn thiện tài liệu | `ctest`, hoàn thiện báo cáo + ghi chú demo | Đã xong; báo cáo hoàn thiện |

---

## 4) Trách nhiệm từng thành viên

Trách nhiệm được thống nhất theo danh sách thành viên ở Checkpoint 01, và cập nhật lại cho khớp với các đầu việc thực sự đã tích hợp ở phiên bản cuối.

| STT | Thành viên | MSSV | Trách nhiệm |
|---|---|---|---|
| 1 | Võ Nguyễn Minh Hưng (Leader) | 087207010338 | Kiến trúc, tích hợp, điều phối benchmark, QA/báo cáo cuối |
| 2 | Huỳnh Tuấn Kiệt | 066207016352 | Cài đặt Non-preemptive scheduler + unit tests |
| 3 | Nguyễn Trường Vĩ | 095207004773 | Cài đặt Preemptive scheduler + unit tests |
| 4 | Lê Đình Minh Hưng | 045207004732 | Làm GUI Qt (MainWindow/Gantt/bảng ảo) + GUI tests |
| 5 | Nguyễn Kim Hoàng Lan | 068307000700 | CSV I/O, quy tắc validate, test IO end-to-end |

**Các đầu việc cụ thể (đã merge vào codebase):**

- `Kiệt NP`: `src/core/NonPreemptiveScheduler.{h,cpp}`, `tests/unit/non-preemptive/*`
- `Vĩ P`: `src/core/PreemptiveScheduler.{h,cpp}`, `tests/unit/preemptive/*`
- `Minh Hưng GUI`: `src/ui/MainWindow.{h,cpp}`, `src/ui/GanttChart.{h,cpp}`, `src/ui/ProcessTableModel.{h,cpp}`, `tests/gui/gui_test.cpp`
- `Lan CSV`: `src/io/CsvHandler.{h,cpp}`, `tests/unit/csv/csv_handler_test.cpp`
- `Hưng (Leader)`: tích hợp, công cụ benchmark, tài liệu báo cáo cuối

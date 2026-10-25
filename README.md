# Priority Scheduling

Đề tài **OS_SCH_03** — mô phỏng/hiển thị hai biến thể **lập lịch CPU theo độ ưu tiên**
trên cùng một bộ dữ liệu:

- **Non-preemptive** — tiến trình đã được chọn CPU thì chạy hết Burst Time, không bị ngắt
- **Preemptive** — tiến trình ưu tiên cao hơn đến thì giành CPU ngay

Chương trình đọc danh sách tiến trình từ **CSV**, kiểm tra hợp lệ, tính các chỉ số
thời gian (CT, TAT, WT, RT), hiển thị bảng kết quả và **Gantt Chart trên màn hình**,
xuất kết quả ra **CSV**. Giao diện desktop viết bằng **Qt6 Widgets** (không phải console / Web App).

> Trạng thái: core CSV và Non-preemptive đã xong; test preemptive, integration, GUI và
> benchmark còn thiếu. Xem [docs/plan.md](docs/plan.md).

## Công nghệ

| Thành phần | Công nghệ |
|---|---|
| Ngôn ngữ | C++17 |
| Build | CMake ≥ 3.21 |
| Giao diện | Qt6 Widgets (6.4+) — desktop, ưu tiên Windows, chạy được trên Linux |
| Kiểm thử | doctest (unit), benchmark hiệu năng |

## Cấu trúc dự án

```
src/
  core/       Process, ScheduleResult, GanttSegment,
              NonPreemptiveScheduler, PreemptiveScheduler, Metrics
  io/         CsvHandler — đọc/ghi CSV
  ui/         MainWindow, GanttChart (Qt)
  benchmark/  đo hiệu năng hai thuật toán
samples/      dữ liệu mẫu chuẩn (10 và 20 process) + bộ data benchmark
input/        dữ liệu đầu vào
output/       kết quả ghi ra
report/       số liệu benchmark, tài liệu báo cáo
tests/        unit/ , integration/, third_party/ (doctest)
docs/         api.md + conventions.md + test-cases/ + tài liệu module
```

Quy ước chung của nhóm: [docs/conventions.md](docs/conventions.md)

## Cài đặt

### Windows (môi trường chính)

1. **Cài Qt 6.x**
   - Tải [Qt Online Installer](https://www.qt.io/download-qt-installer).
   - Khi cài, chọn module **Qt 6.x Desktop** và bộ compiler tương ứng:
     - **MinGW 64-bit** (khuyên dùng nếu chưa quen), hoặc
     - **MSVC 2019/2022** (nếu đã có Visual Studio).
   - Ghi lại đường dẫn cài Qt, ví dụ `C:\Qt\6.7.2\mingw_64`.

2. **Cài CMake**
   - Tải bản chính thức: https://cmake.org/download/ → chọn *"Add CMake to the system PATH"* khi cài.

3. **Clone repo**
   ```bat
   git clone https://github.com/Hhanguu/PriorityScheduling.git
   cd PriorityScheduling
   ```

4. **Build — cách dễ nhất (Qt Creator)**
   - Mở Qt Creator → *Open Project* → chọn `CMakeLists.txt` trong thư mục repo.
   - Chọn Kit đã cài ở bước 1 (MinGW hoặc MSVC) → cấu hình đến khi *Configure Complete*.
   - Nhấn nút Run (mũi tên xanh) để build + chạy.

5. **Build bằng dòng lệnh (tùy chọn)**
   ```bat
   cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.7.2\mingw_64
   cmake --build build --config Debug
   ```
   Thay `CMAKE_PREFIX_PATH` bằng đường dẫn Qt của máy bạn. Nếu dùng MinGW, thêm thư mục `bin` của MinGW (ví dụ `C:\Qt\Tools\MinGW132_64\bin`) vào PATH trước khi build.

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install qt6-base-dev cmake g++ git
```

### Lỗi thường gặp

| Thông báo | Nguyên nhân / cách sửa |
|---|---|
| `Could not find a package configuration file provided by "Qt6"` | Sai hoặc thiếu `CMAKE_PREFIX_PATH` (Windows) hoặc chưa cài `qt6-base-dev` (Linux) |
| `No CMAKE_CXX_COMPILER could be found` | Chưa cài compiler (g++ / MinGW / MSVC), hoặc PATH chưa có |
| Lỗi moc/autogen khi build trên Windows | Đang trộn 2 kit (MSVC + MinGW) — dùng lại đúng 1 kit đã chọn ở Qt Creator |

## Chạy chương trình

Sau khi build xong:

**Qt Creator:** nhấn Run.

**Dòng lệnh — Linux:**
```bash
./build/PriorityScheduling
```

**Dòng lệnh — Windows (MinGW):**
```bat
build\PriorityScheduling.exe
```

**Windows (MSVC):**
```bat
build\Debug\PriorityScheduling.exe
```

## Dữ liệu đầu vào

File CSV, header cố định `pid,arrival,burst,priority` (ưu tiên: số nhỏ = ưu tiên cao):

```
pid,arrival,burst,priority
1,0,5,3
2,1,3,2
3,2,8,1
```

Thứ tự dòng quyết định khi các tiến trình hòa điểm — chương trình không sắp xếp lại dữ liệu.
Chi tiết ràng buộc và quy tắc hòa: [docs/conventions.md](docs/conventions.md).

## Dữ liệu đầu ra

Sau khi chạy, chương trình ghi vào `output/`:

| File | Nội dung |
|---|---|
| `np_result.csv` | CT, TAT, WT, RT của từng tiến trình (non-preemptive) |
| `np_gantt.csv` | các đoạn Gantt (pid = -1 là IDLE) |
| `p_result.csv` | như trên, cho preemptive |
| `p_gantt.csv` | Gantt của preemptive |
| `comparison.csv` | trung bình WT, TAT, RT của hai thuật toán cạnh nhau |

## Kiểm thử và hiệu năng

Build kèm test (mặc định `BUILD_TESTING=ON`):

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Hoặc chạy trực tiếp từng bộ test:

```bash
./build/unit_tests
./build/integration_tests
```

- Framework: **doctest** (`tests/third_party/doctest.h`), mỗi bộ test có `doctest_main.cpp` riêng.
- Unit test: `tests/unit/<thành viên>/` — expected vs actual cho case bình thường, IDLE, hòa điểm, preemption, input lỗi.
- Integration test: `tests/integration/` — pipeline `CSV → 2 scheduler → Metrics → ghi file`.
- Benchmark: dãy 1 → 10 → 50 → 100 → 150 → 200 → 500 → 1.000 → 5.000 → 10.000 → 50.000 → 100.000 → 500.000 → 1.000.000 tiến trình, sinh bằng `./build/generate_perf`; runner đo và số liệu trong `report/` đang được bổ sung.
- Test cuối do Leader nghiệm thu: chạy `samples/sample.csv` qua cả hai thuật toán, đối chiếu `output/` với bảng chuẩn trong `docs/conventions.md`.

## Tài liệu và phân công

| Tài liệu | Nội dung |
|---|---|
| [docs/api.md](docs/api.md) | Hợp đồng API v1.0 + bảng kết quả chuẩn |
| [docs/conventions.md](docs/conventions.md) | Quy ước chung — pháp nhân: Leader |
| [docs/test-cases/](docs/test-cases/) | Bộ test case chia theo thành viên |
| [docs/plan.md](docs/plan.md) | Kế hoạch 8 tuần và trạng thái |
| [docs/csv-handler.md](docs/csv-handler.md) | Module CSV I/O |
| [docs/NonPreemptive.md](docs/NonPreemptive.md) | Thuật toán non-preemptive |
| [docs/Preemptive.md](docs/Preemptive.md) | Thuật toán preemptive |
| [docs/GUI.md](docs/GUI.md) | Giao diện Qt |

Nhánh git: `feature/csv`, `feature/non-preemptive`, `feature/preemptive`, `feature/gui` → PR vào `main`.
Mỗi thành viên commit đều đặn hằng tuần và ghi rõ nội dung commit (không để repository im 3 tuần liên tiếp).

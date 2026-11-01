# Tối ưu Non-preemptive Priority Scheduling

Ngày đo: 29/10/2026. Nhánh làm việc: `feature/non-preemptive`, cập nhật từ
`origin/main` tại `c52861f`. Baseline là scheduler tại commit này, không đổi dữ
liệu hoặc cờ biên dịch giữa hai bản.

## Điều kiện và kết quả

- Máy đo: Windows, Intel Core i7-1185G7 @ 3.00 GHz; GNU C++ 16.2.0 / MSYS2 UCRT64.
- CMake Ninja, `CMAKE_BUILD_TYPE=Release` (`-O3 -DNDEBUG`), C++17, 64 bit.
- Dữ liệu: `generate_perf --out input --seed 42 --sizes 1000000 2000000`.
  Arrival ngẫu nhiên từ 0 đến n; BT 1–20, priority 1–10, PID theo dòng.
- Mỗi file chạy trong tiến trình riêng, 5 lượt liên tiếp; không bỏ lượt đầu.
- Chỉ đo lời gọi scheduler bằng `steady_clock`; bao gồm cấp phát đầu ra, loại
  đọc CSV, tính average, checksum, in kết quả và hủy đầu ra.
- Peak RSS là đỉnh bộ nhớ toàn tiến trình qua Windows `GetProcessMemoryInfo`,
  bao gồm đọc CSV và mọi lượt chạy; đơn vị MiB. Không phải RAM riêng của NP.

| Số tiến trình | Bản gốc TB (ms) | Bản tối ưu TB (ms) | Trung vị tối ưu (ms) | Mục tiêu | Kết quả tại máy đo |
|---:|---:|---:|---:|---:|---|
| 1.000.000 | 1.180,746 | 352,995 | 354,193 | < 400 ms | Đạt |
| 2.000.000 | 2.787,690 | 889,527 | 875,522 | < 1.000 ms | Đạt |

Nhanh hơn khoảng 3,35 lần ở 1M và 3,13 lần ở 2M trong phép đo này. Cả 5 lượt
của mỗi kích thước đều dưới ngưỡng yêu cầu.

| Số tiến trình | Peak RSS bản gốc (MiB) | Peak RSS tối ưu (MiB) |
|---:|---:|---:|
| 1.000.000 | 114,000 | 104,730 |
| 2.000.000 | 221,336 | 249,840 |

Ở 2M, peak RSS toàn tiến trình tăng khoảng 28,5 MiB so với baseline cục bộ;
không khẳng định RAM giảm ở mọi kích thước. Số 256 MB do nhóm cung cấp chưa có
cùng phương pháp đo nên không dùng làm phép so trực tiếp. Dữ liệu baseline nhóm
cung cấp (773 ms / 1.856 ms) cũng được tách khỏi baseline đo tại đây. Cần chạy
lại lệnh ở `docs/NonPreemptive.md` §10 trên máy nhóm trưởng trước nghiệm thu.

## Dữ liệu từng lượt

| Bản | n | Lượt 1 | Lượt 2 | Lượt 3 | Lượt 4 | Lượt 5 |
|---|---:|---:|---:|---:|---:|---:|
| Gốc | 1.000.000 | 1163.996 | 1226.396 | 1217.952 | 1147.047 | 1148.338 |
| Tối ưu | 1.000.000 | 357.936 | 360.028 | 349.055 | 354.193 | 343.764 |
| Gốc | 2.000.000 | 2983.360 | 2742.112 | 2680.585 | 2685.557 | 2846.837 |
| Tối ưu | 2.000.000 | 875.522 | 870.215 | 908.405 | 918.355 | 875.138 |

Đơn vị: ms. Checksum của mọi lượt trước/sau khớp:

| n | Checksum |
|---:|---:|
| 1.000.000 | `4457389557638003088` |
| 2.000.000 | `6755014532237243516` |

## Thay đổi thuật toán

1. Thay `stable_sort` chỉ số bằng `sort` với khóa `(AT, vị trí vector)`, giữ đúng
   thứ tự hòa một cách tường minh.
2. Các tiến trình vào ready theo `(AT, vị trí vector)` nên trong cùng priority
   chỉ cần FIFO. Dùng `map` để tìm priority nhỏ nhất trong các nhóm đang sẵn sàng,
   thay heap chứa toàn bộ tiến trình chờ. Không giả định priority chỉ có 1–10;
   số âm, INT_MIN và INT_MAX vẫn được hỗ trợ.
3. FIFO dùng mảng liên kết chỉ số và cặp head/tail cho mỗi priority, tránh tạo
   một deque cho mỗi mức ưu tiên.
4. Dùng chỉ số phụ uint32_t khi kích thước cho phép, có fallback size_t khi vượt
   phạm vi. PID vẫn int; thời gian công khai vẫn long long.
5. Reserve trước n đoạn Gantt; nếu có nhiều IDLE hơn dự kiến, vector vẫn tự mở
   rộng đúng. Không sửa input hoặc cấu trúc kết quả.

Chi phí: O(n log n + n log k), k là số priority đang sẵn sàng; bộ nhớ O(n).
Khi k nhỏ như generator chuẩn, giảm đáng kể chi phí lựa chọn. Không cam kết cùng
mức tăng tốc khi mọi priority đều khác nhau.

Scheduler chỉ điền `processes` và `gantt`, giữ thứ tự input và để 3 average bằng
0 theo I5. Không thêm Qt, I/O hay Metrics vào scheduler.

## Kiểm chứng

- Các test NP có sẵn đối chiếu CSV chuẩn, tie, IDLE, không ngắt và average.
- Thêm oracle O(n²) độc lập, đối chiếu từng trường kết quả/Gantt trên 200 đầu
  vào ngẫu nhiên với seed cố định, gồm đầu vào rỗng và nhiều khoảng IDLE.
- Thêm priority âm, INT_MIN/INT_MAX, nhiều priority khác nhau và cùng priority.
- Unit test sau thay đổi: 55 test, 96.798 assertions đạt, 0 failed, 0 skipped.
- Build Release toàn bộ target thành công. CTest đạt 3/3: unit, integration và GUI offscreen.

CSV 1M/2M là dữ liệu sinh tự động, được bỏ qua trong Git bằng `input/perf_*.csv`.
Không commit binary hoặc dữ liệu benchmark lớn.

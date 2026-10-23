# Test case — Leader

**File test:** `tests/unit/leader/metrics_test.cpp`, `tests/unit/leader/invariants_test.cpp`, `tests/integration/ket_qua_test.cpp`

## 1. Metrics

| ID | Case | Input | Kỳ vọng | Mức |
|---|---|---|---|---|
| M01 | `leader/sample.csv` → NP | kết quả chuẩn NP | WT 8.00, TAT 11.50, RT 8.00 | C |
| M02 | `leader/sample.csv` → P | kết quả chuẩn P | WT 7.80, TAT 11.30, RT 5.30 | C |
| M03 | Rỗng | `{}` | cả 3 = 0.0, không NaN | C |
| M04 | Một process | AT 5, BT 3 | CT 8, TAT 3, WT 0, RT 0 | C |
| M05 | Idempotent | gọi 2 lần | kết quả không đổi | N |
| M06 | Giữ nguyên per-process | gọi fill | CT/TAT/WT/RT không đổi | C |
| M07 | `compare()` | sample NP/P | `waitingTime = {8.00, 7.80}`, `turnaround = {11.50, 11.30}`, `response = {8.00, 5.30}` | C |

## 2. Integration

Pipeline: `CSV → scheduler → Metrics → ghi file → so kết quả chuẩn`.

| ID | Case | Kỳ vọng | Mức |
|---|---|---|---|
| I01 | `leader/sample.csv` → NP → ghi | khớp `ket_qua/leader/sample_np_result.csv` + `ket_qua/leader/sample_np_gantt.csv` | C |
| I02 | `leader/sample.csv` → P → ghi | khớp `ket_qua/leader/sample_p_result.csv` + `ket_qua/leader/sample_p_gantt.csv` | C |
| I03 | `leader/sample.csv` → comparison | khớp `ket_qua/leader/sample_comparison.csv` | C |
| I04 | chạy 2 lần | output giống hệt | C |
| I05 | CSV lỗi | không tạo file output, error trả nguyên vẹn | C |
| I06 | ghi file | chỉ dùng thư mục tạm, không ghi vào `output/` | C |
| I07 | `leader/stress_2000.csv` chạy NP | 2000 process, 2001 đoạn (1 IDLE `-1[0-2]`), phủ 11170, dưới 3 giây | C |
| I08 | `leader/sample_20.csv` | khớp `ket_qua/leader/sample_20_np_result.csv`, `ket_qua/leader/sample_20_np_gantt.csv`, `ket_qua/leader/sample_20_p_result.csv`, `ket_qua/leader/sample_20_p_gantt.csv`, `ket_qua/leader/sample_20_comparison.csv`: NP 21.50/25.50/21.50, P 21.60/25.60/18.45 | C |

## 3. Invariant test dùng chung

Sinh 100–1000 input ngẫu nhiên (AT 0–50, BT 1–10, priority 1–10), chạy cả 2 scheduler,
kiểm tra mọi kết quả:

1. Tổng độ dài các đoạn Gantt của mỗi pid = `burstTime` của pid đó.
2. `max(endTime) == max(completionTime)`.
3. Gantt liên tục từ 0: `gantt[i].endTime == gantt[i+1].startTime`.
4. Không có 2 đoạn liền kề cùng pid (kể cả `-1`).
5. `WT >= 0`, `RT >= 0`, `RT <= WT`.
6. Mọi pid trong Gantt đều có trong input hoặc bằng `-1`.
7. `processes.size()` bằng số input, đúng thứ tự input.
8. `WT = TAT - BT`, `TAT = CT - AT`, `RT = ST - AT`.

## 4. Checklist nghiệm thu cuối

- [ ] Tất cả case `C` xanh
- [ ] `ctest` chạy 2 bộ, không skip
- [ ] Chạy `samples/sample.csv` ra đúng 4 file kết quả chuẩn
- [ ] Không test nào ghi vào `output/`
- [ ] Build không warning mới
- [ ] Mỗi case lỗi có ít nhất 1 assert vào `error`
- [ ] Benchmark chạy đủ dãy 1 → 1.000.000, thứ tự tăng dần

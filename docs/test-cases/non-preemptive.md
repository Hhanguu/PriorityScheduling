# Test case — Non-preemptive

Dữ liệu nằm trong `tests/data/non-preemptive/`. Mọi file case đều **10 process**.
Kết quả mỗi tiến trình ghi dạng (CT, TAT, WT, RT).

## N01 — Sample chuẩn (C)

File: `non-preemptive/sample.csv`

- Gantt: `1[0-5] → 3[5-13] → 6[13-14] → 2[14-17] → 4[17-19] → 8[19-21] → 10[21-23] → 7[23-26] → 5[26-30] → 9[30-35]`
- Trung bình: WT 8.00, TAT 11.50, RT 8.00
- Khớp `ket_qua/non-preemptive/sample_np_result.csv` và `ket_qua/non-preemptive/sample_np_gantt.csv`

## N02 — IDLE đầu (C)

File: `non-preemptive/idle_start.csv` (process đầu đến ở t=5)

- Gantt: `-1[0-5] → 1[5-8] → 3[8-9] → 2[9-11] → 6[11-13] → 5[13-14] → 10[14-16] → 8[16-18] → 4[18-20] → 9[20-21] → 7[21-22]`
- Trung bình: WT 4.00, TAT 5.70, RT 4.00

## N03 — IDLE giữa (C)

File: `non-preemptive/idle_middle.csv` (CPU rảnh 8–20)

- Gantt: `1[0-2] → 2[2-3] → 3[3-5] → 4[5-6] → 5[6-8] → -1[8-20] → 6[20-22] → 7[22-23] → 8[23-25] → 9[25-26] → 10[26-28]`
- Trung bình: WT 1.20, TAT 2.80, RT 1.20

## N04 — Hòa priority, cùng AT (C)

File: `non-preemptive/tie_same_arrival.csv` → pid theo thứ tự input 30, 10, 20, 40, 50, 60, 70, 80, 90, 100

- Gantt: `30[0-5] → 10[5-8] → 20[8-10] → 40[10-14] → 50[14-15] → 60[15-17] → 70[17-20] → 80[20-21] → 90[21-23] → 100[23-25]`
- Trung bình: WT 13.30, TAT 15.80, RT 13.30
- Sort theo pid sẽ ra 10, 20, 30… → **fail**
- Kết quả phải giữ đúng thứ tự input

## N05 — Hòa priority, khác AT (C)

File: `non-preemptive/tie_arrival.csv` (cùng priority 1, AT 2/0/4/6/…)

- Gantt: `2[0-3] → 1[3-6] → 3[6-9] → 4[9-11] → 5[11-12] → 6[12-14] → 7[14-17] → 8[17-18] → 9[18-20] → 10[20-21]`
- Trung bình: WT 2.00, TAT 4.10, RT 2.00

## N06 — Priority nhỏ hơn thắng (C)

File: `non-preemptive/priority_order.csv` (cùng AT 0, priority 1 đến 5)

- Gantt: `4[0-4] → 6[4-8] → 3[8-12] → 8[12-16] → 1[16-20] → 9[20-24] → 5[24-28] → 10[28-32] → 2[32-36] → 7[36-40]`
- Trung bình: WT 18.00, TAT 22.00, RT 18.00

## N07 — Không preempt (C)

File: `non-preemptive/np_no_preempt.csv` → P1 chạy 20 đơn vị, P2–P10 chen vào giữa

- Gantt: `1[0-20] → 2[20-22] → 6[22-24] → 10[24-26] → 3[26-27] → 7[27-28] → 4[28-30] → 8[30-32] → 5[32-33] → 9[33-34]`
- Trung bình: WT 19.70, TAT 23.10, RT 19.70
- P2 priority 1 đến lúc t=1 nhưng **không** ngắt P1
- Cùng file chạy P: P1 bị ngắt ngay t=1 (đối chiếu)

## N08 — Không sort input theo AT (C)

File: `non-preemptive/unsorted_arrival.csv` (pid 1 đến t=10 nhưng đứng đầu file)

- Gantt: `2[0-3] → 4[3-5] → 3[5-6] → 6[6-7] → -1[7-8] → 5[8-10] → 1[10-13] → 7[13-16] → 8[16-18] → 9[18-19] → -1[19-20] → 10[20-22]`
- Trung bình: WT 0.40, TAT 2.40, RT 0.40
- `processes[0].pid == 1` dù P1 hoàn thành sau (kiểm tra I1)

## N09 — Rỗng (C)

- `schedule({})` → `processes` rỗng, `gantt` rỗng, không crash

## N10 — Dữ liệu lớn 100 process (C)

File: `non-preemptive/np_large_100.csv`

- Đủ 100 process, mỗi pid đúng **1 đoạn** Gantt
- Gantt bắt đầu 0, phủ kín 608 đơn vị, không IDLE
- Chạy dưới 1 giây

## N11 — Dữ liệu lớn 1000 process (C)

File: `non-preemptive/np_large_1000.csv`

- Đủ 1000 process, mỗi pid đúng 1 đoạn, IDLE đầu `-1[0-2]`
- Tổng phủ 5460, đoạn cuối kết thúc ở 5460
- Chạy dưới 1 giây

## N12 — Demo 20 process (C)

File: `non-preemptive/sample_20.csv`

- 20 process, Gantt đúng 20 đoạn, phủ `[0-80]`, không IDLE
- Khớp `ket_qua/non-preemptive/sample_20_np_result.csv` và `ket_qua/non-preemptive/sample_20_np_gantt.csv`
- Trung bình: WT 21.50, TAT 25.50, RT 21.50

## Bất biến phải giữ

- Kết quả giữ đúng thứ tự input.
- Gantt phủ kín `[0, max(CT)]`, liên tục.
- Đoạn liền kề cùng pid đã được scheduler gộp.
- Ba trường `average*` để nguyên `0.0`; `Metrics` điền sau.
- `WT = TAT - BT`, `TAT = CT - AT`, `RT = ST - AT`.

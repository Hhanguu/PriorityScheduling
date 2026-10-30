# Test case — Preemptive

Dữ liệu nằm trong `tests/data/preemptive/`. Các case P01–P15 dùng **10 process**;
P16 dùng 100 process, P17 dùng 1000 process, P18 dùng 20 process.
Kết quả mỗi tiến trình ghi dạng (CT, TAT, WT, RT).

## P01 — Sample chuẩn (C)

File: `preemptive/sample.csv`

- Gantt: `1[0-1] → 2[1-2] → 3[2-10] → 6[10-11] → 2[11-13] → 4[13-15] → 8[15-17] → 1[17-21] → 10[21-23] → 7[23-26] → 5[26-30] → 9[30-35]`
- Trung bình: WT 7.80, TAT 11.30, RT 5.30
- Khớp `ket_qua/preemptive/sample_p_result.csv` và `ket_qua/preemptive/sample_p_gantt.csv`

## P02 — RT tính từ lần chạy đầu (C)

Cùng P01. P1 bị ngắt ở t=1, quay lại ở t=17 nhưng `RT = 0`, không được ra 17.

## P03 — Không ngắt khi priority bằng nhau (C)

File: `preemptive/preempt_same_priority.csv` (10 process priority 1, đến lần lượt)

- Gantt: `1[0-3] → 2[3-5] → 3[5-6] → 4[6-8] → 5[8-9] → 6[9-11] → 7[11-12] → 8[12-14] → 9[14-15] → 10[15-17]`
- Trung bình: WT 3.80, TAT 5.50, RT 3.80

## P04 — Nhiều lần ngắt (C)

File: `preemptive/multi_preempt.csv`

- Gantt: `1[0-1] → 2[1-2] → 3[2-3] → 4[3-4] → 5[4-6] → 4[6-7] → 3[7-8] → 7[8-9] → 6[9-10] → 8[10-11] → 2[11-12] → 9[12-14] → 10[14-15] → 1[15-24]`
- Trung bình: WT 3.20, TAT 5.60, RT 0.30
- P4, P3, P2 đều bị ngắt rồi chạy tiếp, không được gộp đoạn

## P05 — Ngắt đúng biên (C)

File: `preemptive/preempt_boundary.csv` (process mới đến đúng lúc đoạn kết thúc)

- Gantt: `1[0-2] → 2[2-4] → 3[4-6] → 4[6-8] → 1[8-10] → 6[10-12] → 7[12-14] → 5[14-16] → 9[16-18] → 10[18-20] → 8[20-22]`
- Trung bình: WT 1.80, TAT 4.00, RT 1.20
- Cấm sinh đoạn 0 giây

## P06 — IDLE đầu (C)

File: `preemptive/idle_start.csv`

- Gantt: `-1[0-5] → 1[5-8] → 3[8-9] → 2[9-10] → 6[10-12] → 2[12-13] → 5[13-14] → 10[14-16] → 8[16-18] → 4[18-20] → 9[20-21] → 7[21-22]`
- Trung bình: WT 4.10, TAT 5.80, RT 3.90

## P07 — IDLE giữa (C)

File: `preemptive/idle_middle.csv`

- Gantt: `1[0-2] → 2[2-3] → 3[3-5] → 4[5-6] → 5[6-8] → -1[8-20] → 6[20-22] → 7[22-23] → 8[23-25] → 9[25-26] → 10[26-28]`
- Trung bình: WT 1.20, TAT 2.80, RT 1.20

## P08 — Priority nhỏ hơn thắng (C)

File: `preemptive/priority_order.csv`

- Gantt: `4[0-4] → 6[4-8] → 3[8-12] → 8[12-16] → 1[16-20] → 9[20-24] → 5[24-28] → 10[28-32] → 2[32-36] → 7[36-40]`
- Trung bình: WT 18.00, TAT 22.00, RT 18.00

## P09 — Hòa priority, khác AT (C)

File: `preemptive/tie_arrival.csv`

- Gantt: `2[0-3] → 1[3-6] → 3[6-9] → 4[9-11] → 5[11-12] → 6[12-14] → 7[14-17] → 8[17-18] → 9[18-20] → 10[20-21]`
- Trung bình: WT 2.00, TAT 4.10, RT 2.00

## P10 — Không sort input theo AT (C)

File: `preemptive/unsorted_arrival.csv`

- Gantt: `2[0-3] → 4[3-5] → 3[5-6] → 6[6-7] → -1[7-8] → 5[8-10] → 1[10-13] → 7[13-15] → 8[15-17] → 7[17-18] → 9[18-19] → -1[19-20] → 10[20-22]`
- Trung bình: WT 0.50, TAT 2.50, RT 0.30
- Kết quả giữ thứ tự input

## P11 — Cùng priority, chạy lần lượt (C)

File: `preemptive/preempt_fcfs.csv`

- Gantt: `1[0-3] → 2[3-5] → 3[5-6] → 4[6-8] → 5[8-11] → 6[11-13] → 7[13-14] → 8[14-16] → 9[16-17] → 10[17-19]`
- Trung bình: WT 4.80, TAT 6.70, RT 4.80

## P12 — Ngắt liên tiếp 9 lần (C)

File: `preemptive/preempt_staircase.csv` → P1 burst 15, P2–P10 burst 1, priority tăng dần

- Gantt: `1[0-1] → 2[1-2] → 3[2-3] → 4[3-4] → 5[4-5] → 6[5-6] → 7[6-7] → 8[7-8] → 9[8-9] → 10[9-10] → 1[10-24]`
- Trung bình: WT 0.90, TAT 3.30, RT 0.00
- P1 đúng 2 đoạn, tổng 15 = BT

## P13 — Bị ngắt rồi quay lại nhiều lần (C)

File: `preemptive/preempt_alternate.csv` → P1 bị ngắt 5 lần

- Gantt: `1[0-1] → 2[1-3] → 1[3-4] → 3[4-6] → 1[6-7] → 4[7-9] → 1[9-10] → 5[10-12] → 6[12-13] → 1[13-14] → 7[14-15] → 1[15-16] → 8[16-17] → 1[17-18] → 9[18-19] → 1[19-20] → 10[20-21] → 1[21-23]`
- Trung bình: WT 1.30, TAT 3.60, RT 0.00
- P1 có 6 đoạn, tổng 10 = BT

## P14 — Hòa priority, cùng AT (C)

File: `preemptive/tie_same_arrival.csv` → pid theo thứ tự input 30, 10, 20, 40, 50, 60, 70, 80, 90, 100

- Gantt: `30[0-5] → 10[5-8] → 20[8-10] → 40[10-14] → 50[14-15] → 60[15-17] → 70[17-20] → 80[20-21] → 90[21-23] → 100[23-25]`
- Trung bình: WT 13.30, TAT 15.80, RT 13.30
- Không được sort theo pid

## P15 — Rỗng (C)

- `schedule({})` → `processes` rỗng, `gantt` rỗng

## P16 — Dữ liệu lớn 100 process (C)

File: `preemptive/p_large_100.csv`

- Đủ 100 process, IDLE đầu `-1[0-2]`, tổng phủ 594
- Số đoạn Gantt **109 > 100** → có ngắt thật
- Mỗi pid: tổng độ dài các đoạn = BT

## P17 — Dữ liệu lớn 1000 process (C)

File: `preemptive/p_large_1000.csv`

- Đủ 1000 process, tổng phủ 5546, đoạn cuối kết thúc ở 5546
- Số đoạn **1069 > 1000**
- Chạy dưới 2 giây

## P18 — Demo 20 process (C)

File: `preemptive/sample_20.csv`

- 20 process, Gantt **25 đoạn**, phủ `[0-80]`
- Khớp `ket_qua/preemptive/sample_20_p_result.csv` và `ket_qua/preemptive/sample_20_p_gantt.csv`
- Trung bình: WT 21.60, TAT 25.60, RT 18.45

## Bất biến phải giữ

- Kết quả giữ đúng thứ tự input.
- Gantt phủ kín `[0, max(CT)]`, liên tục, không đoạn 0 giây.
- Đoạn liền kề cùng pid đã được scheduler gộp.
- Ba trường `average*` để nguyên `0.0`; `Metrics` điền sau.
- `WT = TAT - BT`, `TAT = CT - AT`, `RT = ST - AT`.

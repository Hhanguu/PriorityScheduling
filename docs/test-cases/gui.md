# Test case — GUI

## 1. UI

| ID | Case | Kỳ vọng | Mức |
|---|---|---|---|
| U01 | Nạp `gui/sample.csv` | bảng đúng thứ tự, đủ pid/AT/BT/priority | C |
| U02 | Nạp `gui/err_bad_header.csv` | lỗi hiện trên cửa sổ, không crash, dữ liệu cũ giữ nguyên | C |
| U03 | Chạy NP | bảng + Gantt khớp `ket_qua/gui/sample_np_gantt.csv`; pid hiện `P1` | C |
| U04 | Có IDLE | Gantt hiện chữ `IDLE` | C |
| U05 | Chạy P | Gantt khớp `ket_qua/gui/sample_p_gantt.csv`, thấy đoạn bị ngắt | C |
| U06 | Trung bình | hiện 2 chữ số `8.00`, `7.80`, khớp `ket_qua/gui/sample_*_result.csv` | C |
| U07 | Bảng rỗng | bấm chạy → thông báo, không chia 0 | C |
| U08 | pid trùng khi nhập tay | báo lỗi, không gọi scheduler | N |
| U09 | Xuất CSV | chế độ cả hai: đủ 5 file; chế độ riêng: 2 file tương ứng | C |
| U10 | Resize | Gantt vẽ lại đúng, không tràn | N |
| U11 | Nạp `gui/gui_large_500.csv` | bảng 500 dòng, cuộn mượt, không treo | N |
| U12 | Nạp `gui/sample_20.csv` | bảng 20 dòng, chạy NP/P ra Gantt có ngắt | C |
| U13 | Bảng so sánh | số liệu khớp `ket_qua/gui/sample_comparison.csv` | C |

Lưu ý:

- UI không tự tính CT/TAT/WT/RT hay trung bình; gọi `Metrics::fillAverages`.
- Lỗi đọc file lấy từ chuỗi `error` của `CsvHandler`, không tự viết lại.
- IDLE trong dữ liệu là `-1`; chỉ UI đổi thành chữ "IDLE".

## 2. Benchmark

Chạy theo thứ tự tăng dần:

**1 → 10 → 50 → 100 → 150 → 200 → 500 → 1.000 → 5.000 → 10.000 → 50.000 → 100.000 → 500.000 → 1.000.000** tiến trình.

Sinh dữ liệu (file lớn đã gitignore):

```bash
cmake --build build --target generate_perf
./build/generate_perf                              # tất cả kích thước
./build/generate_perf --sizes 1 10 50 100          # một số kích thước
```

| ID | Case | Kỳ vọng | Mức |
|---|---|---|---|
| B01 | 1–200 process | chạy xong, thời gian >= 0 | C |
| B02 | Cùng input | `processCount` NP và P bằng nhau | C |
| B03 | Đo 3 lần | lấy trung bình, không lấy lần đầu | N |
| B04 | Ghi report | header + số dòng đúng, file trong `report/`, thứ tự tăng dần | C |
| B05 | Không đo đọc file | chỉ đo phần scheduler | N |
| B06 | 500k và 1 triệu | chạy xong hoặc báo thiếu bộ nhớ/thời gian, không crash | N |

Lưu ý: 1 triệu tiến trình chỉ xong nếu scheduler dùng heap/priority queue.
Cài đặt quét tuần tự O(n²) sẽ treo — ghi nhận "không đạt", không tự giảm kích thước.

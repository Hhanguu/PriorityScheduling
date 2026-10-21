# Test case — CSV Handler

Dữ liệu nằm trong `tests/data/csv/`. Đọc đường dẫn bằng `PS_TEST_DATA_DIR`.

## 1. Đọc hợp lệ

| ID | File | Kỳ vọng | Mức |
|---|---|---|---|
| C01 | `csv/sample.csv` | 10 process, thứ tự pid 1→10; P1 (0,5,3), P2 (1,3,2), P3 (2,8,1) | C |
| C02 | `csv/crlf_sample.csv` | giống C01 (file dùng `\r\n`) | C |
| C03 | file có dòng trống giữa/cuối + khoảng trắng thừa | giống C01 | C |
| C04 | `csv/unsorted_arrival.csv` | giữ đúng thứ tự dòng, **không sort lại** | C |
| C05 | `csv/sample_20.csv` | đọc đủ 20 process, giữ thứ tự | N |
| C06 | đọc thành công | `error.empty() == true` | C |

## 2. Đọc lỗi

| ID | File | Kỳ vọng (chứa) | Mức |
|---|---|---|---|
| C07 | `csv/err_bad_header.csv` | `false`, "Dòng 1", "header" | C |
| C08 | `csv/err_missing_column.csv` | `false`, "Dòng 2", "4 cột" | C |
| C09 | `csv/err_not_number.csv` | `false`, "Dòng 2", "arrival" | C |
| C10 | `csv/err_negative_arrival.csv` | `false`, "Dòng 2", "arrival" | C |
| C11 | `csv/err_zero_burst.csv` | `false`, "Dòng 2", "burst" | C |
| C12 | `csv/err_duplicate_pid.csv` | `false`, "Dòng 3", "trùng" | C |
| C13 | `csv/err_header_only.csv` | `false`, "không có tiến trình" | C |
| C14 | đường dẫn không tồn tại | `false`, "Không mở được file" | C |
| C15 | đọc lỗi | `out` rỗng hoặc không dùng tiếp | N |

## 3. Ghi

| ID | Case | Kỳ vọng | Mức |
|---|---|---|---|
| C16 | ghi kết quả NP | khớp `ket_qua/csv/sample_np_result.csv` | C |
| C17 | ghi Gantt NP | khớp `ket_qua/csv/sample_np_gantt.csv` | C |
| C18 | ghi kết quả P | khớp `ket_qua/csv/sample_p_result.csv` | C |
| C19 | ghi Gantt P | khớp `ket_qua/csv/sample_p_gantt.csv` | C |
| C20 | ghi comparison | khớp `ket_qua/csv/sample_comparison.csv` | C |
| C21 | IDLE | ghi `pid = -1`, không ghi "IDLE" | C |
| C22 | mọi file | byte cuối là `\n` | N |
| C23 | đường dẫn không ghi được | `false` + error, không crash | N |
| C24 | `csv/large_1000.csv` | đọc đủ 1000 process, giữ thứ tự pid 1→1000, không mất dòng | C |
| C25 | ghi `sample_20` | khớp 5 file `ket_qua/csv/sample_20_*.csv` (NP, P, comparison) | C |

## Ghi chú

- Không sắp xếp lại dữ liệu khi đọc.
- `error` phải rỗng khi thành công.
- Không hard-code đường dẫn; hàm nhận `path` từ nơi gọi.

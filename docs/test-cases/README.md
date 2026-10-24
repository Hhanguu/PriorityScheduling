# BỘ TEST CASE — chia theo thành viên

Đây là **đặc tả test case**, không phải code test. Mỗi thành viên tự viết unit test
cho module mình trong `tests/unit/<thành viên>/`; Leader viết test tích hợp trong `tests/integration/`.

Kỳ vọng lấy từ `docs/api.md` và `docs/conventions.md`. Nếu code khác tài liệu này thì
code sai, không phải test sai.

| File | Người phụ trách | Case |
|---|---|---|
| [csv.md](csv.md) | Thành viên CSV | C01–C25 |
| [non-preemptive.md](non-preemptive.md) | Thành viên NP | N01–N12 |
| [preemptive.md](preemptive.md) | Thành viên P | P01–P18 |
| [gui.md](gui.md) | Thành viên GUI | U01–U13, B01–B06 |
| [leader.md](leader.md) | Leader | M01–M07, I01–I08, invariant |

## Dữ liệu test

`tests/data/` chia theo thành viên:

| Thư mục | Dùng cho |
|---|---|
| `tests/data/csv/` | C01–C25 |
| `tests/data/non-preemptive/` | N01–N12 |
| `tests/data/preemptive/` | P01–P18 |
| `tests/data/gui/` | U01–U13 |
| `tests/data/leader/` | input M01–M07, I01–I08 |
| `tests/data/ket_qua/csv/` | kết quả cho C16–C20, C25 |
| `tests/data/ket_qua/non-preemptive/` | kết quả cho N01, N12 |
| `tests/data/ket_qua/preemptive/` | kết quả cho P01, P18 |
| `tests/data/ket_qua/gui/` | kết quả cho U03, U05, U06, U13 |
| `tests/data/ket_qua/leader/` | kết quả cho M01–M02, I01–I03, I08 |

Mỗi thư mục dữ liệu chỉ chứa input. Kết quả để so sánh nằm trong `tests/data/ket_qua/<thành viên>/`.

## Quy ước

- `C` = bắt buộc, `N` = nên có.
- Đọc file bằng `PS_TEST_DATA_DIR` (CMake đã định nghĩa), ví dụ:
  `std::string(PS_TEST_DATA_DIR) + "/csv/sample.csv"`.
- Test không ghi vào `output/`; dùng `std::filesystem::temp_directory_path()`.
- Test lỗi CSV chỉ so **"Dòng N" + tên trường**, không so nguyên câu.
- Kết quả scheduler phải giữ bất biến I1–I8 trong `docs/api.md`.

## Chạy

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Sinh dữ liệu benchmark:

```bash
cmake --build build --target generate_perf
./build/generate_perf
```

# BÁO CÁO TÀI LIỆU - PREEMPTIVE PRIORITY SCHEDULING

## 1. Nghiên cứu về phần mình đảm nhiệm (Thuật toán Lập lịch Ưu tiên Độc quyền)
- **Khái niệm:** Là thuật toán lập lịch CPU trong đó mỗi tiến trình được gán một số nguyên gọi là độ ưu tiên (Priority). CPU sẽ được cấp phát cho tiến trình có độ ưu tiên cao nhất.
- **Quy ước:** Số Priority càng nhỏ thì độ ưu tiên càng cao (Ví dụ: Priority 1 > Priority 2).
- **Cơ chế Preemptive (Độc quyền):** Nếu một tiến trình mới đến có Priority cao hơn tiến trình đang chạy, hệ thống sẽ ngay lập tức tạm dừng (preempt) tiến trình hiện tại, đưa nó trở lại hàng đợi (ready queue) và cấp CPU cho tiến trình mới.
- **Xử lý xung đột:** Nếu hai tiến trình có cùng Priority, hệ thống sẽ ưu tiên tiến trình có Arrival Time (AT) nhỏ hơn (đến sớm hơn). Nếu vẫn bằng nhau, áp dụng FCFS (First Come First Served) dựa trên thứ tự nhập (PID).

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần
- **Struct `Process`:** Đại diện cho một tiến trình. Chứa các thuộc tính: `pid` (Mã tiến trình), `arrival_time` (AT), `burst_time` (BT), `priority` (Độ ưu tiên). Ngoài ra còn chứa các thuộc tính đầu ra: `completion_time` (CT), `turnaround_time` (TAT), `waiting_time` (WT), `response_time` (RT), `start_time` (ST) và `remaining_burst_time` (thời gian còn lại).
- **Struct `ScheduleResult`:** Đóng gói kết quả trả về bao gồm: Danh sách các tiến trình đã xử lý, và các giá trị trung bình (`avg_tat`, `avg_wt`, `avg_rt`).
- **Hàm `priorityPreemptive(vector<Process> processes)`:** Thực hiện thuật toán lập lịch ưu tiên độc quyền. Trả về đối tượng `ScheduleResult`.
- **Hàm `calculateMetrics(ScheduleResult& result)`:** Tính toán các chỉ số TAT, WT, RT dựa trên công thức:
  - TAT = CT - AT
  - WT = TAT - BT
  - RT = ST - AT

## 3. Hướng dẫn sử dụng, cách gọi hàm, đầu vào, đầu ra và ví dụ minh họa
- **Đầu vào:** Một vector các đối tượng `Process`.
- **Cách gọi hàm:**
  ```cpp
  vector<Process> process_list = {
      Process(1, 0, 5, 2), // P1: AT=0, BT=5, Pri=2
      Process(2, 1, 3, 1), // P2: AT=1, BT=3, Pri=1
      Process(3, 2, 4, 3)  // P3: AT=2, BT=4, Pri=3
  };
  ScheduleResult result = priorityPreemptive(process_list);

  ## 4. Flow hoạt động (Từ đầu vào đến đầu ra)
1. Khởi tạo: Nhận danh sách tiến trình. Khởi tạo current_time = 0, completed = 0.
2. Vòng lặp lập lịch: Chạy cho đến khi tất cả tiến trình hoàn thành.
   - Bước 1: Duyệt qua tất cả tiến trình, tìm tiến trình thỏa mãn: AT <= current_time và remaining_burst_time > 0.
   - Bước 2: Nếu không tìm thấy, tăng current_time lên 1 (CPU IDLE) và quay lại Bước 1.
   - Bước 3: Trong số các tiến trình thỏa mãn, chọn tiến trình có Priority nhỏ nhất. Nếu cùng Priority, chọn tiến trình có AT nhỏ hơn. Nếu vẫn bằng, chọn PID nhỏ hơn.
   - Bước 4: Ghi nhận start_time nếu là lần đầu chạy. Giảm remaining_burst_time đi 1. Tăng current_time lên 1.
   - Bước 5: Nếu remaining_burst_time == 0, cập nhật completion_time = current_time, tăng completed lên 1.
3. Tính toán: Sau khi tất cả hoàn thành, tính TAT, WT, RT cho từng tiến trình và tính trung bình.
4. Trả về: Đóng gói vào ScheduleResult và trả về.

## 5. Cấu trúc và thiết kế
- File main.cpp: Chứa hàm main để chạy thử nghiệm và in kết quả ra màn hình.
- File scheduler.h: Chứa khai báo struct Process, ScheduleResult và prototype các hàm.
- File scheduler.cpp: Chứa phần triển khai (implementation) các hàm thuật toán.
- File test_scheduler.cpp: Chứa các bài test độc lập (Unit Test) để kiểm tra tính đúng đắn của từng hàm trước khi tích hợp vào main.

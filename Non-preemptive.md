# Tài liệu Non-preemptive Priority Scheduling

**Phần phụ trách:** Thuật toán lập lịch CPU theo độ ưu tiên **không ngắt**.  
**Công việc tuần này:** Nghiên cứu và viết tài liệu; chưa yêu cầu triển khai code.

## 1. Chức năng và nhiệm vụ

Non-preemptive Priority Scheduling chọn một tiến trình đang sẵn sàng để CPU xử lý. **Khi đã bắt đầu chạy, tiến trình sẽ chạy đến khi hoàn thành**, không bị tiến trình mới đến ngắt giữa chừng.

Phần này cần:
- Chọn đúng tiến trình theo quy tắc ưu tiên của đề tài.
- Xử lý khoảng thời gian CPU không có tiến trình sẵn sàng (**IDLE**).
- Tính kết quả thời gian của từng tiến trình và các giá trị trung bình.
- Cung cấp các đoạn thực thi để biểu diễn bằng **Gantt Chart**.

## 2. Quy tắc lập lịch

Tại mỗi thời điểm CPU rảnh, chỉ xét các tiến trình **đã đến và chưa hoàn thành**.

1. **Priority có giá trị nhỏ hơn thì độ ưu tiên cao hơn.**
2. Nếu Priority bằng nhau, chọn tiến trình có **Arrival Time (AT) nhỏ hơn**.
3. Nếu Priority và AT cùng bằng nhau, chọn theo **thứ tự nhập ban đầu**.
4. Tiến trình đã được chọn sẽ chạy hết **Burst Time (BT)**, không bị ngắt.
5. Nếu không có tiến trình sẵn sàng, CPU ở trạng thái **IDLE** cho đến khi có tiến trình đến.

**Starvation:** Tiến trình có độ ưu tiên thấp có thể phải chờ lâu khi các tiến trình ưu tiên cao liên tục được chọn. Đây là hạn chế cần nêu trong phần phân tích thuật toán; đề bài hiện chưa bắt buộc cài đặt cơ chế khắc phục.

## 3. Đầu vào, đầu ra và cách sử dụng

**Đầu vào:** Danh sách tiến trình `Process`, trong đó thông tin cần thiết cho thuật toán gồm:

| Thông tin | Ý nghĩa |
|---|---|
| Mã tiến trình | Phân biệt từng tiến trình |
| AT (Arrival Time) | Thời điểm tiến trình đến |
| BT (Burst Time) | Tổng thời gian CPU cần để xử lý |
| Priority | Mức ưu tiên của tiến trình |
| Thứ tự nhập | Dùng để phân xử nếu Priority và AT đều bằng nhau |

**Cách sử dụng ở mức module:** Chương trình chung cung cấp danh sách `Process` cho thuật toán Non-preemptive; thuật toán trả về `ScheduleResult`. **Tên hàm, đường dẫn file và cú pháp gọi cụ thể cần lấy từ mã nguồn chung của nhóm**, chưa được xác nhận nên không ghi cố định ở đây.

**Đầu ra:** Cấu trúc dùng chung `ScheduleResult`, bao gồm:
- Kết quả của từng tiến trình.
- Các đoạn trên Gantt Chart, gồm tiến trình chạy (hoặc IDLE), thời điểm bắt đầu và kết thúc.
- Các chỉ số thời gian trung bình.

### Công thức tính

| Ký hiệu | Ý nghĩa / cách tính |
|---|---|
| AT | Arrival Time — thời điểm đến |
| BT | Burst Time — thời gian CPU cần |
| ST | Start Time — thời điểm được chạy lần đầu |
| CT | Completion Time — thời điểm hoàn thành |
| TAT | Turnaround Time = CT − AT |
| WT | Waiting Time = TAT − BT |
| RT | Response Time = ST − AT |

Các chỉ số trung bình được tính bằng tổng chỉ số của các tiến trình chia cho số tiến trình (với danh sách không rỗng).

## 4. Luồng xử lý (Flow)

1. Nhận danh sách tiến trình và bắt đầu tại thời điểm mô phỏng `t = 0`.
2. Tìm các tiến trình có `AT ≤ t` và chưa hoàn thành.
3. Nếu **không có** tiến trình sẵn sàng: ghi đoạn **IDLE**, chuyển `t` đến thời điểm đến gần nhất của tiến trình chưa chạy.
4. Nếu **có** tiến trình sẵn sàng: chọn theo **Priority → AT → thứ tự nhập**.
5. Cho tiến trình được chọn chạy **liên tục đến khi hoàn thành**.
6. Ghi đoạn Gantt, cập nhật thời gian và tính **CT, TAT, WT, RT**.
7. Lặp lại bước 2 đến khi mọi tiến trình hoàn thành.
8. Tính các giá trị trung bình và trả kết quả.

## 5. Cấu trúc và thiết kế

Theo quy định chung của đề tài:

- **`Process`**: Cấu trúc dữ liệu đầu vào được **cả Non-preemptive và Preemptive sử dụng chung**.
- **`ScheduleResult`**: Cấu trúc đầu ra chung, lưu kết quả từng tiến trình, các đoạn Gantt và các giá trị trung bình.
- **Module Non-preemptive**: Nhận dữ liệu `Process`, áp dụng quy tắc không ngắt và tạo `ScheduleResult`.
- **Thành phần hiển thị/so sánh**: Sử dụng kết quả được trả về; không phải nhiệm vụ xử lý lập lịch của module này.

**Không liệt kê cấu trúc thư mục, tên file, class hoặc chữ ký hàm chưa được kiểm chứng từ repository của nhóm.** Khi nhóm xác nhận các thành phần này, bổ sung đường dẫn và giao diện hàm thực tế vào tài liệu.

## 6. Kiểm thử cần chuẩn bị

Trước khi tạo Pull Request (PR) vào `main`, module cần được kiểm thử độc lập với các trường hợp:

- Chỉ có một tiến trình.
- Tiến trình ưu tiên cao đến khi CPU đang bận nhưng **không được ngắt** tiến trình đang chạy.
- Nhiều tiến trình cùng Priority: xét AT rồi xét thứ tự nhập.
- CPU có khoảng **IDLE**.
- Kết quả **CT, TAT, WT, RT**, các đoạn Gantt và giá trị trung bình được tính đúng.

**Trạng thái hiện tại:** Tài liệu nghiên cứu và thiết kế; chưa khẳng định đã có code hay test hoàn thành.

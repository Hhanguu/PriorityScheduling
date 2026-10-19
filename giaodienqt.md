# BÁO CÁO NHIỆM VỤ: TÀI LIỆU THIẾT KẾ VÀ HƯỚNG DẪN GIAO DIỆN QT (GUI)

**Dự án:** Mô phỏng Thuật toán Lập lịch Tiến trình theo Mức độ Ưu tiên (Priority Scheduling)

**Môn học:** Hệ Điều Hành

## 1. Nghiên cứu về phần đảm nhiệm

Phần công việc đảm nhiệm tập trung vào việc nghiên cứu, thiết kế và phát thảo giao diện người dùng đồ họa (GUI) sử dụng **Qt Framework (C++)** nhằm hỗ trợ trực quan hóa thuật toán lập lịch **Priority Scheduling** (gồm cả hai chế độ *Preemptive* và *Non-Preemptive*).

### Phát thảo mẫu và Định hướng thiết kế

* **Mục tiêu:** Cung cấp môi trường trực quan cho phép người dùng cấu hình danh sách tiến trình, tùy chọn thuật toán, thực thi và theo dõi kết quả lập lịch dưới dạng Biểu đồ Gantt (Gantt Chart) và Bảng số liệu thống kê.

* **Quy định chung của dự án:**

  * **Độ ưu tiên (Priority):** Giá trị số càng nhỏ thì độ ưu tiên càng cao (ví dụ: Priority 1 cao hơn Priority 2). Nếu hai tiến trình có cùng Priority, tiến trình có $AT$ (Arrival Time) nhỏ hơn được chọn trước; nếu $AT$ bằng nhau thì ưu tiên theo thứ tự nhập vào.

  * **Trạng thái CPU:** Nếu không có tiến trình nào sẵn sàng trong danh sách chờ, CPU chuyển sang trạng thái nhàn rỗi (IDLE).

  * **Chuẩn hóa ký hiệu & công thức tính toán:**

    * $AT$ (Arrival Time): Thời điểm tiến trình đến hệ thống.

    * $BT$ (Burst Time): Tổng thời gian CPU cần để thực thi tiến trình.

    * $CT$ (Completion Time): Thời điểm tiến trình hoàn tất.

    * $TAT$ (Turnaround Time): Thời gian lưu lại trong hệ thống, tính bằng $TAT = CT - AT$.

    * $WT$ (Waiting Time): Thời gian chờ trong hàng đợi, tính bằng $WT = TAT - BT$.

    * $RT$ (Response Time): Thời gian phản hồi lần đầu, tính bằng $RT = ST - AT$ (với $ST$ là Start Time - thời điểm bắt đầu thực thi lần đầu).

## 2. Mô tả các chức năng chính và nhiệm vụ của từng thành phần

### 2.1. Cửa sổ làm việc chính (`MainWindow`)

* **Tiếp nhận & Quản lý dữ liệu đầu vào:**

  * Cho phép thêm, xóa, chỉnh sửa các tham số của từng tiến trình $P_i$: Process ID, $AT$, $BT$, Priority.

  * Lựa chọn loại thuật toán lập lịch: **Preemptive Priority** hoặc **Non-Preemptive Priority**.

  * Cung cấp chức năng nạp dữ liệu từ tệp mẫu (`samples/input/`) hoặc nhập trực tiếp từ bảng giao diện.

* **Điều khiển & Gọi thực thi:** Khởi chạy quá trình tính toán bằng cách đóng gói danh sách tiến trình và truyền sang module thuật toán xử lý (`core`).

* **Hiển thị kết quả:** Cập nhật bảng kết quả chứa các chỉ số $CT, TAT, WT, RT$ cho từng tiến trình và hiển thị giá trị trung bình tổng thể ($\overline{TAT}, \overline{WT}, \overline{RT}$).

### 2.2. Biểu đồ Gantt (`GanttChart`)

* **Vẽ tiến trình CPU:** Biểu diễn các khoảng thời gian chiếm dụng CPU của từng tiến trình dưới dạng các khối chữ nhật có màu sắc phân biệt.

* **Hiển thị phân đoạn Preemptive:** Trực quan hóa chính xác các khoảng tiến trình bị ngắt (preempted) và chèn ngang khi có tiến trình có độ ưu tiên cao hơn xuất hiện.

* **Trạng thái IDLE:** Hiển thị rõ ràng các phân đoạn CPU rảnh (IDLE) trên trục thời gian.

## 3. Hướng dẫn sử dụng, cách gọi hàm, đầu vào, đầu ra và ví dụ minh họa

### 3.1. Cấu trúc dữ liệu chung (Data Structures)

Mọi tương tác giữa GUI và Core đều sử dụng thống nhất các cấu trúc dữ liệu sau:

```
// Cấu trúc mô tả một tiến trình
struct Process {
    std::string id;  // Mã tiến trình (VD: "P1", "P2")
    int arrivalTime;  // Thời điểm đến (AT)
    int burstTime;    // Thời gian thực thi (BT)
    int priority;     // Độ ưu tiên (Giá trị nhỏ = Ưu tiên cao)
};

// Cấu trúc mô tả một phân đoạn trên biểu đồ Gantt
struct GanttSegment {
    std::string processId; // ID tiến trình hoặc "IDLE"
    int startTime;         // Thời điểm bắt đầu phân đoạn
    int endTime;           // Thời điểm kết thúc phân đoạn
};

// Cấu trúc lưu kết quả thống kê của từng tiến trình
struct ProcessMetrics {
    std::string id;
    int completionTime; // CT
    int turnaroundTime; // TAT = CT - AT
    int waitingTime;    // WT = TAT - BT
    int responseTime;   // RT = ST - AT
};

// Cấu trúc trả về kết quả lập lịch chung (ScheduleResult)
struct ScheduleResult {
    std::vector<GanttSegment> ganttSegments;    // Danh sách phân đoạn Gantt
    std::vector<ProcessMetrics> processMetrics; // Kết quả chỉ số từng tiến trình
    double averageTurnaroundTime;               // TAT trung bình
    double averageWaitingTime;                  // WT trung bình
    double averageResponseTime;                 // RT trung bình
};

```

### 3.2. Cách gọi hàm & Luồng dữ liệu (Input/Output)

* **Đầu vào (Input):** Danh sách `std::vector<Process>` được lấy từ dữ liệu nhập trên bảng giao diện `QTableWidget` cùng cấu hình loại thuật toán được chọn từ `QComboBox`.

* **Cách gọi hàm xử lý:**

  ```
  // Ví dụ trong MainWindow.cpp khi người dùng bấm nút "Run"
  std::vector<Process> processes = getInputProcessesFromTable();
  bool isPreemptive = ui->comboAlgorithm->currentIndex() == 1;
  
  // Gọi Module Core tính toán
  ScheduleResult result = Scheduler::runPriorityScheduling(processes, isPreemptive);
  
  // Cập nhật Đầu ra (Output) lên Giao diện
  ui->ganttChartWidget->setGanttData(result.ganttSegments);
  updateMetricsTable(result.processMetrics);
  updateAverageLabels(result.averageTurnaroundTime, result.averageWaitingTime, result.averageResponseTime);
  
  ```

### 3.3. Ví dụ minh họa

**Dữ liệu đầu vào:**

| Process ID | Arrival Time ($AT$) | Burst Time ($BT$) | Priority | 
| ----- | ----- | ----- | ----- | 
| P1 | 0 | 4 | 2 | 
| P2 | 1 | 3 | 1 | 
| P3 | 2 | 1 | 3 | 

**Kết quả đầu ra (Preemptive Priority):**

* **Biểu đồ Gantt segments:** `[P1: 0-1] -> [P2: 1-4] -> [P1: 4-7] -> [P3: 7-8]`

* **Bảng chỉ số chi tiết:**

| Process | $AT$ | $BT$ | Priority | $CT$ | $TAT = CT-AT$ | $WT = TAT-BT$ | $RT = ST-AT$ | 
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | 
| P1 | 0 | 4 | 2 | 7 | $7 - 0 = 7$ | $7 - 4 = 3$ | $0 - 0 = 0$ | 
| P2 | 1 | 3 | 1 | 4 | $4 - 1 = 3$ | $3 - 3 = 0$ | $1 - 1 = 0$ | 
| P3 | 2 | 1 | 3 | 8 | $8 - 2 = 6$ | $6 - 1 = 5$ | $7 - 2 = 5$ | 

* **Chỉ số trung bình:**

  * $\overline{TAT} = (7 + 3 + 6) / 3 = 5.33$

  * $\overline{WT} = (3 + 0 + 5) / 3 = 2.67$

  * $\overline{RT} = (0 + 0 + 5) / 3 = 1.67$

## 4. Flow hoạt động (Luồng xử lý hệ thống)

$$
\text{Người dùng nhập dữ liệu (GUI)} \xrightarrow{\quad\text{Bấm 'Run'}\quad} \text{MainWindow::on\_btnRun\_clicked()}
$$

$$
\downarrow
$$

$$
\text{Kiểm tra tính hợp lệ dữ liệu (Validate Input)}
$$

$$
\downarrow
$$

$$
\text{Đóng gói } \texttt{std::vector<Process>} \xrightarrow{\quad\text{Truyền dữ liệu}\quad} \text{Core Scheduler Module}
$$

$$
\downarrow
$$

$$
\text{Tính toán Thuật toán Lập lịch (Priority Preemptive / Non-Preemptive)}
$$

$$
\downarrow
$$

$$
\text{MainWindow nhận trả về } \texttt{ScheduleResult}
$$

$$
\downarrow
$$

$$
\text{Cập nhật } \texttt{GanttChart::setGanttData()} \quad\&\quad \text{Cập nhật Bảng } \texttt{ProcessMetrics} + \overline{TAT}, \overline{WT}, \overline{RT}
$$

### Chi tiết các bước thực hiện:

1. **Khởi tạo:** Người dùng mở ứng dụng, giao diện `MainWindow` hiển thị bảng rỗng hoặc nạp tệp cấu hình mẫu.

2. **Nhập & Kiểm tra dữ liệu:** Người dùng nhập danh sách tiến trình. `MainWindow` thực hiện kiểm tra tính hợp lệ (ví dụ: $AT \ge 0$, $BT > 0$, Priority không rỗng).

3. **Kích hoạt tính toán:** Người dùng nhấn nút **"Chạy lập lịch"**. `MainWindow` trích xuất thông tin, đóng gói thành danh sách `Process` và gọi hàm xử lý thuật toán ở lớp Core.

4. **Xử lý trung tâm (Core):** Thuật toán lập lịch căn cứ vào $AT$, $BT$, Priority để sắp xếp thứ tự thực thi, tạo ra các danh sách phân đoạn `GanttSegment` và tính toán chỉ số $CT, TAT, WT, RT$. Tất cả được đóng gói lại trong đối tượng `ScheduleResult`.

5. **Hiển thị kết quả:**

   * `MainWindow` chuyển danh sách `GanttSegment` sang cho widget `GanttChart` để thực hiện ghi đè hàm vẽ `paintEvent()` biểu diễn biểu đồ Gantt.

   * Cập nhật các dòng dữ liệu kết quả lên bảng thống kê và ghi các chỉ số trung bình ra màn hình giao diện.

## 5. Cấu trúc và thiết kế (Phần UI trong Repository)

Đường dẫn lưu trữ các tệp thuộc thành phần Giao diện trong repository dự án: `src/ui/`.

```
src/
└── ui/
    ├── MainWindow.h
    ├── MainWindow.cpp
    ├── GanttChart.h
    └── GanttChart.cpp

```

### Giải thích các File, Class và Function quan trọng:

#### 1. `src/ui/MainWindow.h` & `src/ui/MainWindow.cpp`

* **Lớp `MainWindow` (Thừa kế từ `QMainWindow`):** Là cửa sổ chính điều khiển toàn bộ luồng tương tác của giao diện.

* **Các hàm quan trọng:**

  * `explicit MainWindow(QWidget *parent = nullptr)`: Khởi tạo các thành phần giao diện, thiết lập bố cục layout, kết nối tín hiệu (`signals & slots`).

  * `void setupTables()`: Cấu hình cột, tiêu đề cho bảng nhập liệu và bảng hiển thị kết quả.

  * `std::vector<Process> getProcessesFromUI()`: Trích xuất dữ liệu từ bảng nhập liệu và chuyển đổi thành danh sách các cấu trúc `Process`.

  * `void on_btnRun_clicked()`: Hàm phản hồi khi người dùng bấm nút chạy lập lịch. Thực hiện kiểm tra dữ liệu, gọi module `Core` tính toán và phân phối kết quả hiển thị.

  * `void displayResults(const ScheduleResult &result)`: Đưa số liệu $CT, TAT, WT, RT$ và các giá trị trung bình ($\overline{TAT}, \overline{WT}, \overline{RT}$) lên các widget tương ứng.

#### 2. `src/ui/GanttChart.h` & `src/ui/GanttChart.cpp`

* **Lớp `GanttChart` (Thừa kế từ `QWidget`):** Thành phần tùy chỉnh (Custom Widget) phục vụ việc vẽ đồ họa trực quan hóa biểu đồ Gantt.

* **Các hàm quan trọng:**

  * `void setGanttData(const std::vector<GanttSegment> &segments)`: Nhận danh sách các phân đoạn Gantt từ `MainWindow` và gọi `update()` để kích hoạt vẽ lại widget.

  * `protected: void paintEvent(QPaintEvent *event) override`: Hàm ghi đè xử lý sự kiện vẽ đồ họa. Sử dụng `QPainter` để tính toán tỉ lệ tọa độ x, y, vẽ các khối hình chữ nhật đại diện cho tiến trình, hiển thị ID tiến trình và đánh dấu các mốc thời gian thực thi $t_{start}, t_{end}$.
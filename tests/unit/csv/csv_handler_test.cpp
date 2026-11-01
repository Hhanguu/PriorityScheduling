#include "doctest.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/Process.h"
#include "core/ScheduleResult.h"
#include "io/CsvHandler.h"

namespace {

namespace fs = std::filesystem;

std::string dataPath(const std::string& relative) {
    return std::string(PS_TEST_DATA_DIR) + "/" + relative;
}

// Test không ghi vào output/, chỉ ghi vào thư mục tạm của hệ điều hành.
std::string tempPath(const std::string& name) {
    return (fs::temp_directory_path() / ("ps_csv_test_" + name)).string();
}

std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

// File chuẩn trong repo là LF, nhưng Git trên Windows có thể đổi sang CRLF khi checkout.
// Chuẩn hóa về LF để so sánh với file chương trình ghi ra (luôn dùng LF).
std::string slurpExpected(const std::string& path) {
    std::string text = slurp(path);
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        if (c != '\r') {
            out += c;
        }
    }
    return out;
}

void writeText(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << content;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// Xóa file tạm khi ra khỏi phạm vi, kể cả khi CHECK thất bại.
struct TempFile {
    std::string path;
    explicit TempFile(const std::string& name) : path(tempPath(name)) {}
    ~TempFile() {
        std::error_code ec;
        fs::remove(path, ec);
    }
};

// Đếm số lần một đoạn chữ xuất hiện (dùng để đếm số dòng "Dòng N: ..." trong thông báo lỗi).
int countOccurrences(const std::string& text, const std::string& part) {
    int count = 0;
    for (std::size_t pos = text.find(part); pos != std::string::npos; pos = text.find(part, pos + part.size())) {
        ++count;
    }
    return count;
}

// Số dòng (ngăn bằng '\n') của một đoạn chữ không rỗng.
int countLines(const std::string& text) {
    return text.empty() ? 0 : countOccurrences(text, "\n") + 1;
}

// Ghi content vào file tạm, đọc thử bằng readProcesses và trả kết quả để test kiểm tra.
struct ReadOutcome {
    bool ok = false;
    std::vector<Process> out;
    std::string error;
};

ReadOutcome readText(const std::string& name, const std::string& content) {
    TempFile file(name);
    writeText(file.path, content);
    ReadOutcome r;
    r.ok = CsvHandler::readProcesses(file.path, r.out, r.error);
    return r;
}

std::vector<Process> readOk(const std::string& relative) {
    std::vector<Process> out;
    std::string error = "chưa gọi";
    REQUIRE(CsvHandler::readProcesses(dataPath(relative), out, error));
    CHECK(error.empty());
    return out;
}

std::string readFail(const std::string& relative) {
    std::vector<Process> out;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(dataPath(relative), out, error));
    CHECK(out.empty());
    CHECK_FALSE(error.empty());
    return error;
}

// Kết quả chuẩn của samples/sample.csv (docs/api.md, mục 8).
ScheduleResult sampleNp() {
    ScheduleResult r;
    r.processes = {{1, 5, 5, 0, 0},    {2, 17, 16, 13, 13}, {3, 13, 11, 3, 3},
                   {4, 19, 15, 13, 13}, {5, 30, 24, 20, 20}, {6, 14, 5, 4, 4},
                   {7, 26, 14, 11, 11}, {8, 21, 6, 4, 4},    {9, 35, 17, 12, 12},
                   {10, 23, 2, 0, 0}};
    r.gantt = {{1, 0, 5},   {3, 5, 13},  {6, 13, 14}, {2, 14, 17}, {4, 17, 19},
               {8, 19, 21}, {10, 21, 23}, {7, 23, 26}, {5, 26, 30}, {9, 30, 35}};
    r.averageWaitingTime = 8.00;
    r.averageTurnaroundTime = 11.50;
    r.averageResponseTime = 8.00;
    return r;
}

ScheduleResult sampleP() {
    ScheduleResult r;
    r.processes = {{1, 21, 21, 16, 0},  {2, 13, 12, 9, 0},  {3, 10, 8, 0, 0},
                   {4, 15, 11, 9, 9},   {5, 30, 24, 20, 20}, {6, 11, 2, 1, 1},
                   {7, 26, 14, 11, 11}, {8, 17, 2, 0, 0},   {9, 35, 17, 12, 12},
                   {10, 23, 2, 0, 0}};
    r.gantt = {{1, 0, 1},   {2, 1, 2},   {3, 2, 10},  {6, 10, 11}, {2, 11, 13}, {4, 13, 15},
               {8, 15, 17}, {1, 17, 21}, {10, 21, 23}, {7, 23, 26}, {5, 26, 30}, {9, 30, 35}};
    r.averageWaitingTime = 7.80;
    r.averageTurnaroundTime = 11.30;
    r.averageResponseTime = 5.30;
    return r;
}

// Đọc lại file kết quả chuẩn thành ScheduleResult (dùng cho sample_20).
std::vector<std::vector<long long>> parseRows(const std::string& text) {
    std::vector<std::vector<long long>> rows;
    std::istringstream in(text);
    std::string line;
    std::getline(in, line); // bỏ header
    while (std::getline(in, line)) {
        std::vector<long long> row;
        std::istringstream cells(line);
        std::string cell;
        while (std::getline(cells, cell, ',')) {
            row.push_back(std::stoll(cell));
        }
        rows.push_back(row);
    }
    return rows;
}

ScheduleResult loadExpected(const std::string& prefix, double wt, double tat, double rt) {
    ScheduleResult r;
    for (const auto& row : parseRows(slurpExpected(dataPath("ket_qua/csv/" + prefix + "_result.csv")))) {
        r.processes.push_back({static_cast<int>(row[0]), row[1], row[2], row[3], row[4]});
    }
    for (const auto& row : parseRows(slurpExpected(dataPath("ket_qua/csv/" + prefix + "_gantt.csv")))) {
        r.gantt.push_back({static_cast<int>(row[0]), row[1], row[2]});
    }
    r.averageWaitingTime = wt;
    r.averageTurnaroundTime = tat;
    r.averageResponseTime = rt;
    return r;
}

} // namespace

// ---------------------------------------------------------------------------
// 1. Đọc hợp lệ (C01–C06)
// ---------------------------------------------------------------------------

TEST_CASE("C01 doc sample.csv dung 10 process theo thu tu pid") {
    const std::vector<Process> p = readOk("csv/sample.csv");
    REQUIRE(p.size() == 10);
    for (std::size_t i = 0; i < p.size(); ++i) {
        CHECK(p[i].pid == static_cast<int>(i) + 1);
    }
    CHECK(p[0].arrivalTime == 0);
    CHECK(p[0].burstTime == 5);
    CHECK(p[0].priority == 3);
    CHECK(p[1].arrivalTime == 1);
    CHECK(p[1].burstTime == 3);
    CHECK(p[1].priority == 2);
    CHECK(p[2].arrivalTime == 2);
    CHECK(p[2].burstTime == 8);
    CHECK(p[2].priority == 1);
}

TEST_CASE("C02 file CRLF cho ket qua giong C01") {
    const std::vector<Process> lf = readOk("csv/sample.csv");
    const std::vector<Process> crlf = readOk("csv/crlf_sample.csv");
    REQUIRE(crlf.size() == lf.size());
    for (std::size_t i = 0; i < lf.size(); ++i) {
        CHECK(crlf[i].pid == lf[i].pid);
        CHECK(crlf[i].arrivalTime == lf[i].arrivalTime);
        CHECK(crlf[i].burstTime == lf[i].burstTime);
        CHECK(crlf[i].priority == lf[i].priority);
    }
}

TEST_CASE("C03 dong trong va khoang trang thua bi bo qua") {
    TempFile file("blank_spaces.csv");
    writeText(file.path,
              "pid,arrival,burst,priority\n"
              "\n"
              "  1 , 0 ,5, 3 \n"
              "\n"
              "\t\n"
              "2,1,3,2\n"
              "3,2,8,1\n"
              "\n");
    std::vector<Process> p;
    std::string error;
    REQUIRE(CsvHandler::readProcesses(file.path, p, error));
    REQUIRE(p.size() == 3);
    CHECK(p[0].pid == 1);
    CHECK(p[0].arrivalTime == 0);
    CHECK(p[0].burstTime == 5);
    CHECK(p[0].priority == 3);
    CHECK(p[2].pid == 3);
}

TEST_CASE("C04 khong sap xep lai theo arrival") {
    const std::vector<Process> p = readOk("csv/unsorted_arrival.csv");
    REQUIRE(p.size() == 10);
    const int expectedArrival[10] = {10, 0, 4, 2, 8, 6, 12, 15, 18, 20};
    for (int i = 0; i < 10; ++i) {
        CHECK(p[i].pid == i + 1);
        CHECK(p[i].arrivalTime == expectedArrival[i]);
    }
}

TEST_CASE("C05 doc sample_20.csv du 20 process giu thu tu") {
    const std::vector<Process> p = readOk("csv/sample_20.csv");
    REQUIRE(p.size() == 20);
    for (std::size_t i = 0; i < p.size(); ++i) {
        CHECK(p[i].pid == static_cast<int>(i) + 1);
    }
}

TEST_CASE("C06 doc thanh cong thi error rong") {
    std::vector<Process> p;
    std::string error = "loi cu";
    REQUIRE(CsvHandler::readProcesses(dataPath("csv/sample.csv"), p, error));
    CHECK(error.empty());
}

TEST_CASE("C24 doc large_1000.csv du 1000 process giu thu tu") {
    const std::vector<Process> p = readOk("csv/large_1000.csv");
    REQUIRE(p.size() == 1000);
    for (std::size_t i = 0; i < p.size(); ++i) {
        CHECK(p[i].pid == static_cast<int>(i) + 1);
    }
    CHECK(p.front().arrivalTime == 0);
    CHECK(p.front().burstTime == 4);
    CHECK(p.front().priority == 9);
    CHECK(p.back().arrivalTime == 1500);
    CHECK(p.back().burstTime == 9);
    CHECK(p.back().priority == 6);
}

TEST_CASE("file co BOM UTF-8 van doc duoc") {
    TempFile file("bom.csv");
    writeText(file.path, "\xEF\xBB\xBFpid,arrival,burst,priority\n1,0,5,3\n");
    std::vector<Process> p;
    std::string error;
    REQUIRE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(p.size() == 1);
}

// ---------------------------------------------------------------------------
// 2. Đọc lỗi (C07–C15)
// ---------------------------------------------------------------------------

TEST_CASE("C07 sai header") {
    const std::string e = readFail("csv/err_bad_header.csv");
    CHECK(contains(e, "Dòng 1"));
    CHECK(contains(e, "header"));
}

TEST_CASE("C08 thieu cot") {
    const std::string e = readFail("csv/err_missing_column.csv");
    CHECK(contains(e, "Dòng 2"));
    CHECK(contains(e, "4 cột"));
}

TEST_CASE("C09 khong phai so") {
    const std::string e = readFail("csv/err_not_number.csv");
    CHECK(contains(e, "Dòng 2"));
    CHECK(contains(e, "arrival"));
}

TEST_CASE("C10 arrival am") {
    const std::string e = readFail("csv/err_negative_arrival.csv");
    CHECK(contains(e, "Dòng 2"));
    CHECK(contains(e, "arrival"));
}

TEST_CASE("C11 burst bang 0") {
    const std::string e = readFail("csv/err_zero_burst.csv");
    CHECK(contains(e, "Dòng 2"));
    CHECK(contains(e, "burst"));
}

TEST_CASE("C12 pid trung") {
    const std::string e = readFail("csv/err_duplicate_pid.csv");
    CHECK(contains(e, "Dòng 3"));
    CHECK(contains(e, "trùng"));
}

TEST_CASE("C13 chi co header") {
    const std::string e = readFail("csv/err_header_only.csv");
    CHECK(contains(e, "không có tiến trình"));
}

TEST_CASE("C14 duong dan khong ton tai") {
    std::vector<Process> p;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(dataPath("csv/khong_ton_tai.csv"), p, error));
    CHECK(contains(error, "Không mở được file"));
}

TEST_CASE("C15 doc loi thi out rong du truoc do co du lieu") {
    std::vector<Process> p = {{99, 0, 1, 1}};
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(dataPath("csv/err_zero_burst.csv"), p, error));
    CHECK(p.empty());
}

TEST_CASE("file rong hoan toan bao loi header") {
    TempFile file("empty.csv");
    writeText(file.path, "");
    std::vector<Process> p;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(contains(error, "Dòng 1"));
}

TEST_CASE("pid bang 0 hoac am bi tu choi") {
    TempFile file("pid_zero.csv");
    writeText(file.path, "pid,arrival,burst,priority\n0,0,5,3\n");
    std::vector<Process> p;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(contains(error, "Dòng 2"));
    CHECK(contains(error, "pid"));
}

// Test biên: mỗi kiểu lỗi một case riêng để case nào đỏ thì biết ngay sai ở loại nào.

TEST_CASE("bien: thua cot bi tu choi") {
    const ReadOutcome r = readText("extra_col.csv", "pid,arrival,burst,priority\n1,0,5,3,7\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "cần 4 cột, có 5"));
}

TEST_CASE("bien: thieu cot bi tu choi") {
    const ReadOutcome r = readText("missing_col.csv", "pid,arrival,burst,priority\n1,0,5\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "cần 4 cột, có 3"));
}

TEST_CASE("bien: so thuc bi tu choi") {
    const ReadOutcome r = readText("real_number.csv", "pid,arrival,burst,priority\n1,0,5.5,3\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "burst"));
    CHECK(contains(r.error, "không phải số nguyên"));
}

TEST_CASE("bien: pid trung bi tu choi") {
    const ReadOutcome r = readText("dup_pid.csv", "pid,arrival,burst,priority\n7,0,5,3\n7,1,2,1\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 3"));
    CHECK(contains(r.error, "pid 7 bị trùng"));
}

TEST_CASE("bien: burst am bi tu choi") {
    const ReadOutcome r = readText("neg_burst.csv", "pid,arrival,burst,priority\n1,0,-4,3\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "burst"));
}

TEST_CASE("bien: burst bang 0 bi tu choi") {
    const ReadOutcome r = readText("zero_burst.csv", "pid,arrival,burst,priority\n1,0,0,3\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "burst"));
}

TEST_CASE("bien: arrival am bi tu choi") {
    const ReadOutcome r = readText("neg_arrival.csv", "pid,arrival,burst,priority\n1,-1,5,3\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 2"));
    CHECK(contains(r.error, "arrival"));
}

TEST_CASE("bien: arrival bang 0 la hop le") {
    const ReadOutcome r = readText("zero_arrival.csv", "pid,arrival,burst,priority\n1,0,5,3\n");
    CHECK(r.ok);
    REQUIRE(r.out.size() == 1);
    CHECK(r.out[0].arrivalTime == 0);
}

TEST_CASE("bien: dong trong xen giua khong gay loi va van dem dung so dong") {
    const ReadOutcome ok = readText("blank_mid.csv", "pid,arrival,burst,priority\n\n1,0,5,3\n\n\n2,1,3,2\n");
    CHECK(ok.ok);
    CHECK(ok.out.size() == 2);

    // Dòng trống vẫn được đếm: dòng sai nằm ở dòng vật lý số 5.
    const ReadOutcome bad = readText("blank_then_bad.csv", "pid,arrival,burst,priority\n\n1,0,5,3\n\n2,1,0,2\n");
    CHECK_FALSE(bad.ok);
    CHECK(contains(bad.error, "Dòng 5"));
}

TEST_CASE("bien: chi co dong trong sau header bao khong co tien trinh") {
    const ReadOutcome r = readText("only_blank.csv", "pid,arrival,burst,priority\n\n   \n\t\n");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "không có tiến trình"));
}

TEST_CASE("bien: file rong bao loi header") {
    const ReadOutcome r = readText("empty_file.csv", "");
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());
    CHECK(contains(r.error, "Dòng 1"));
    CHECK(contains(r.error, "header"));
}

TEST_CASE("so qua lon khong lam crash") {
    TempFile file("overflow.csv");
    writeText(file.path, "pid,arrival,burst,priority\n1,0,99999999999999999999999,3\n");
    std::vector<Process> p;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(contains(error, "Dòng 2"));
}

TEST_CASE("arrival hoac burst vuot gioi han tren bi tu choi") {
    TempFile file("gioi_han.csv");
    std::vector<Process> p;
    std::string error;

    writeText(file.path, "pid,arrival,burst,priority\n1,1000000000001,5,3\n");
    CHECK_FALSE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(contains(error, "Dòng 2"));
    CHECK(contains(error, "arrival"));

    writeText(file.path, "pid,arrival,burst,priority\n1,0,1000000000001,3\n");
    CHECK_FALSE(CsvHandler::readProcesses(file.path, p, error));
    CHECK(contains(error, "burst"));

    writeText(file.path, "pid,arrival,burst,priority\n1,1000000000000,1000000000000,3\n");
    CHECK(CsvHandler::readProcesses(file.path, p, error));
}

TEST_CASE("thong bao loi la UTF-8 dung byte") {
    const std::string e = readFail("csv/err_zero_burst.csv");
    // "Dòng" ở dạng UTF-8: D, o, dấu huyền (C3 B2), n, g
    CHECK(e.compare(0, 5, "D\xC3\xB2ng") == 0);
}

// ---------------------------------------------------------------------------
// 2b. Gom mọi lỗi theo dòng (không dừng ở lỗi đầu tiên)
// ---------------------------------------------------------------------------

TEST_CASE("gom loi: moi dong sai mot dong thong bao, dong hop le khong bi bao") {
    const ReadOutcome r = readText("multi_errors.csv",
                                   "pid,arrival,burst,priority\n"
                                   "1,0,5,3\n"    // dòng 2: hợp lệ
                                   "2,1,0,2\n"    // dòng 3: burst = 0
                                   "3,2,4\n"      // dòng 4: thiếu cột
                                   "4,x,3,1\n"    // dòng 5: arrival không phải số
                                   "5,3,2,2\n"    // dòng 6: hợp lệ
                                   "1,4,2,1\n"    // dòng 7: pid 1 trùng với dòng 2
                                   "6,-1,2,2\n"); // dòng 8: arrival âm
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());

    CHECK(contains(r.error, "Dòng 3: burst"));
    CHECK(contains(r.error, "Dòng 4: cần 4 cột, có 3"));
    CHECK(contains(r.error, "Dòng 5: arrival không phải số nguyên"));
    CHECK(contains(r.error, "Dòng 7: pid 1 bị trùng"));
    CHECK(contains(r.error, "Dòng 8: arrival"));

    CHECK_FALSE(contains(r.error, "Dòng 2"));
    CHECK_FALSE(contains(r.error, "Dòng 6"));
    CHECK(countOccurrences(r.error, "Dòng ") == 5);
    CHECK(countLines(r.error) == 5); // mỗi lỗi nằm trên một dòng riêng
}

TEST_CASE("gom loi: loi duoc liet ke theo dung thu tu dong trong file") {
    const ReadOutcome r = readText("order_errors.csv",
                                   "pid,arrival,burst,priority\n"
                                   "1,0,0,3\n"
                                   "2,0,0,3\n"
                                   "3,0,0,3\n");
    CHECK_FALSE(r.ok);
    const std::size_t a = r.error.find("Dòng 2");
    const std::size_t b = r.error.find("Dòng 3");
    const std::size_t c = r.error.find("Dòng 4");
    REQUIRE(a != std::string::npos);
    REQUIRE(b != std::string::npos);
    REQUIRE(c != std::string::npos);
    CHECK(a < b);
    CHECK(b < c);
}

TEST_CASE("gom loi: mot dong sai nhieu cho chi bao loi dau tien cua dong do") {
    // arrival không phải số VÀ burst = 0: chỉ báo một lỗi cho dòng 2 (lỗi gặp trước).
    const ReadOutcome r = readText("one_per_line.csv", "pid,arrival,burst,priority\n1,x,0,3\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 2: arrival"));
    CHECK_FALSE(contains(r.error, "burst"));
    CHECK(countOccurrences(r.error, "Dòng ") == 1);
}

TEST_CASE("gom loi: pid cua dong sai khong bi tinh la da dung") {
    // Dòng 2 sai (burst = 0) nên pid 1 chưa được "dùng"; dòng 3 dùng pid 1 là hợp lệ, không bị báo trùng.
    const ReadOutcome r = readText("pid_of_bad_row.csv", "pid,arrival,burst,priority\n1,0,0,3\n1,1,5,2\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 2: burst"));
    CHECK_FALSE(contains(r.error, "Dòng 3"));
    CHECK_FALSE(contains(r.error, "trùng"));
}

TEST_CASE("gom loi: dong trong khong tinh la loi nhung van dem so dong") {
    const ReadOutcome r = readText("blank_between_errors.csv",
                                   "pid,arrival,burst,priority\n"
                                   "\n"
                                   "2,1,0,2\n"
                                   "\n"
                                   "3,2,4\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 3"));
    CHECK(contains(r.error, "Dòng 5"));
    CHECK(countOccurrences(r.error, "Dòng ") == 2);
}

TEST_CASE("gom loi: sai header thi chi bao header va dung") {
    const ReadOutcome r = readText("bad_header_and_rows.csv", "pid,at,bt,prio\n1,0,0,3\n2,1,5\n");
    CHECK_FALSE(r.ok);
    CHECK(contains(r.error, "Dòng 1"));
    CHECK(contains(r.error, "header"));
    CHECK(countOccurrences(r.error, "Dòng ") == 1);
}

TEST_CASE("gom loi: file hop le thi error rong") {
    const ReadOutcome r = readText("all_good.csv", "pid,arrival,burst,priority\n1,0,5,3\n2,1,3,2\n");
    CHECK(r.ok);
    CHECK(r.error.empty());
    CHECK(r.out.size() == 2);
}

TEST_CASE("gom loi: chi liet ke 100 dong dau, phan con lai gop thanh mot dong") {
    std::string content = "pid,arrival,burst,priority\n";
    for (int i = 0; i < 150; ++i) {
        content += "1,0,0,3\n"; // 150 dòng đều sai (burst = 0), từ dòng 2 đến dòng 151
    }
    const ReadOutcome r = readText("many_errors.csv", content);
    CHECK_FALSE(r.ok);
    CHECK(r.out.empty());

    CHECK(contains(r.error, "Dòng 2:"));
    CHECK(contains(r.error, "Dòng 101:"));        // lỗi thứ 100
    CHECK_FALSE(contains(r.error, "Dòng 102:"));  // lỗi thứ 101 không được liệt kê
    CHECK(countOccurrences(r.error, "Dòng ") == 100);
    CHECK(contains(r.error, "và 50 dòng lỗi khác"));
    CHECK(countLines(r.error) == 101);            // 100 dòng lỗi + 1 dòng tổng kết
}

// ---------------------------------------------------------------------------
// 3. Ghi (C16–C23, C25)
// ---------------------------------------------------------------------------

TEST_CASE("C16 ghi ket qua NP khop file chuan") {
    TempFile file("np_result.csv");
    std::string error;
    REQUIRE(CsvHandler::writeProcessResults(file.path, sampleNp(), error));
    CHECK(error.empty());
    CHECK(slurp(file.path) == slurpExpected(dataPath("ket_qua/csv/sample_np_result.csv")));
}

TEST_CASE("C17 ghi Gantt NP khop file chuan") {
    TempFile file("np_gantt.csv");
    std::string error;
    REQUIRE(CsvHandler::writeGantt(file.path, sampleNp(), error));
    CHECK(slurp(file.path) == slurpExpected(dataPath("ket_qua/csv/sample_np_gantt.csv")));
}

TEST_CASE("C18 ghi ket qua P khop file chuan") {
    TempFile file("p_result.csv");
    std::string error;
    REQUIRE(CsvHandler::writeProcessResults(file.path, sampleP(), error));
    CHECK(slurp(file.path) == slurpExpected(dataPath("ket_qua/csv/sample_p_result.csv")));
}

TEST_CASE("C19 ghi Gantt P khop file chuan") {
    TempFile file("p_gantt.csv");
    std::string error;
    REQUIRE(CsvHandler::writeGantt(file.path, sampleP(), error));
    CHECK(slurp(file.path) == slurpExpected(dataPath("ket_qua/csv/sample_p_gantt.csv")));
}

TEST_CASE("C20 ghi comparison khop file chuan") {
    TempFile file("comparison.csv");
    std::string error;
    REQUIRE(CsvHandler::writeComparison(file.path, sampleNp(), sampleP(), error));
    CHECK(slurp(file.path) == slurpExpected(dataPath("ket_qua/csv/sample_comparison.csv")));
}

TEST_CASE("C21 IDLE ghi pid = -1, khong ghi chu IDLE") {
    ScheduleResult r;
    r.gantt = {{-1, 0, 3}, {1, 3, 8}, {-1, 8, 10}};
    TempFile file("idle_gantt.csv");
    std::string error;
    REQUIRE(CsvHandler::writeGantt(file.path, r, error));
    const std::string text = slurp(file.path);
    CHECK(text == "pid,start,end\n-1,0,3\n1,3,8\n-1,8,10\n");
    CHECK_FALSE(contains(text, "IDLE"));
}

TEST_CASE("C22 moi file ket thuc bang mot ky tu xuong dong") {
    TempFile a("end_result.csv");
    TempFile b("end_gantt.csv");
    TempFile c("end_comparison.csv");
    std::string error;
    REQUIRE(CsvHandler::writeProcessResults(a.path, sampleNp(), error));
    REQUIRE(CsvHandler::writeGantt(b.path, sampleNp(), error));
    REQUIRE(CsvHandler::writeComparison(c.path, sampleNp(), sampleP(), error));
    for (const std::string& path : {a.path, b.path, c.path}) {
        const std::string text = slurp(path);
        REQUIRE(text.size() >= 2);
        CHECK(text.back() == '\n');
        CHECK(text[text.size() - 2] != '\n'); // không có dòng trống thừa
        CHECK(text.find('\r') == std::string::npos); // luôn là \n, không phải \r\n
    }
}

TEST_CASE("C23 duong dan khong ghi duoc thi tra false va co thong bao") {
    const std::string bad = (fs::temp_directory_path() / "ps_csv_khong_co_thu_muc" / "x.csv").string();
    std::string error;
    CHECK_FALSE(CsvHandler::writeProcessResults(bad, sampleNp(), error));
    CHECK_FALSE(error.empty());
    error.clear();
    CHECK_FALSE(CsvHandler::writeGantt(bad, sampleNp(), error));
    CHECK_FALSE(error.empty());
    error.clear();
    CHECK_FALSE(CsvHandler::writeComparison(bad, sampleNp(), sampleP(), error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("C25 ghi sample_20 khop 5 file chuan") {
    const ScheduleResult np = loadExpected("sample_20_np", 21.50, 25.50, 21.50);
    const ScheduleResult p = loadExpected("sample_20_p", 21.60, 25.60, 18.45);

    TempFile npResult("s20_np_result.csv");
    TempFile npGantt("s20_np_gantt.csv");
    TempFile pResult("s20_p_result.csv");
    TempFile pGantt("s20_p_gantt.csv");
    TempFile comparison("s20_comparison.csv");
    std::string error;
    REQUIRE(CsvHandler::writeProcessResults(npResult.path, np, error));
    REQUIRE(CsvHandler::writeGantt(npGantt.path, np, error));
    REQUIRE(CsvHandler::writeProcessResults(pResult.path, p, error));
    REQUIRE(CsvHandler::writeGantt(pGantt.path, p, error));
    REQUIRE(CsvHandler::writeComparison(comparison.path, np, p, error));

    CHECK(slurp(npResult.path) == slurpExpected(dataPath("ket_qua/csv/sample_20_np_result.csv")));
    CHECK(slurp(npGantt.path) == slurpExpected(dataPath("ket_qua/csv/sample_20_np_gantt.csv")));
    CHECK(slurp(pResult.path) == slurpExpected(dataPath("ket_qua/csv/sample_20_p_result.csv")));
    CHECK(slurp(pGantt.path) == slurpExpected(dataPath("ket_qua/csv/sample_20_p_gantt.csv")));
    CHECK(slurp(comparison.path) == slurpExpected(dataPath("ket_qua/csv/sample_20_comparison.csv")));
}

TEST_CASE("ghi comparison lam tron 2 chu so va dung dau cham") {
    ScheduleResult np;
    np.averageWaitingTime = 5.666666;
    np.averageTurnaroundTime = 10.0;
    np.averageResponseTime = 0.004;
    ScheduleResult p;
    p.averageWaitingTime = 3.0;
    p.averageTurnaroundTime = 7.665;
    p.averageResponseTime = 1.0;
    TempFile file("rounding.csv");
    std::string error;
    REQUIRE(CsvHandler::writeComparison(file.path, np, p, error));
    const std::string text = slurp(file.path);
    CHECK(contains(text, "avg_WT,5.67,3.00\n"));
    CHECK(contains(text, "avg_TAT,10.00,"));
    CHECK(contains(text, "avg_RT,0.00,1.00\n"));
    CHECK(text.find(';') == std::string::npos);
}

TEST_CASE("ghi ket qua rong chi co header") {
    TempFile result("empty_result.csv");
    TempFile gantt("empty_gantt.csv");
    std::string error;
    REQUIRE(CsvHandler::writeProcessResults(result.path, ScheduleResult{}, error));
    REQUIRE(CsvHandler::writeGantt(gantt.path, ScheduleResult{}, error));
    CHECK(slurp(result.path) == "pid,CT,TAT,WT,RT\n");
    CHECK(slurp(gantt.path) == "pid,start,end\n");
}

TEST_CASE("ghi roi doc lai dung gia tri") {
    // Quy trình CSV điển hình: đọc input -> ghi lại -> đọc lại bằng cùng hàm đọc.
    const std::vector<Process> original = readOk("csv/sample.csv");
    TempFile file("roundtrip.csv");
    std::string text = "pid,arrival,burst,priority\n";
    for (const Process& p : original) {
        text += std::to_string(p.pid) + "," + std::to_string(p.arrivalTime) + "," +
                std::to_string(p.burstTime) + "," + std::to_string(p.priority) + "\n";
    }
    writeText(file.path, text);
    std::vector<Process> again;
    std::string error;
    REQUIRE(CsvHandler::readProcesses(file.path, again, error));
    REQUIRE(again.size() == original.size());
    for (std::size_t i = 0; i < original.size(); ++i) {
        CHECK(again[i].pid == original[i].pid);
        CHECK(again[i].burstTime == original[i].burstTime);
    }
}

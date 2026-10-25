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

TEST_CASE("thua cot va so thuc deu bi tu choi") {
    TempFile extra("extra_col.csv");
    writeText(extra.path, "pid,arrival,burst,priority\n1,0,5,3,7\n");
    std::vector<Process> p;
    std::string error;
    CHECK_FALSE(CsvHandler::readProcesses(extra.path, p, error));
    CHECK(contains(error, "Dòng 2"));
    CHECK(contains(error, "4 cột"));

    TempFile real("real_number.csv");
    writeText(real.path, "pid,arrival,burst,priority\n1,0,5.5,3\n");
    CHECK_FALSE(CsvHandler::readProcesses(real.path, p, error));
    CHECK(contains(error, "Dòng 2"));
    CHECK(contains(error, "burst"));
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

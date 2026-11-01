#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "core/Metrics.h"
#include "core/NonPreemptiveScheduler.h"
#include "core/PreemptiveScheduler.h"
#include "core/Process.h"
#include "core/ScheduleResult.h"
#include "io/CsvHandler.h"

// Test đầu-cuối: chạy cả dây chuyền như người dùng thật.
//   đọc CSV -> NP và P -> Metrics -> ghi 5 file -> đọc lại 5 file -> so TỪNG DÒNG với file kỳ vọng.
// Không dựng sẵn kết quả giả: hai scheduler chạy thật rồi mới đem ghi.

namespace {

namespace fs = std::filesystem;

std::string dataPath(const std::string& relative) {
    return std::string(PS_TEST_DATA_DIR) + "/" + relative;
}

// Thư mục tạm riêng cho mỗi test, tự xóa khi xong (kể cả khi test fail).
// Test không ghi vào output/ của người dùng.
struct TempDir {
    fs::path dir;
    explicit TempDir(const std::string& name) : dir(fs::temp_directory_path() / ("ps_e2e_" + name)) {
        std::error_code ec;
        fs::remove_all(dir, ec);
        fs::create_directories(dir);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(dir, ec);
    }
    std::string file(const std::string& name) const { return (dir / name).string(); }
};

// Đọc file thành danh sách dòng. stripCr = true cho file kỳ vọng: Git trên Windows có thể đổi LF
// thành CRLF khi checkout, còn file chương trình ghi ra thì giữ nguyên để lộ nếu có '\r' lạc vào.
std::vector<std::string> readLines(const std::string& path, bool stripCr) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream all;
    all << in.rdbuf();
    const std::string text = all.str();

    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\n') {
            lines.push_back(current);
            current.clear();
        } else if (!(stripCr && c == '\r')) {
            current += c;
        }
    }
    if (!current.empty()) {
        lines.push_back(current); // dòng cuối không có '\n'
    }
    return lines;
}

// So hai file từng dòng; nếu lệch thì thông báo nói rõ file nào, dòng nào, nội dung thực tế và kỳ vọng.
void expectSameLines(const std::string& actualPath, const std::string& expectedPath, const std::string& label) {
    REQUIRE_MESSAGE(fs::exists(actualPath), label << ": chương trình không tạo ra file");
    REQUIRE_MESSAGE(fs::exists(expectedPath), label << ": thiếu file kỳ vọng " << expectedPath);

    const std::vector<std::string> actual = readLines(actualPath, false);
    const std::vector<std::string> expected = readLines(expectedPath, true);

    CHECK_MESSAGE(actual.size() == expected.size(),
                  label << ": số dòng thực tế " << actual.size() << " khác kỳ vọng " << expected.size());

    const std::size_t common = std::min(actual.size(), expected.size());
    for (std::size_t i = 0; i < common; ++i) {
        CHECK_MESSAGE(actual[i] == expected[i],
                      label << " lệch ở dòng " << (i + 1) << ": thực tế \"" << actual[i]
                            << "\" nhưng kỳ vọng \"" << expected[i] << "\"");
    }
}

// Kết quả của cả dây chuyền cho một file input.
struct Pipeline {
    std::vector<Process> processes;
    ScheduleResult np;
    ScheduleResult p;
};

// Đọc input, chạy cả hai scheduler, tính trung bình rồi ghi 5 file vào dir.
Pipeline runPipeline(const std::string& inputRelative, const TempDir& dir) {
    Pipeline run;
    std::string error;

    REQUIRE_MESSAGE(CsvHandler::readProcesses(dataPath(inputRelative), run.processes, error),
                    "đọc input thất bại: " << error);
    REQUIRE(error.empty());
    REQUIRE_FALSE(run.processes.empty());

    run.np = NonPreemptiveScheduler::schedule(run.processes);
    run.p = PreemptiveScheduler::schedule(run.processes);
    Metrics::fillAverages(run.np);
    Metrics::fillAverages(run.p);

    REQUIRE_MESSAGE(CsvHandler::writeProcessResults(dir.file("np_result.csv"), run.np, error), error);
    REQUIRE_MESSAGE(CsvHandler::writeGantt(dir.file("np_gantt.csv"), run.np, error), error);
    REQUIRE_MESSAGE(CsvHandler::writeProcessResults(dir.file("p_result.csv"), run.p, error), error);
    REQUIRE_MESSAGE(CsvHandler::writeGantt(dir.file("p_gantt.csv"), run.p, error), error);
    REQUIRE_MESSAGE(CsvHandler::writeComparison(dir.file("comparison.csv"), run.np, run.p, error), error);
    return run;
}

// Tên file ghi ra và file kỳ vọng tương ứng của một bộ dữ liệu (prefix: "sample" hoặc "sample_20").
struct Expected {
    const char* written;
    const char* suffix;
};
const Expected kFiles[] = {
    {"np_result.csv", "_np_result.csv"}, {"np_gantt.csv", "_np_gantt.csv"},
    {"p_result.csv", "_p_result.csv"},   {"p_gantt.csv", "_p_gantt.csv"},
    {"comparison.csv", "_comparison.csv"},
};

} // namespace

// ---------------------------------------------------------------------------
// sample.csv (10 tiến trình): mỗi file ghi ra một case riêng
// ---------------------------------------------------------------------------

TEST_CASE("e2e sample: np_result.csv khop file ky vong tung dong") {
    TempDir dir("sample_np_result");
    runPipeline("leader/sample.csv", dir);
    expectSameLines(dir.file("np_result.csv"), dataPath("ket_qua/leader/sample_np_result.csv"), "np_result.csv");
}

TEST_CASE("e2e sample: np_gantt.csv khop file ky vong tung dong") {
    TempDir dir("sample_np_gantt");
    runPipeline("leader/sample.csv", dir);
    expectSameLines(dir.file("np_gantt.csv"), dataPath("ket_qua/leader/sample_np_gantt.csv"), "np_gantt.csv");
}

TEST_CASE("e2e sample: p_result.csv khop file ky vong tung dong") {
    TempDir dir("sample_p_result");
    runPipeline("leader/sample.csv", dir);
    expectSameLines(dir.file("p_result.csv"), dataPath("ket_qua/leader/sample_p_result.csv"), "p_result.csv");
}

TEST_CASE("e2e sample: p_gantt.csv khop file ky vong tung dong") {
    TempDir dir("sample_p_gantt");
    runPipeline("leader/sample.csv", dir);
    expectSameLines(dir.file("p_gantt.csv"), dataPath("ket_qua/leader/sample_p_gantt.csv"), "p_gantt.csv");
}

TEST_CASE("e2e sample: comparison.csv khop file ky vong tung dong") {
    TempDir dir("sample_comparison");
    runPipeline("leader/sample.csv", dir);
    expectSameLines(dir.file("comparison.csv"), dataPath("ket_qua/leader/sample_comparison.csv"), "comparison.csv");
}

// ---------------------------------------------------------------------------
// sample_20.csv (20 tiến trình): đủ 5 file trong một case
// ---------------------------------------------------------------------------

TEST_CASE("e2e sample_20: du 5 file deu khop file ky vong tung dong") {
    TempDir dir("sample20");
    runPipeline("leader/sample_20.csv", dir);
    for (const Expected& f : kFiles) {
        INFO("file: " << std::string(f.written));
        expectSameLines(dir.file(f.written),
                        dataPath(std::string("ket_qua/leader/sample_20") + f.suffix),
                        std::string("sample_20/") + f.written);
    }
}

// ---------------------------------------------------------------------------
// Tính chất của cả dây chuyền
// ---------------------------------------------------------------------------

TEST_CASE("e2e: du lieu doc vao giu dung thu tu va ket qua tra ve theo thu tu input") {
    TempDir dir("order");
    const Pipeline run = runPipeline("leader/sample.csv", dir);

    REQUIRE(run.processes.size() == 10);
    REQUIRE(run.np.processes.size() == 10);
    REQUIRE(run.p.processes.size() == 10);
    for (std::size_t i = 0; i < run.processes.size(); ++i) {
        CHECK(run.processes[i].pid == static_cast<int>(i) + 1);
        CHECK(run.np.processes[i].pid == run.processes[i].pid); // bất biến I1: đúng thứ tự input
        CHECK(run.p.processes[i].pid == run.processes[i].pid);
    }
}

TEST_CASE("e2e: ghi hai lan tren cung du lieu cho ket qua giong het nhau") {
    TempDir first("repeat_a");
    TempDir second("repeat_b");
    runPipeline("leader/sample.csv", first);
    runPipeline("leader/sample.csv", second);
    for (const Expected& f : kFiles) {
        INFO("file: " << std::string(f.written));
        CHECK(readLines(first.file(f.written), false) == readLines(second.file(f.written), false));
    }
}

TEST_CASE("e2e: 5 file ghi ra deu ton tai, khong rong, header dung") {
    TempDir dir("headers");
    runPipeline("leader/sample.csv", dir);

    const std::pair<const char*, const char*> headers[] = {
        {"np_result.csv", "pid,CT,TAT,WT,RT"},   {"np_gantt.csv", "pid,start,end"},
        {"p_result.csv", "pid,CT,TAT,WT,RT"},     {"p_gantt.csv", "pid,start,end"},
        {"comparison.csv", "metric,np,p"},
    };
    for (const auto& h : headers) {
        INFO("file: " << std::string(h.first));
        REQUIRE(fs::exists(dir.file(h.first)));
        const std::vector<std::string> lines = readLines(dir.file(h.first), false);
        REQUIRE_FALSE(lines.empty());
        CHECK(lines.front() == h.second);
        CHECK(lines.size() >= 2);
    }
}

TEST_CASE("e2e: file input sai thi dung o buoc doc, danh sach rong, khong chay scheduler") {
    std::vector<Process> processes;
    std::string error;
    REQUIRE_FALSE(CsvHandler::readProcesses(dataPath("csv/err_zero_burst.csv"), processes, error));
    CHECK(processes.empty());
    CHECK_FALSE(error.empty());

    // Nếu có lỡ đưa danh sách rỗng cho scheduler thì cũng không crash và trả kết quả rỗng (bất biến I6).
    const ScheduleResult np = NonPreemptiveScheduler::schedule(processes);
    const ScheduleResult p = PreemptiveScheduler::schedule(processes);
    CHECK(np.processes.empty());
    CHECK(np.gantt.empty());
    CHECK(p.processes.empty());
    CHECK(p.gantt.empty());
}

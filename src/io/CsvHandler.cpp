#include "io/CsvHandler.h"

#include <charconv>
#include <climits>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <unordered_set>

namespace {

const char* const kHeader = "pid,arrival,burst,priority";
const int kColumnCount = 4;

// Cắt khoảng trắng (và ký tự \r của file Windows) ở hai đầu chuỗi.
std::string trim(const std::string& s) {
    const char* spaces = " \t\r\n";
    const std::size_t first = s.find_first_not_of(spaces);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = s.find_last_not_of(spaces);
    return s.substr(first, last - first + 1);
}

// Tách dòng theo dấu phẩy, giữ cả ô rỗng để đếm đúng số cột.
std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    for (char c : line) {
        if (c == ',') {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(trim(current));
    return fields;
}

// Chuyển cả chuỗi thành số nguyên; sai định dạng (chữ, số thực, rỗng) thì trả false.
// Dùng from_chars nên không ném exception.
bool parseInteger(const std::string& text, long long& value) {
    if (text.empty()) {
        return false;
    }
    const char* begin = text.data();
    const char* end = begin + text.size();
    const std::from_chars_result r = std::from_chars(begin, end, value);
    return r.ec == std::errc() && r.ptr == end;
}

std::string lineError(int lineNumber, const std::string& message) {
    return "Dòng " + std::to_string(lineNumber) + ": " + message;
}

// Mở file ghi ở chế độ nhị phân để cuối dòng luôn là '\n' (Windows không đổi thành \r\n).
bool openForWrite(const std::string& path, std::ofstream& file, std::string& error) {
    file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        error = "Không ghi được file: " + path;
        return false;
    }
    return true;
}

bool finishWrite(const std::string& path, std::ofstream& file, std::string& error) {
    file.flush();
    if (!file) {
        error = "Ghi file thất bại: " + path;
        return false;
    }
    return true;
}

// Số thực làm tròn 2 chữ số thập phân, luôn dùng dấu chấm.
std::string formatTwoDecimals(double value) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

} // namespace

bool CsvHandler::readProcesses(const std::string& path,
                               std::vector<Process>& out,
                               std::string& error) {
    out.clear();
    error.clear();

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        error = "Không mở được file: " + path;
        return false;
    }

    std::string line;
    int lineNumber = 0;

    // Dòng 1 phải là header đúng chuẩn.
    if (!std::getline(file, line)) {
        error = lineError(1, std::string("header phải là ") + kHeader);
        return false;
    }
    lineNumber = 1;
    if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
        line.erase(0, 3); // bỏ BOM UTF-8 nếu file được lưu từ Excel/Notepad
    }
    if (splitLine(line) != splitLine(kHeader)) {
        error = lineError(lineNumber, std::string("header phải là ") + kHeader);
        return false;
    }

    std::unordered_set<int> seenPids;
    std::vector<Process> result;

    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue; // bỏ qua dòng trống
        }

        const std::vector<std::string> fields = splitLine(line);
        if (static_cast<int>(fields.size()) != kColumnCount) {
            error = lineError(lineNumber, "cần 4 cột, có " + std::to_string(fields.size()));
            return false;
        }

        static const char* const names[kColumnCount] = {"pid", "arrival", "burst", "priority"};
        long long values[kColumnCount] = {0, 0, 0, 0};
        for (int i = 0; i < kColumnCount; ++i) {
            if (!parseInteger(fields[i], values[i])) {
                error = lineError(lineNumber, std::string(names[i]) + " không phải số nguyên");
                return false;
            }
        }

        const long long pid = values[0];
        const long long arrival = values[1];
        const long long burst = values[2];
        const long long priority = values[3];

        if (pid > INT_MAX || priority > INT_MAX || priority < INT_MIN) {
            error = lineError(lineNumber, "giá trị vượt quá phạm vi cho phép");
            return false;
        }
        if (pid <= 0) {
            error = lineError(lineNumber, "pid phải > 0");
            return false;
        }
        if (arrival < 0) {
            error = lineError(lineNumber, "arrival phải >= 0");
            return false;
        }
        if (burst <= 0) {
            error = lineError(lineNumber, "burst phải > 0");
            return false;
        }
        if (!seenPids.insert(static_cast<int>(pid)).second) {
            error = lineError(lineNumber, "pid " + std::to_string(pid) + " bị trùng");
            return false;
        }

        // Thêm vào cuối nên thứ tự dòng trong file được giữ nguyên.
        result.push_back(Process{static_cast<int>(pid), arrival, burst, static_cast<int>(priority)});
    }

    if (result.empty()) {
        error = "File không có tiến trình nào";
        return false;
    }

    out = std::move(result);
    return true;
}

bool CsvHandler::writeProcessResults(const std::string& path,
                                     const ScheduleResult& result,
                                     std::string& error) {
    error.clear();
    std::ofstream file;
    if (!openForWrite(path, file, error)) {
        return false;
    }

    file << "pid,CT,TAT,WT,RT\n";
    for (const ProcessResult& r : result.processes) {
        file << r.pid << ',' << r.completionTime << ',' << r.turnaroundTime << ','
             << r.waitingTime << ',' << r.responseTime << '\n';
    }
    return finishWrite(path, file, error);
}

bool CsvHandler::writeGantt(const std::string& path,
                            const ScheduleResult& result,
                            std::string& error) {
    error.clear();
    std::ofstream file;
    if (!openForWrite(path, file, error)) {
        return false;
    }

    // Scheduler đã gộp đoạn liền kề; ở đây chỉ ghi lại đúng như nhận được (IDLE = -1).
    file << "pid,start,end\n";
    for (const GanttSegment& s : result.gantt) {
        file << s.pid << ',' << s.startTime << ',' << s.endTime << '\n';
    }
    return finishWrite(path, file, error);
}

bool CsvHandler::writeComparison(const std::string& path,
                                 const ScheduleResult& np,
                                 const ScheduleResult& p,
                                 std::string& error) {
    error.clear();
    std::ofstream file;
    if (!openForWrite(path, file, error)) {
        return false;
    }

    // Chỉ định dạng các trung bình Metrics đã điền, không tự tính lại.
    file << "metric,np,p\n";
    file << "avg_WT," << formatTwoDecimals(np.averageWaitingTime) << ','
         << formatTwoDecimals(p.averageWaitingTime) << '\n';
    file << "avg_TAT," << formatTwoDecimals(np.averageTurnaroundTime) << ','
         << formatTwoDecimals(p.averageTurnaroundTime) << '\n';
    file << "avg_RT," << formatTwoDecimals(np.averageResponseTime) << ','
         << formatTwoDecimals(p.averageResponseTime) << '\n';
    return finishWrite(path, file, error);
}

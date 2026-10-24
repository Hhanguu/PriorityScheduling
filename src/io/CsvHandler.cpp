#include "io/CsvHandler.h"

#include <charconv>
#include <climits>
#include <fstream>
#include <iomanip>
#include <locale>
#include <unordered_set>

namespace {

const char* const kHeader = "pid,arrival,burst,priority";
const char* const kFieldNames[] = {"pid", "arrival", "burst", "priority"};
const int kColumnCount = 4;

// Chặn arrival/burst quá lớn để scheduler cộng dồn thời gian không bị tràn long long.
const long long kMaxTime = 1000000000000LL;

// Cắt khoảng trắng và '\r' (file Windows) ở hai đầu chuỗi.
std::string trim(const std::string& s) {
    const char* spaces = " \t\r\n";
    const std::size_t first = s.find_first_not_of(spaces);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = s.find_last_not_of(spaces);
    return s.substr(first, last - first + 1);
}

// Giữ cả ô rỗng để đếm đúng số cột.
std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(trim(line.substr(start)));
            return fields;
        }
        fields.push_back(trim(line.substr(start, comma - start)));
        start = comma + 1;
    }
}

// Chỉ nhận cả chuỗi là số nguyên; "abc", "3.5", "" đều trả false.
bool parseInteger(const std::string& text, long long& value) {
    if (text.empty()) {
        return false;
    }
    const char* end = text.data() + text.size();
    const std::from_chars_result r = std::from_chars(text.data(), end, value);
    return r.ec == std::errc() && r.ptr == end;
}

// Mở file nhị phân (cuối dòng luôn là '\n', kể cả trên Windows), gọi write(file), rồi kiểm tra lỗi.
template <typename WriteFn>
bool writeFile(const std::string& path, std::string& error, WriteFn write) {
    error.clear();
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        error = "Không ghi được file: " + path;
        return false;
    }
    file.imbue(std::locale::classic()); // không để locale máy chèn dấu phân cách hàng nghìn
    write(file);
    file.flush();
    if (!file) {
        error = "Ghi file thất bại: " + path;
        return false;
    }
    return true;
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

    int lineNumber = 1;
    auto fail = [&](const std::string& message) {
        error = "Dòng " + std::to_string(lineNumber) + ": " + message;
        return false;
    };

    std::string line;
    if (!std::getline(file, line)) {
        return fail(std::string("header phải là ") + kHeader);
    }
    if (line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
        line.erase(0, 3); // BOM UTF-8 do Excel/Notepad thêm vào
    }
    static const std::vector<std::string> expectedHeader = splitLine(kHeader);
    if (splitLine(line) != expectedHeader) {
        return fail(std::string("header phải là ") + kHeader);
    }

    std::unordered_set<int> seenPids;
    std::vector<Process> result;

    while (std::getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;
        }

        const std::vector<std::string> fields = splitLine(line);
        if (static_cast<int>(fields.size()) != kColumnCount) {
            return fail("cần " + std::to_string(kColumnCount) + " cột, có " +
                        std::to_string(fields.size()));
        }

        long long values[kColumnCount];
        for (int i = 0; i < kColumnCount; ++i) {
            if (!parseInteger(fields[i], values[i])) {
                return fail(std::string(kFieldNames[i]) + " không phải số nguyên");
            }
        }
        const long long pid = values[0];
        const long long arrival = values[1];
        const long long burst = values[2];
        const long long priority = values[3];

        if (pid <= 0 || pid > INT_MAX) {
            return fail("pid phải trong khoảng 1 đến " + std::to_string(INT_MAX));
        }
        if (arrival < 0 || arrival > kMaxTime) {
            return fail("arrival phải trong khoảng 0 đến " + std::to_string(kMaxTime));
        }
        if (burst <= 0 || burst > kMaxTime) {
            return fail("burst phải trong khoảng 1 đến " + std::to_string(kMaxTime));
        }
        if (priority < INT_MIN || priority > INT_MAX) {
            return fail("priority vượt quá phạm vi số nguyên 32 bit");
        }
        if (!seenPids.insert(static_cast<int>(pid)).second) {
            return fail("pid " + std::to_string(pid) + " bị trùng");
        }

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
    return writeFile(path, error, [&](std::ofstream& file) {
        file << "pid,CT,TAT,WT,RT\n";
        for (const ProcessResult& r : result.processes) {
            file << r.pid << ',' << r.completionTime << ',' << r.turnaroundTime << ','
                 << r.waitingTime << ',' << r.responseTime << '\n';
        }
    });
}

bool CsvHandler::writeGantt(const std::string& path,
                            const ScheduleResult& result,
                            std::string& error) {
    return writeFile(path, error, [&](std::ofstream& file) {
        file << "pid,start,end\n";
        for (const GanttSegment& s : result.gantt) {
            file << s.pid << ',' << s.startTime << ',' << s.endTime << '\n';
        }
    });
}

bool CsvHandler::writeComparison(const std::string& path,
                                 const ScheduleResult& np,
                                 const ScheduleResult& p,
                                 std::string& error) {
    return writeFile(path, error, [&](std::ofstream& file) {
        file << std::fixed << std::setprecision(2);
        file << "metric,np,p\n";
        file << "avg_WT," << np.averageWaitingTime << ',' << p.averageWaitingTime << '\n';
        file << "avg_TAT," << np.averageTurnaroundTime << ',' << p.averageTurnaroundTime << '\n';
        file << "avg_RT," << np.averageResponseTime << ',' << p.averageResponseTime << '\n';
    });
}

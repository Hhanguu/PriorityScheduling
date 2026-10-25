#pragma once
#include <string>
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class CsvHandler {
public:
    // Đọc + validate. Thành công: true, out có dữ liệu theo đúng thứ tự dòng.
    // Thất bại: false, error chứa thông báo (tiếng Việt, kèm số dòng).
    static bool readProcesses(const std::string& path,
                              std::vector<Process>& out,
                              std::string& error);

    // Ghi kết quả từng tiến trình: pid,CT,TAT,WT,RT
    static bool writeProcessResults(const std::string& path,
                                    const ScheduleResult& result,
                                    std::string& error);

    // Ghi Gantt: pid,start,end (IDLE = -1)
    static bool writeGantt(const std::string& path,
                           const ScheduleResult& result,
                           std::string& error);

    // Ghi bảng so sánh: metric,np,p — chỉ format, không tính toán
    static bool writeComparison(const std::string& path,
                                const ScheduleResult& np,
                                const ScheduleResult& p,
                                std::string& error);
};

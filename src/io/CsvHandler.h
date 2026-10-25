#pragma once

#include <string>
#include <vector>
#include "core/Process.h"
#include "core/ScheduleResult.h"

class CsvHandler {
public:
    static bool readProcesses(const std::string& path,
                              std::vector<Process>& out,
                              std::string& error);

    static bool writeProcessResults(const std::string& path,
                                    const ScheduleResult& result,
                                    std::string& error);

    static bool writeGantt(const std::string& path,
                           const ScheduleResult& result,
                           std::string& error);

    static bool writeComparison(const std::string& path,
                                const ScheduleResult& np,
                                const ScheduleResult& p,
                                std::string& error);
};

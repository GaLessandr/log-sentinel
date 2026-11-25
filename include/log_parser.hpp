#pragma once

#include "log_entry.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace sentinel {

class LogParser {
public:
    static std::optional<LogEntry> parseLine(std::string_view line);

private:
    static int64_t parseTimestampToEpoch(std::string_view ts);
};

} // namespace sentinel

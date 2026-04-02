#pragma once
#include "common.h"
 
class Logger {
public:
    enum class Level { INFO, WARNING, ERR, SUCCESS };
 
    static Logger& instance();
 
    void init(const fs::path& logDir);
    void log(Level level, const std::string& action, const std::string& detail = "");
    void flush();
 
private:
    Logger() = default;
    fs::path logFile_;
    std::mutex mutex_;
    std::vector<std::string> buffer_;
 
    std::string levelStr(Level l);
    std::string escapeJson(const std::string& s);
};








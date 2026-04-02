#include "logger.h"
 
Logger& Logger::instance() {
    static Logger inst;
    return inst;
}
 
void Logger::init(const fs::path& logDir) {
    fs::create_directories(logDir);
 
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream name;
    name << "log_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".json";
    logFile_ = logDir / name.str();
 
    // Инициализируем файл пустым массивом
    std::ofstream f(logFile_);
    f << "[\n";
    f.close();
 
    log(Level::INFO, "STARTUP", "File Manager started");
}
 
void Logger::log(Level level, const std::string& action, const std::string& detail) {
    std::lock_guard<std::mutex> lock(mutex_);
 
    std::ostringstream entry;
    entry << "  {\n"
          << "    \"timestamp\": \"" << escapeJson(getCurrentTimestamp()) << "\",\n"
          << "    \"level\": \""     << escapeJson(levelStr(level))       << "\",\n"
          << "    \"action\": \""    << escapeJson(action)                << "\",\n"
          << "    \"detail\": \""    << escapeJson(detail)                << "\"\n"
          << "  }";
 
    buffer_.push_back(entry.str());
 
    // Пишем сразу, чтобы не терять при краше
    if (!logFile_.empty()) {
        // Читаем текущее содержимое
        std::ifstream fin(logFile_);
        std::string content((std::istreambuf_iterator<char>(fin)),
                             std::istreambuf_iterator<char>());
        fin.close();
 
        // Удаляем последний ']' и пробельные символы
        while (!content.empty() && (content.back() == '\n' || content.back() == '\r' ||
                                    content.back() == ' '  || content.back() == ']'))
            content.pop_back();
 
        // Добавляем запятую если уже есть записи
        bool needComma = content.find('{') != std::string::npos;
        if (needComma) content += ",\n";
 
        content += entry.str();
        content += "\n]";
 
        // Перезаписываем файл
        std::ofstream out(logFile_);
        out << content;
    }
}
 
void Logger::flush() {
    log(Level::INFO, "SHUTDOWN", "File Manager exited normally");
}
 
std::string Logger::levelStr(Level l) {
    switch (l) {
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERR:     return "ERROR";
        case Level::SUCCESS: return "SUCCESS";
    }
    return "INFO";
}
 
std::string Logger::escapeJson(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}
 
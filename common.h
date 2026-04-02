#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <functional>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
namespace fs = std::filesystem;
 














// анси цвета
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";
 
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
 
    const std::string BG_BLUE  = "\033[44m";
    const std::string BG_BLACK = "\033[40m";
}
 
// утилиты
inline std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
 
inline std::string formatSize(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit = 0;
    while (size >= 1024.0 && unit < 4) { size /= 1024.0; ++unit; }
    std::ostringstream oss;
    if (unit == 0) oss << (uintmax_t)size << " " << units[unit];
    else           oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}
 
inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
 
inline void printDivider(const std::string& ch = "─", int width = 60) {
    std::cout << Color::DIM;
    for (int i = 0; i < width; ++i) std::cout << ch;
    std::cout << Color::RESET << "\n";
}
 
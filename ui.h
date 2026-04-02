#pragma once
#include "common.h"
#include "fileops.h"
#include "search.h"
 
class UI {
public:
    static void printBanner();
    static void printHeader(const fs::path& currentPath, size_t total);
    static void printHelp();
    static void printEntries(const std::vector<FileOps::EntryInfo>& entries,
                             int selected = -1, bool showIndex = true);
    static void printSearchResults(const std::vector<SearchResult>& results);
    static void printFileInfo(const FileOps::EntryInfo& info, const fs::path& full);
    static void printPreview(const fs::path& path);
    static void printSuccess(const std::string& msg);
    static void printError(const std::string& msg);
    static void printWarning(const std::string& msg);
    static void printInfo(const std::string& msg);
 
    // ввод
    static std::string prompt(const std::string& question);
    static bool        confirm(const std::string& question);
};
 

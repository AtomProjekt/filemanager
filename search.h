#pragma once
#include "common.h"
 
struct SearchResult {
    fs::path  path;
    uintmax_t size;
    bool      isDir;
    std::string lastModified;
};
 
class SearchEngine {
public:
    struct Options {
        std::string pattern;        // подстрока или маска
        bool        caseSensitive = false;
        bool        searchContent  = false; // искать внутри файлов
        uintmax_t   minSize        = 0;
        uintmax_t   maxSize        = UINTMAX_MAX;
        std::string extension;      // фильтр по расширению без точки
    };
 
// быстрый поиск по имени многопоточный

    std::vector<SearchResult> searchByName(const fs::path& root, const Options& opts,
    std::atomic<bool>& cancelled);
 
// поиск содержимого внутри текстовых файлов

    std::vector<SearchResult> searchByContent(const fs::path& root, const Options& opts,
    std::atomic<bool>& cancelled);
 
private:
    bool matchName(const std::string& filename, const Options& opts);
    bool matchContent(const fs::path& file, const std::string& pattern, bool caseSensitive);
    std::string getLastModified(const fs::path& p);
};
 
#pragma once
#include "common.h"
 
class FileOps {
public:
    struct EntryInfo {
        std::string name;
        bool        isDir;
        uintmax_t   size;
        std::string permissions;
        std::string lastModified;
        std::string extension;
    };
 
    // чтение дериктории
    static std::vector<EntryInfo> listDir(const fs::path& dir, bool showHidden = false);
 
    // Операции
    static bool copyEntry(const fs::path& src, const fs::path& dst, std::string& error);
    static bool moveEntry(const fs::path& src, const fs::path& dst, std::string& error);
    static bool deleteEntry(const fs::path& path, std::string& error);
    static bool createDir(const fs::path& path, std::string& error);
    static bool renameEntry(const fs::path& path, const std::string& newName, std::string& error);
 
    // информация о файле
    static EntryInfo getInfo(const fs::path& path);
 
    // открыть файл в системном редакторе или просмотрщике
    static void openFile(const fs::path& path);
 
    // чтение текстового файла первые N строк
    static std::vector<std::string> readTextFile(const fs::path& path, size_t maxLines = 50);
 
    // размер директории рекурсивно
    static uintmax_t dirSize(const fs::path& dir);
 
private:
    static std::string getPermissions(const fs::path& path);
    static std::string getLastModified(const fs::path& path);
};
 
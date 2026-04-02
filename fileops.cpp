#include "fileops.h"
 
// ─── Вспомогательные ─────────────────────────────────────────────────────────
 
std::string FileOps::getPermissions(const fs::path& path) {
    try {
        auto perms = fs::status(path).permissions();
        std::string s = "----------";
        if (fs::is_directory(path)) s[0] = 'd';
        if ((perms & fs::perms::owner_read)   != fs::perms::none) s[1] = 'r';
        if ((perms & fs::perms::owner_write)  != fs::perms::none) s[2] = 'w';
        if ((perms & fs::perms::owner_exec)   != fs::perms::none) s[3] = 'x';
        if ((perms & fs::perms::group_read)   != fs::perms::none) s[4] = 'r';
        if ((perms & fs::perms::group_write)  != fs::perms::none) s[5] = 'w';
        if ((perms & fs::perms::group_exec)   != fs::perms::none) s[6] = 'x';
        if ((perms & fs::perms::others_read)  != fs::perms::none) s[7] = 'r';
        if ((perms & fs::perms::others_write) != fs::perms::none) s[8] = 'w';
        if ((perms & fs::perms::others_exec)  != fs::perms::none) s[9] = 'x';
        return s;
    } catch (...) { return "??????????"; }
}
 
std::string FileOps::getLastModified(const fs::path& path) {
    try {
        auto ftime = fs::last_write_time(path);
        auto sctp  = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                         ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t t = std::chrono::system_clock::to_time_t(sctp);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
        return oss.str();
    } catch (...) { return "unknown"; }
}
 
// ─── Листинг ─────────────────────────────────────────────────────────────────
 
std::vector<FileOps::EntryInfo> FileOps::listDir(const fs::path& dir, bool showHidden) {
    std::vector<EntryInfo> entries;
    std::error_code ec;
 
    for (const auto& entry : fs::directory_iterator(dir,
            fs::directory_options::skip_permission_denied, ec)) {
        try {
            std::string name = entry.path().filename().string();
            if (!showHidden && !name.empty() && name[0] == '.') continue;
 
            EntryInfo info;
            info.name         = name;
            info.isDir        = entry.is_directory();
            info.permissions  = getPermissions(entry.path());
            info.lastModified = getLastModified(entry.path());
            info.extension    = entry.path().extension().string();
 
            if (!info.isDir) {
                std::error_code se;
                info.size = fs::file_size(entry.path(), se);
                if (se) info.size = 0;
            } else {
                info.size = 0;
            }
 
            entries.push_back(std::move(info));
        } catch (...) {}
    }
 
    // Сначала директории, потом файлы, по алфавиту
    std::sort(entries.begin(), entries.end(), [](const EntryInfo& a, const EntryInfo& b) {
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return a.name < b.name;
    });
 
    return entries;
}
 
// ─── Операции ────────────────────────────────────────────────────────────────
 
bool FileOps::copyEntry(const fs::path& src, const fs::path& dst, std::string& error) {
    try {
        fs::path target = dst;
        if (fs::is_directory(dst)) target = dst / src.filename();
 
        if (fs::is_directory(src)) {
            fs::copy(src, target,
                fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        } else {
            fs::copy_file(src, target, fs::copy_options::overwrite_existing);
        }
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}
 
bool FileOps::moveEntry(const fs::path& src, const fs::path& dst, std::string& error) {
    try {
        fs::path target = dst;
        if (fs::is_directory(dst)) target = dst / src.filename();
        fs::rename(src, target);
        return true;
    } catch (const std::exception& e) {
        // Если разные тома — копируем + удаляем
        try {
            fs::path target = dst;
            if (fs::is_directory(dst)) target = dst / src.filename();
            if (fs::is_directory(src))
                fs::copy(src, target, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            else
                fs::copy_file(src, target, fs::copy_options::overwrite_existing);
            fs::remove_all(src);
            return true;
        } catch (const std::exception& e2) {
            error = e2.what();
            return false;
        }
    }
}
 
bool FileOps::deleteEntry(const fs::path& path, std::string& error) {
    try {
        fs::remove_all(path);
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}
 
bool FileOps::createDir(const fs::path& path, std::string& error) {
    try {
        fs::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}
 
bool FileOps::renameEntry(const fs::path& path, const std::string& newName, std::string& error) {
    try {
        fs::path newPath = path.parent_path() / newName;
        fs::rename(path, newPath);
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}
 
FileOps::EntryInfo FileOps::getInfo(const fs::path& path) {
    EntryInfo info;
    info.name         = path.filename().string();
    info.isDir        = fs::is_directory(path);
    info.permissions  = getPermissions(path);
    info.lastModified = getLastModified(path);
    info.extension    = path.extension().string();
    if (!info.isDir) {
        std::error_code ec;
        info.size = fs::file_size(path, ec);
        if (ec) info.size = 0;
    } else {
        info.size = 0;
    }
    return info;
}
 
void FileOps::openFile(const fs::path& path) {
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + path.string() + "\"";
#elif __APPLE__
    std::string cmd = "open \"" + path.string() + "\"";
#else
    std::string cmd = "xdg-open \"" + path.string() + "\" &";
#endif
    system(cmd.c_str());
}
 
std::vector<std::string> FileOps::readTextFile(const fs::path& path, size_t maxLines) {
    std::vector<std::string> lines;
    std::ifstream f(path);
    std::string line;
    size_t count = 0;
    while (std::getline(f, line) && count < maxLines) {
        lines.push_back(line);
        ++count;
    }
    return lines;
}
 
uintmax_t FileOps::dirSize(const fs::path& dir) {
    uintmax_t total = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dir,
                fs::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file()) {
                std::error_code ec;
                total += fs::file_size(entry.path(), ec);
            }
        }
    } catch (...) {}
    return total;
}
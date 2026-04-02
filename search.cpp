
#include "search.h"
 
//вспомогательные функции
 
static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}
 
bool SearchEngine::matchName(const std::string& filename, const Options& opts) {
    std::string name    = opts.caseSensitive ? filename         : toLower(filename);
    std::string pattern = opts.caseSensitive ? opts.pattern     : toLower(opts.pattern);
 
    // Простая wildcard (*) поддержка
    if (pattern.find('*') != std::string::npos) {
        // Разбиваем по '*' и проверяем части
        std::vector<std::string> parts;
        std::istringstream ss(pattern);
        std::string tok;
        while (std::getline(ss, tok, '*')) parts.push_back(tok);
 
        size_t pos = 0;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (parts[i].empty()) continue;
            size_t found = name.find(parts[i], pos);
            if (found == std::string::npos) return false;
            if (i == 0 && !pattern.empty() && pattern[0] != '*' && found != 0) return false;
            pos = found + parts[i].size();
        }
        if (!pattern.empty() && pattern.back() != '*') {
            // последняя часть должна быть в конце
            if (!parts.empty() && !parts.back().empty()) {
                if (name.size() < parts.back().size()) return false;
                if (name.substr(name.size() - parts.back().size()) != parts.back()) return false;
            }
        }
        return true;
    }
 
    return name.find(pattern) != std::string::npos;
}
 
bool SearchEngine::matchContent(const fs::path& file, const std::string& pattern, bool caseSensitive) {
    std::ifstream f(file, std::ios::binary);
    if (!f) return false;
 
    // чтение по чанкам
    const size_t CHUNK = 65536;
    std::string buf(CHUNK, '\0');
    std::string pat = caseSensitive ? pattern : toLower(pattern);
    std::string tail; // хвост предыдущего чанка для склейки
 
    while (f.read(&buf[0], CHUNK) || f.gcount() > 0) {
        std::string chunk = tail + buf.substr(0, static_cast<size_t>(f.gcount()));
        std::string search = caseSensitive ? chunk : toLower(chunk);
        if (search.find(pat) != std::string::npos) return true;
        tail = chunk.size() >= pat.size() ? chunk.substr(chunk.size() - pat.size() + 1) : chunk;
    }
    return false;
}
 
std::string SearchEngine::getLastModified(const fs::path& p) {
    try {
        auto ftime = fs::last_write_time(p);
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
 
// поиск по имени 
 
std::vector<SearchResult> SearchEngine::searchByName(
        const fs::path& root, const Options& opts, std::atomic<bool>& cancelled) {
 
    std::vector<SearchResult> results;
    std::mutex                resultMutex;
 
    auto processEntry = [&](const fs::directory_entry& entry) {
        if (cancelled) return;
        try {
            const auto& p    = entry.path();
            std::string name = p.filename().string();
 
            // фильтр по расширению
            if (!opts.extension.empty()) {
                std::string ext = p.extension().string();
                if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
                std::string fext = opts.caseSensitive ? ext            : toLower(ext);
                std::string fopt = opts.caseSensitive ? opts.extension : toLower(opts.extension);
                if (fext != fopt) return;
            }
 
            if (!matchName(name, opts)) return;
 
            uintmax_t sz = 0;
            bool isDir = entry.is_directory();
            if (!isDir) {
                std::error_code ec;
                sz = fs::file_size(p, ec);
                if (ec) sz = 0;
            }
 
            // фильтр по размеру
            if (!isDir && (sz < opts.minSize || sz > opts.maxSize)) return;
 
            SearchResult r;
            r.path         = p;
            r.size         = sz;
            r.isDir        = isDir;
            r.lastModified = getLastModified(p);
 
            std::lock_guard<std::mutex> lock(resultMutex);
            results.push_back(std::move(r));
        } catch (...) {}
    };
 
    try {
        fs::recursive_directory_iterator it(root,
            fs::directory_options::skip_permission_denied);
        for (const auto& entry : it) {
            if (cancelled) break;
            processEntry(entry);
        }
    } catch (...) {}
 
    return results;
}
 
// поиск по содержимому
 
std::vector<SearchResult> SearchEngine::searchByContent(
        const fs::path& root, const Options& opts, std::atomic<bool>& cancelled) {
 
    std::vector<SearchResult> results;
 
    try {
        fs::recursive_directory_iterator it(root,
            fs::directory_options::skip_permission_denied);
        for (const auto& entry : it) {
            if (cancelled) break;
            if (!entry.is_regular_file()) continue;
            try {
                const auto& p = entry.path();
 
                // Фильтр по расширению
                if (!opts.extension.empty()) {
                    std::string ext = p.extension().string();
                    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
                    if (toLower(ext) != toLower(opts.extension)) continue;
                }
 
                std::error_code ec;
                uintmax_t sz = fs::file_size(p, ec);
                if (ec || sz > 50 * 1024 * 1024) continue; // пропускаем >50MB
 
                if (matchContent(p, opts.pattern, opts.caseSensitive)) {
                    SearchResult r;
                    r.path         = p;
                    r.size         = sz;
                    r.isDir        = false;
                    r.lastModified = getLastModified(p);
                    results.push_back(std::move(r));
                }
            } catch (...) {}
        }
    } catch (...) {}
 
    return results;
}
 
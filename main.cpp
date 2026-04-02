#include <windows.h>
#include "common.h"
#include "logger.h"
#include "fileops.h"
#include "search.h"
#include "ui.h"
 
// вспомогательные функции
 
static std::vector<std::string> splitArgs(const std::string& line) {
    std::vector<std::string> args;
    std::istringstream ss(line);
    std::string tok;
    // Простая обработка кавычек
    bool inQuote = false;
    std::string cur;
    for (char c : line) {
        if (c == '"') { inQuote = !inQuote; continue; }
        if (c == ' ' && !inQuote) {
            if (!cur.empty()) { args.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) args.push_back(cur);
    return args;
}
 
static int parseIndex(const std::string& s, size_t max) {
    try {
        int idx = std::stoi(s) - 1;
        if (idx < 0 || (size_t)idx >= max) return -1;
        return idx;
    } catch (...) { return -1; }
}
 
// главный цикл
 
int main() {
#ifdef _WIN32
    // ютф 8 и анси
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // заголовок окна
    SetConsoleTitleA("FM++ File Manager");
#endif
 
    // инициализация логера
    fs::path logDir = fs::current_path() / "fm_logs";
    Logger::instance().init(logDir);
 
    // приветствие 
    UI::printBanner();
    UI::printInfo("Логи сохраняются в: " + logDir.string());
    UI::printInfo("Введите 'help' для справки.");
    std::cout << "\n";
 
    // состояние
    fs::path currentPath = fs::current_path();
    std::vector<FileOps::EntryInfo> entries;
    bool showHidden = false;
    bool needRefresh = true;
 
    SearchEngine searcher;
 
    auto refresh = [&]() {
        try {
            entries = FileOps::listDir(currentPath, showHidden);
        } catch (const std::exception& e) {
            UI::printError("Не удалось прочитать директорию: " + std::string(e.what()));
            entries.clear();
        }
    };
 
    // REPL
    while (true) {
        if (needRefresh) {
            refresh();
            needRefresh = false;
        }
 
        UI::printHeader(currentPath, entries.size());
        UI::printEntries(entries);
 
        std::string line = UI::prompt("Команда:");
        if (line.empty()) continue;
 
        auto args = splitArgs(line);
        if (args.empty()) continue;
 
        std::string cmd = args[0];
        // Приводим команду к нижнему регистру
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
 
        Logger::instance().log(Logger::Level::INFO, "COMMAND", line);
 
        // exit
        if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            UI::printSuccess("До свидания!");
            Logger::instance().flush();
            break;
        }
 
        // help
        else if (cmd == "help") {
            UI::printHelp();
            std::cout << "  Нажмите Enter для продолжения...";
            std::cin.ignore();
            needRefresh = true;
        }
 
        // cls clear
        else if (cmd == "cls" || cmd == "clear") {
            clearScreen();
            needRefresh = true;
        }
 
        // ls dir
        else if (cmd == "ls" || cmd == "dir" || cmd == "ll") {
            needRefresh = true;
        }
 
        // hidden
        else if (cmd == "hidden") {
            showHidden = !showHidden;
            UI::printInfo(showHidden ? "Скрытые файлы: показаны" : "Скрытые файлы: скрыты");
            needRefresh = true;
        }
 
        // cd
        else if (cmd == "cd") {
            if (args.size() < 2) {
                UI::printError("Укажите путь. Пример: cd /home/user");
                continue;
            }
            std::string dest = args[1];
            fs::path newPath;
            if (dest == "..") {
                newPath = currentPath.parent_path();
            } else if (dest == "~" || dest == "home") {
#ifdef _WIN32
                newPath = fs::path(getenv("USERPROFILE") ? getenv("USERPROFILE") : "C:\\");
#else
                newPath = fs::path(getenv("HOME") ? getenv("HOME") : "/");
#endif
            } else {
                newPath = fs::absolute(currentPath / dest);
            }
 
            if (fs::is_directory(newPath)) {
                currentPath = newPath;
                Logger::instance().log(Logger::Level::INFO, "CD", newPath.string());
                needRefresh = true;
            } else {
                UI::printError("Директория не существует: " + newPath.string());
            }
        }
 
        // open
        else if (cmd == "open") {
            if (args.size() < 2) { UI::printError("Укажите номер файла."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            const auto& e = entries[idx];
            fs::path full = currentPath / e.name;
            if (e.isDir) {
                currentPath = full;
                Logger::instance().log(Logger::Level::INFO, "OPEN_DIR", full.string());
                needRefresh = true;
            } else {
                FileOps::openFile(full);
                Logger::instance().log(Logger::Level::INFO, "OPEN_FILE", full.string());
            }
        }
 
        // info
        else if (cmd == "info") {
            if (args.size() < 2) { UI::printError("Укажите номер."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path full = currentPath / entries[idx].name;
            auto info = FileOps::getInfo(full);
            UI::printFileInfo(info, full);
            Logger::instance().log(Logger::Level::INFO, "INFO", full.string());
            std::cout << "  Нажмите Enter..."; std::cin.ignore();
            needRefresh = true;
        }
 
        // preview
        else if (cmd == "preview") {
            if (args.size() < 2) { UI::printError("Укажите номер."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path full = currentPath / entries[idx].name;
            UI::printPreview(full);
            Logger::instance().log(Logger::Level::INFO, "PREVIEW", full.string());
            std::cout << "  Нажмите Enter..."; std::cin.ignore();
            needRefresh = true;
        }
 
        // cp N dst
        else if (cmd == "cp") {
            if (args.size() < 3) { UI::printError("Использование: cp <N> <путь-назначения>"); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path src = currentPath / entries[idx].name;
            fs::path dst = fs::absolute(currentPath / args[2]);
            std::string err;
            if (FileOps::copyEntry(src, dst, err)) {
                UI::printSuccess("Скопировано: " + src.filename().string() + " → " + dst.string());
                Logger::instance().log(Logger::Level::SUCCESS, "COPY",
                    src.string() + " -> " + dst.string());
            } else {
                UI::printError("Ошибка копирования: " + err);
                Logger::instance().log(Logger::Level::ERR, "COPY_FAIL", err);
            }
            needRefresh = true;
        }
 
        // mv N dst
        else if (cmd == "mv") {
            if (args.size() < 3) { UI::printError("Использование: mv <N> <путь-назначения>"); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path src = currentPath / entries[idx].name;
            fs::path dst = fs::absolute(currentPath / args[2]);
            std::string err;
            if (FileOps::moveEntry(src, dst, err)) {
                UI::printSuccess("Перемещено: " + src.filename().string() + " → " + dst.string());
                Logger::instance().log(Logger::Level::SUCCESS, "MOVE",
                    src.string() + " -> " + dst.string());
                needRefresh = true;
            } else {
                UI::printError("Ошибка перемещения: " + err);
                Logger::instance().log(Logger::Level::ERR, "MOVE_FAIL", err);
            }
        }
 
        // rm N
        else if (cmd == "rm" || cmd == "del" || cmd == "delete") {
            if (args.size() < 2) { UI::printError("Укажите номер."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path target = currentPath / entries[idx].name;
            if (UI::confirm("Удалить \"" + target.filename().string() + "\"? Это необратимо!")) {
                std::string err;
                if (FileOps::deleteEntry(target, err)) {
                    UI::printSuccess("Удалено: " + target.filename().string());
                    Logger::instance().log(Logger::Level::SUCCESS, "DELETE", target.string());
                    needRefresh = true;
                } else {
                    UI::printError("Ошибка удаления: " + err);
                    Logger::instance().log(Logger::Level::ERR, "DELETE_FAIL", err);
                }
            } else {
                UI::printInfo("Отменено.");
            }
        }
 
        // mkdir имя
        else if (cmd == "mkdir") {
            std::string name;
            if (args.size() >= 2) {
                name = args[1];
            } else {
                name = UI::prompt("Имя новой директории:");
            }
            if (name.empty()) { UI::printError("Имя не может быть пустым."); continue; }
            fs::path newDir = currentPath / name;
            std::string err;
            if (FileOps::createDir(newDir, err)) {
                UI::printSuccess("Создана директория: " + name);
                Logger::instance().log(Logger::Level::SUCCESS, "MKDIR", newDir.string());
                needRefresh = true;
            } else {
                UI::printError("Ошибка: " + err);
                Logger::instance().log(Logger::Level::ERR, "MKDIR_FAIL", err);
            }
        }
 
        // rename N
        else if (cmd == "rename" || cmd == "rn") {
            if (args.size() < 2) { UI::printError("Укажите номер."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            std::string newName;
            if (args.size() >= 3) {
                newName = args[2];
            } else {
                newName = UI::prompt("Новое имя для \"" + entries[idx].name + "\":");
            }
            if (newName.empty()) { UI::printError("Имя не может быть пустым."); continue; }
            std::string err;
            fs::path target = currentPath / entries[idx].name;
            if (FileOps::renameEntry(target, newName, err)) {
                UI::printSuccess("Переименовано: " + entries[idx].name + " → " + newName);
                Logger::instance().log(Logger::Level::SUCCESS, "RENAME",
                    entries[idx].name + " -> " + newName);
                needRefresh = true;
            } else {
                UI::printError("Ошибка переименования: " + err);
                Logger::instance().log(Logger::Level::ERR, "RENAME_FAIL", err);
            }
        }
 
        // size N
        else if (cmd == "size") {
            if (args.size() < 2) { UI::printError("Укажите номер."); continue; }
            int idx = parseIndex(args[1], entries.size());
            if (idx < 0) { UI::printError("Неверный номер."); continue; }
            fs::path target = currentPath / entries[idx].name;
            if (entries[idx].isDir) {
                UI::printInfo("Вычисляю размер...");
                uintmax_t sz = FileOps::dirSize(target);
                UI::printSuccess("Размер директории \"" + entries[idx].name + "\": " + formatSize(sz));
            } else {
                UI::printSuccess("Размер: " + formatSize(entries[idx].size));
            }
            std::cout << "  Нажмите Enter..."; std::cin.ignore();
            needRefresh = true;
        }
 
        // find pattern
        else if (cmd == "find") {
            std::string pattern;
            if (args.size() >= 2) {
                pattern = args[1];
            } else {
                pattern = UI::prompt("Что искать (имя / маска, например *.txt):");
            }
            if (pattern.empty()) continue;
 
            fs::path searchRoot = currentPath;
            // Можно указать другой корень: find pattern /path
            if (args.size() >= 3) {
                fs::path alt(args[2]);
                if (fs::is_directory(alt)) searchRoot = alt;
            }
 
            UI::printInfo("Поиск \"" + pattern + "\" в " + searchRoot.string() + " ...");
 
            SearchEngine::Options opts;
            opts.pattern = pattern;
            opts.caseSensitive = false;
 
            std::atomic<bool> cancelled(false);
 
            // запускаем в отдельном потоке с возможностью отмены Ctrl+C
            std::vector<SearchResult> results;
            std::thread searchThread([&]() {
                results = searcher.searchByName(searchRoot, opts, cancelled);
            });
            searchThread.join();
 
            Logger::instance().log(Logger::Level::INFO, "SEARCH_NAME",
                "pattern=" + pattern + " root=" + searchRoot.string() +
                " results=" + std::to_string(results.size()));
 
            if (results.empty()) {
                UI::printWarning("Ничего не найдено.");
            } else {
                UI::printSearchResults(results);
                // можно открыть найденный файл
                std::string choice = UI::prompt("Открыть файл №N (или Enter для выхода):");
                if (!choice.empty()) {
                    int idx = parseIndex(choice, results.size());
                    if (idx >= 0) {
                        const auto& r = results[idx];
                        if (r.isDir) {
                            currentPath = r.path;
                            Logger::instance().log(Logger::Level::INFO, "NAVIGATE", r.path.string());
                            needRefresh = true;
                        } else {
                            FileOps::openFile(r.path);
                        }
                    }
                }
            }
            needRefresh = true;
        }
 
        // findc text
        else if (cmd == "findc") {
            std::string pattern;
            if (args.size() >= 2) {
                pattern = args[1];
            } else {
                pattern = UI::prompt("Искомый текст внутри файлов:");
            }
            if (pattern.empty()) continue;
 
            std::string extFilter;
            if (args.size() >= 3) extFilter = args[2];
            else extFilter = UI::prompt("Расширение файлов (txt, cpp... или Enter для всех):");
 
            UI::printInfo("Поиск содержимого \"" + pattern + "\" в " + currentPath.string() + " ...");
            UI::printWarning("Это может занять время. Нажмите Enter для начала.");
            std::cin.ignore();
 
            SearchEngine::Options opts;
            opts.pattern      = pattern;
            opts.caseSensitive = false;
            opts.extension    = extFilter;
 
            std::atomic<bool> cancelled(false);
            std::vector<SearchResult> results = searcher.searchByContent(currentPath, opts, cancelled);
 
            Logger::instance().log(Logger::Level::INFO, "SEARCH_CONTENT",
                "pattern=" + pattern + " results=" + std::to_string(results.size()));
 
            if (results.empty()) {
                UI::printWarning("Ничего не найдено.");
            } else {
                UI::printSearchResults(results);
            }
            std::cout << "  Нажмите Enter..."; std::cin.ignore();
            needRefresh = true;
        }
 
        // неизвестная команда
        else {
            UI::printWarning("Неизвестная команда: \"" + cmd + "\". Введите 'help' для справки.");
            Logger::instance().log(Logger::Level::WARNING, "UNKNOWN_CMD", cmd);
        }
    }
 
    return 0;
}
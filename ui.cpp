#include "ui.h"
 
void UI::printBanner() {
    clearScreen();
    std::cout << Color::CYAN << Color::BOLD;
    std::cout << R"(
╔══════════════════════════════════════════════════════════════════════╗
║                                                                      ║
║  ███╗   ███╗ █████╗ ███╗   ██╗ █████╗  ██████╗ ███████╗██████╗       ║
║  ████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝ ██╔════╝██╔══██╗      ║
║  ██╔████╔██║███████║██╔██╗ ██║███████║██║  ███╗█████╗  ██████╔╝      ║
║  ██║╚██╔╝██║██╔══██║██║╚██╗██║██╔══██║██║   ██║██╔══╝  ██╔══██╗      ║
║  ██║ ╚═╝ ██║██║  ██║██║ ╚████║██║  ██║╚██████╔╝███████╗██║  ██║      ║
║  ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝      ║
║                                                                      ║
║ ──────────────────────────────────────────────────────────────────── ║
║        File Manager v1.0  │  C++17  │  by AtomProjekt                ║
╚══════════════════════════════════════════════════════════════════════╝
)" << Color::RESET;
    std::cout << "\n";
}
 
void UI::printHeader(const fs::path& currentPath, size_t total) {
    printDivider("═", 64);
    std::cout << Color::BOLD << Color::BLUE << "  📁 " << Color::RESET
              << Color::BOLD << currentPath.string() << Color::RESET;
    std::cout << Color::DIM << "  (" << total << " элем.)" << Color::RESET << "\n";
    printDivider("─", 64);
    // Колонки
    std::cout << Color::DIM
              << std::left
              << "  " << std::setw(4)  << "№"
              << std::setw(32) << "Имя"
              << std::setw(12) << "Размер"
              << std::setw(16) << "Изменён"
              << Color::RESET << "\n";
    printDivider("─", 64);
}
 
void UI::printEntries(const std::vector<FileOps::EntryInfo>& entries, int selected, bool showIndex) {
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        bool sel = (selected >= 0 && (int)i == selected);
 
        if (sel) std::cout << Color::BG_BLUE;
 
        // Индекс
        if (showIndex)
            std::cout << Color::DIM << "  " << std::setw(3) << (i + 1) << " " << Color::RESET;
 
        // Иконка + имя
        std::string icon  = e.isDir ? "📂 " : "📄 ";
        std::string color = e.isDir ? Color::YELLOW : Color::WHITE;
        if (!e.extension.empty()) {
            std::string ext = e.extension;
            if (ext == ".cpp" || ext == ".h" || ext == ".py" || ext == ".js" || ext == ".java")
                color = Color::GREEN;
            else if (ext == ".zip" || ext == ".tar" || ext == ".gz" || ext == ".rar" || ext == ".7z")
                color = Color::MAGENTA;
            else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp")
                color = Color::CYAN;
            else if (ext == ".mp3" || ext == ".wav" || ext == ".flac" || ext == ".mp4" || ext == ".mkv")
                color = Color::BLUE;
            else if (ext == ".exe" || ext == ".sh" || ext == ".bat")
                color = Color::RED;
        }
 
        std::string name = e.name;
        if (name.size() > 28) name = name.substr(0, 25) + "...";
 
        std::cout << color << Color::BOLD << icon << std::left << std::setw(28) << name << Color::RESET;
 
        // Размер
        if (e.isDir)
            std::cout << Color::DIM << std::setw(12) << "<DIR>" << Color::RESET;
        else
            std::cout << Color::DIM << std::setw(12) << formatSize(e.size) << Color::RESET;
 
        // Дата
        std::cout << Color::DIM << std::setw(16) << e.lastModified << Color::RESET;
 
        if (sel) std::cout << Color::RESET;
        std::cout << "\n";
    }
}
 
void UI::printSearchResults(const std::vector<SearchResult>& results) {
    printDivider("─", 64);
    std::cout << Color::BOLD << Color::CYAN << "  Результаты поиска: " << results.size() << " найдено\n" << Color::RESET;
    printDivider("─", 64);
 
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        std::cout << Color::DIM << "  " << std::setw(4) << (i + 1) << Color::RESET;
 
        std::string icon  = r.isDir ? "📂 " : "📄 ";
        std::string color = r.isDir ? Color::YELLOW : Color::WHITE;
 
        // Показываем полный путь
        std::string pathStr = r.path.string();
        if (pathStr.size() > 45) pathStr = "..." + pathStr.substr(pathStr.size() - 42);
 
        std::cout << color << icon << std::left << std::setw(46) << pathStr << Color::RESET;
 
        if (!r.isDir)
            std::cout << Color::DIM << formatSize(r.size) << Color::RESET;
 
        std::cout << "\n";
    }
    printDivider("─", 64);
}
 
void UI::printFileInfo(const FileOps::EntryInfo& info, const fs::path& full) {
    printDivider("─", 64);
    std::cout << Color::BOLD << "  Информация о файле\n" << Color::RESET;
    printDivider("─", 64);
    std::cout << Color::CYAN   << "  Имя:        " << Color::RESET << info.name << "\n";
    std::cout << Color::CYAN   << "  Путь:       " << Color::RESET << full.string() << "\n";
    std::cout << Color::CYAN   << "  Тип:        " << Color::RESET << (info.isDir ? "Директория" : "Файл") << "\n";
    if (!info.extension.empty())
        std::cout << Color::CYAN << "  Расширение: " << Color::RESET << info.extension << "\n";
    if (!info.isDir)
        std::cout << Color::CYAN << "  Размер:     " << Color::RESET << formatSize(info.size) << "\n";
    std::cout << Color::CYAN   << "  Изменён:    " << Color::RESET << info.lastModified << "\n";
    std::cout << Color::CYAN   << "  Права:      " << Color::RESET << info.permissions << "\n";
    printDivider("─", 64);
}
 
void UI::printPreview(const fs::path& path) {
    auto lines = FileOps::readTextFile(path, 30);
    if (lines.empty()) {
        printWarning("Файл пуст или не является текстовым.");
        return;
    }
    printDivider("─", 64);
    std::cout << Color::BOLD << "  Предпросмотр (первые 30 строк)\n" << Color::RESET;
    printDivider("─", 64);
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];
        if (line.size() > 60) line = line.substr(0, 57) + "...";
        std::cout << Color::DIM << std::setw(4) << (i + 1) << "  " << Color::RESET << line << "\n";
    }
    printDivider("─", 64);
}
 
void UI::printHelp() {
    clearScreen();
    std::cout << Color::BOLD << Color::CYAN << "\n  ╔═══ СПРАВКА ═══════════════════════════════════════════╗\n" << Color::RESET;
    auto row = [](const std::string& cmd, const std::string& desc) {
        std::cout << Color::CYAN << "  ║  " << Color::RESET
                  << Color::BOLD << Color::YELLOW << std::left << std::setw(14) << cmd << Color::RESET
                  << std::setw(44) << desc
                  << Color::CYAN << "║\n" << Color::RESET;
    };
    std::cout << Color::CYAN << "  ╠════════════════════════════════════════════════════════╣\n" << Color::RESET;
    row("ls / dir",     "Показать содержимое директории");
    row("cd <путь>",    "Перейти в директорию");
    row("cd ..",        "Подняться на уровень выше");
    row("open <N>",     "Открыть файл №N / войти в папку");
    row("info <N>",     "Информация о файле №N");
    row("preview <N>",  "Предпросмотр текстового файла");
    row("cp <N> <dst>", "Копировать файл №N в <dst>");
    row("mv <N> <dst>", "Переместить файл №N в <dst>");
    row("rm <N>",       "Удалить файл/папку №N");
    row("mkdir <имя>",  "Создать директорию");
    row("rename <N>",   "Переименовать файл №N");
    row("size <N>",     "Размер директории №N");
    row("find <имя>",   "Быстрый поиск по имени");
    row("findc <текст>","Поиск по содержимому файлов");
    row("hidden",       "Показать/скрыть скрытые файлы");
    row("cls / clear",  "Очистить экран");
    row("help",         "Эта справка");
    row("exit / q",     "Выход");
    std::cout << Color::CYAN << "  ╚════════════════════════════════════════════════════════╝\n\n" << Color::RESET;
}
 
void UI::printSuccess(const std::string& msg) {
    std::cout << Color::GREEN << Color::BOLD << "  ✔  " << Color::RESET << Color::GREEN << msg << Color::RESET << "\n";
}
void UI::printError(const std::string& msg) {
    std::cout << Color::RED << Color::BOLD << "  ✘  " << Color::RESET << Color::RED << msg << Color::RESET << "\n";
}
void UI::printWarning(const std::string& msg) {
    std::cout << Color::YELLOW << Color::BOLD << "  ⚠  " << Color::RESET << Color::YELLOW << msg << Color::RESET << "\n";
}
void UI::printInfo(const std::string& msg) {
    std::cout << Color::CYAN << Color::BOLD << "  ℹ  " << Color::RESET << msg << "\n";
}
 
std::string UI::prompt(const std::string& question) {
    std::cout << Color::BOLD << Color::MAGENTA << "\n  ❯ " << Color::RESET << question << " ";
    std::string input;
    std::getline(std::cin, input);
    // Убираем пробелы по краям
    size_t s = input.find_first_not_of(' ');
    size_t e = input.find_last_not_of(' ');
    if (s == std::string::npos) return "";
    return input.substr(s, e - s + 1);
}
 
bool UI::confirm(const std::string& question) {
    std::string ans = prompt(question + " [y/N]:");
    return (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'));
}
#ifndef UTILS_H
#define UTILS_H

#include <filesystem>
#include <optional>
#include <QString>

struct Note {
    QString BroaderText;
    QString SelectedText;
    QString MarkNote;
    QString Chapter;
    QString Created;
    QString Modified;
    QString Location;
};

struct Book {
    QString Title;
    QString Author;
};

std::vector<std::string> split(const std::string &str,
                               const std::string &delimiters) {
    std::vector<std::string> result;
    std::size_t pos, lastPos = 0;

    int i = 0;
    while ((pos = str.find_first_of(delimiters, lastPos)) != std::string::npos) {
        i++;
        if (i > 10) {
            break;
        }
        if (pos > lastPos) {
            result.push_back(str.substr(lastPos, pos - lastPos));
        }
        lastPos = pos + 1;
    }

    if (lastPos < str.length()) {
        result.push_back(str.substr(lastPos, std::string::npos));
    }

    return result;
}

bool compareByLocation(const Note &note1, const Note &note2) {
    auto l1 = split(note1.Location.toStdString(), "/,:");
    auto l2 = split(note2.Location.toStdString(), "/,:");

    size_t len = std::min(l1.size(), l2.size());
    for (size_t i = 0; i < len; i++) {
        try {
            int poc_1 = std::stoi(l1[i]);
            int poc_2 = std::stoi(l2[i]);
            if (poc_1 != poc_2) {
                return poc_1 < poc_2;
            }
        } catch (const std::exception &e) {
        }
    }

    return l1.size() < l2.size();
}

std::optional<std::string> findDatabaseFile(const std::string &directory) {
    // Manually expand the tilde '~' to the user's home directory
    std::string dirPathStr = directory;
    if (dirPathStr[0] == '~') {
        const char *homeDir = std::getenv("HOME");
        if (homeDir) {
            dirPathStr.replace(0, 1, homeDir);
        } else {
            // Return empty optional if the HOME environment variable is not set
            return std::nullopt;
        }
    }

    std::filesystem::path dirPath = std::filesystem::path(dirPathStr)
                                        .lexically_normal()
                                        .make_preferred()
                                        .lexically_normal();
    dirPath = dirPath.empty() ? std::filesystem::current_path() : dirPath;

    for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
        if (entry.path().extension() == ".sqlite") {
            return entry.path().string();
        }
    }
    return std::nullopt;
}

#endif // UTILS_H

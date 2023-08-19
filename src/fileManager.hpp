#pragma once

#include <fstream>

struct FileManager {
private:
    std::string fileName;
    std::ofstream file;
public:
    FileManager() = default;
    explicit FileManager(const std::string& fileName);
    void createFile();
    void writeToFile(const std::string& content);
};

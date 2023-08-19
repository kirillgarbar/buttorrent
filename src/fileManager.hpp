#pragma once

#include <fstream>

struct FileManager {
private:
    std::string fileName;
    std::ofstream file;
public:
    FileManager() = default;
    FileManager(const std::string& fileName, const std::string& outputPath);
    void createFile();
    void writeToFile(const std::string& content);
};

#include "fileManager.hpp"

using namespace std;

FileManager::FileManager(const string& fileName): fileName(fileName) {};

void FileManager::createFile() {
    file.open(fileName, std::ofstream::out | std::ofstream::trunc);
    file.close();
}

void FileManager::writeToFile(const string& content) {
    file.open(fileName, std::ofstream::out | std::ofstream::app);
    file << content;
    file.close();
}
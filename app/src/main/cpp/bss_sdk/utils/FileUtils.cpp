//
// Created by vnbk on 02/06/2023.
//
#include "FileUtils.h"

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#define CreateDir(path) mkdir(path.c_str())
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define GetCurrentDir getcwd
#define CreateDir(path) mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

std::vector<std::string> getFileInDir(const std::string &_pathToDir) {
    std::vector<std::string> fileList;
#ifdef WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    std::string dir = _pathToDir;
    dir += "/*";
    hFind = FindFirstFile(dir.c_str(), &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE){
        while (FindNextFile(hFind, &FindFileData)) {
            fileList.emplace_back(FindFileData.cFileName);
        }
        FindClose(hFind);
    }
#else
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_pathToDir.c_str())) != nullptr) {
        while ((ent = readdir (dir)) != nullptr) {
            fileList.emplace_back(ent->d_name);
        }
        closedir (dir);
    } else {
        perror ("");
    }
#endif

    return fileList;
}

bool checkFileExist(const std::string &_pathToDir, const std::string &_fileName) {
#ifdef WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    std::string fileName = _pathToDir;
    fileName += "/" + _fileName;
    hFind = FindFirstFile(fileName.c_str(), &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE){
        FindClose(hFind);
        return true;
    }
    return false;
#else
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_pathToDir.c_str())) != nullptr) {
        while ((ent = readdir (dir)) != nullptr) {
            if(_fileName == ent->d_name){
                closedir (dir);
                return true;
            }
        }
        closedir (dir);
    } else {
        perror ("");
    }
    return false;
#endif
}

bool createDir(const std::string &_name) {
    return CreateDir(_name) == 0;
}

std::string getCurrentDir() {
    char buff[FILENAME_MAX];
    GetCurrentDir(buff, FILENAME_MAX);

    return buff;
}
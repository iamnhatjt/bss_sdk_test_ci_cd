//
// Created by vnbk on 02/06/2023.
//

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <string>
#include <vector>

std::vector<std::string> getFileInDir(const std::string& _pathToDir);

bool checkFileExist(const std::string& _pathToDir,  const std::string& _fileName);

bool createDir(const std::string& _name);

std::string getCurrentDir();

#endif //FILEUTILS_H

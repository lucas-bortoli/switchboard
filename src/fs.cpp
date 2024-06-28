#include "fs.h"
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

// Retorna o diretório atual do processo.
// Fonte: Autoria própria
std::string get_working_directory()
{
    char cwdBuffer[512];
    getcwd(cwdBuffer, sizeof(cwdBuffer));

    return std::string(cwdBuffer);
}

std::vector<File> get_directory_files(std::string directoryPath)
{
    std::vector<File> files;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir == nullptr)
    {
        perror("opendir");
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string filePath = directoryPath + "/" + entry->d_name;

        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode))
        {
            File file;
            file.name = entry->d_name;
            file.size = fileStat.st_size;
            files.push_back(file);
        }
    }

    closedir(dir);
    return files;
}

bool file_exists(const std::string& absPath)
{
    return access(absPath.c_str(), F_OK) == 0;
}

std::string basename(const std::string& absPath)
{
    size_t lastSlash = absPath.find_last_of("/\\");
    if (lastSlash == std::string::npos)
    {
        return absPath; // No slash found, entire path is the base name
    }
    return absPath.substr(lastSlash + 1);
}
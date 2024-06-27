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
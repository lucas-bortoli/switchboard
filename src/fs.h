#pragma once
#include <string>
#include <vector>

struct File
{
    std::string name;
    size_t size;
};

// Retorna o diretório atual do processo.
// Fonte: Autoria própria
std::string get_working_directory();

std::vector<File> get_directory_files(std::string directoryPath);
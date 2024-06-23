#include "utils.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string plural(int count, std::string unitSingular, std::string unitPlural)
{
    if (count == 1)
        return unitSingular;
    return unitPlural;
}

// Divide uma string em partes separadas por um caractere delimitador.
// Fonte: https://stackoverflow.com/a/46931770
// Modificado para excluir itens vazios.
std::vector<std::string> split_by_char(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        if (item.size() < 1)
            continue;
        result.push_back(item);
    }

    return result;
}
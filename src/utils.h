#pragma once
#include <string>
#include <vector>

std::string plural(int count, std::string unitSingular, std::string unitPlural);

// Divide uma string em partes separadas por um caractere delimitador.
// Fonte: https://stackoverflow.com/a/46931770
// Modificado para excluir itens vazios.
std::vector<std::string> split_by_char(const std::string& s, char delim);
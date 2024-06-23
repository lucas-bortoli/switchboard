#include "commands.h"
#include "context.h"
#include "railroad/transport.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <sstream>
#include <string>
#include <vector>

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

Context* context;

int main(int argc, char** argv)
{
    context = new Context();
    context->localDirectory = std::string("./data");

    printf("Seja bem-vindo! Você pode fechar o programa apertando Ctrl+C em qualquer momento...\n\n");
    cmd_ajuda();

    while (true)
    {
        std::string prompt = "Switchboard $ ";
        if (context->server.has_value())
            prompt = "[servidor aberto  ] " + prompt;
        if (context->clientHandle.has_value())
            prompt = "[conexão remota ok] " + prompt;

        char* buf = readline(prompt.c_str());

        // EOF
        if (buf == nullptr)
            break;

        if (strlen(buf) > 0)
            add_history(buf);

        std::vector<std::string> tokens = split_by_char(std::string(buf), ' ');
        // A biblioteca readline faz malloc() em um buffer novo toda vez
        free(buf);

        if (tokens.size() < 1)
            continue;

        std::string command = tokens[0];
        transform(command.begin(), command.end(), command.begin(), ::toupper); // caixa alta

        if (command == "HOSPEDAR")
        {
            if (tokens.size() != 3)
            {
                printf("Uso: hospedar [ip] [porta]\n");
                continue;
            }

            std::string address = tokens[1];
            unsigned short port = std::stoi(tokens[2]);

            cmd_hospedar(address, port);
        }
        else if (command == "CONECTAR")
        {
            if (tokens.size() != 3)
            {
                printf("Uso: conectar [ip] [porta]\n");
                continue;
            }

            std::string address = tokens[1];
            unsigned short port = std::stoi(tokens[2]);

            cmd_conectar(address, port);
        }
        else if (command == "DOWNLOAD")
        {
            if (tokens.size() != 2)
            {
                printf("Uso: download [arquivo]\n");
                continue;
            }

            std::string target = tokens[1];

            cmd_download(target);
        }
        else if (command == "UPLOAD")
        {
            if (tokens.size() != 2)
            {
                printf("Uso: download [arquivo]\n");
                continue;
            }

            std::string target = tokens[1];

            cmd_upload(target);
        }
        else if (command == "RLIST")
        {
            cmd_rlist();
        }
        else if (command == "LLIST")
        {
            cmd_llist();
        }
        else if (command == "AJUDA")
        {
            cmd_ajuda();
        }
        else if (command == "SAIR" || command == "EXIT")
        {
            break;
        }
        else
        {
            printf("Comando desconhecido: %s\n", command.c_str());
        }
    }

    printf("Fechando Switchboard...\n");

    return 0;
}
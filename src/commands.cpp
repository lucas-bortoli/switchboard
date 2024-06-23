#include "commands.h"
#include "context.h"

using namespace std::chrono_literals;

bool check_if_client_is_connected()
{
    if (!context->clientHandle.has_value())
    {
        fprintf(stderr, "É preciso conectar a um servidor Switchboard para utilizar essa função.\n");
        return false;
    }

    return true;
}

void cmd_hospedar(const std::string& hostname, unsigned short port)
{
    printf("Iniciando servidor em %s:%d...\n", hostname.c_str(), port);
    context->startServer(hostname, port);
    std::this_thread::sleep_for(300ms); // delay proposital (não há race conditions)
    printf("Servidor iniciado.\n");
}

void cmd_conectar(const std::string& hostname, unsigned short port)
{
    printf("Conectando ao servidor %s:%d...\n", hostname.c_str(), port);
    context->connectClient(hostname, port);
    std::this_thread::sleep_for(300ms); // delay proposital (não há race conditions)
    printf("Conectado.\n");             // TODO verificar se conectou realmente
}

void cmd_download(const std::string& filename)
{
    if (!check_if_client_is_connected())
        return;

    fprintf(stderr, "cmd_download: não implementado\n");
}

void cmd_upload(const std::string& filename)
{
    if (!check_if_client_is_connected())
        return;

    fprintf(stderr, "cmd_upload: não implementado\n");
}

void cmd_rlist()
{
    if (!check_if_client_is_connected())
        return;

    fprintf(stderr, "cmd_rlist: não implementado\n");
}

void cmd_llist()
{
    fprintf(stderr, "cmd_llist: não implementado\n");
}

void cmd_ajuda()
{
    printf("Instruções de uso:\n");
    printf("* hospedar [ip] [porta]   Hospeda um servidor Switchboard, que pode ser conectado em outra máquina\n");
    printf("* conectar [ip] [porta]   Conecta a um servidor Switchboard remoto\n");
    printf("* download [arquivo]      Baixa um arquivo da pasta remota para a pasta local\n");
    printf("* upload   [arquivo]      Envia um arquivo da pasta local para a pasta remota\n");
    printf("* rlist                   Lista os arquivos da pasta remota\n");
    printf("* llist                   Lista os arquivos da pasta local\n");
    printf("* ajuda                   Exibe os comandos disponíveis\n");
    printf("* sair|exit               Finaliza a aplicação\n");
    printf("\n");
}
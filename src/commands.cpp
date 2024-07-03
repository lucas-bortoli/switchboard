#include "commands.h"
#include "context.h"
#include "fs.h"
#include "protocol.h"
#include <cstdio>
#include <cstring>

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
    if (context->server.has_value())
    {
        fprintf(stderr, "O servidor já está em execução.\n");
        return;
    }

    printf("Iniciando servidor em %s:%d...\n", hostname.c_str(), port);
    context->startServer(hostname, port);
    std::this_thread::sleep_for(300ms); // delay proposital (não há race conditions)
    printf("Servidor iniciado.\n");
}

void cmd_conectar(const std::string& hostname, unsigned short port)
{
    if (context->clientHandle.has_value())
    {
        fprintf(stderr, "Já está conectado a um servidor.\n");
        return;
    }

    printf("Conectando ao servidor %s:%d...\n", hostname.c_str(), port);
    context->connectClient(hostname, port);
    std::this_thread::sleep_for(300ms); // delay proposital (não há race conditions)
    printf("Conectado.\n");             // TODO verificar se conectou realmente
}

void cmd_download(const std::string& filename)
{
    if (!check_if_client_is_connected())
        return;

    auto localTargetPath = context->localDirectory + "/" + filename;

    if (file_exists(localTargetPath))
        fprintf(stderr, "Arquivo local já existe, será sobrescrito!\n");

    std::string fileBasename = basename(localTargetPath);
    DownloadRequest request = DownloadRequest{
        .kind = ProtocolMessageKind::DOWNLOAD_REQUEST,
        .name = {0},
    };
    std::memcpy(request.name, fileBasename.c_str(), std::min(fileBasename.size(), sizeof(request.name)));
    rr_client_send(context->clientHandle.value(), (const char*)&request, sizeof(request));

    char blob[FRAME_BODY_LENGTH];
    size_t recvSize = rr_client_receive(context->clientHandle.value(), blob, sizeof(blob));

    if ((ProtocolMessageKind)blob[0] == ProtocolMessageKind::DOWNLOAD_RESPONSE_DENIED)
    {
        DownloadResponseDenied resp;
        std::memcpy(&resp, blob, recvSize);
        fprintf(stderr, "O servidor rejeitou o download do arquivo: %s\n", resp.message);
        return;
    }

    if ((ProtocolMessageKind)blob[0] == ProtocolMessageKind::IMMINENT_FILE_TRANSFER)
    {
        auto startTime = std::chrono::steady_clock::now();

        ImminentFileTransfer fileTransfer;
        std::memcpy(&fileTransfer, blob, recvSize);

        FILE* fh = fopen(localTargetPath.c_str(), "wb");

        size_t receivedBytes = 0;
        for (size_t i = 0; i < fileTransfer.chunkCount; i++)
        {
            recvSize = rr_client_receive(context->clientHandle.value(), blob, sizeof(blob));
            receivedBytes += recvSize;

            double progress = ((double)receivedBytes / fileTransfer.size) * 100;
            printf("Recebendo... %zu / %zu bytes, chunk #%zu, %.2f%%\n", receivedBytes, fileTransfer.size, i, progress);

            fwrite(blob, recvSize, 1, fh);
        }

        fclose(fh);

        auto endTime = std::chrono::steady_clock::now();

        // Diferença de tempo entre início e final
        auto elapsed = endTime - startTime;
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

        int hours = elapsed_seconds / 3600;
        elapsed_seconds %= 3600;
        int minutes = elapsed_seconds / 60;
        int seconds = elapsed_seconds % 60;

        printf("Transferência levou %d horas, %d minutos e %d segundos.\n", hours, minutes, seconds);
    }
}

void cmd_upload(const std::string& filename)
{
    if (!check_if_client_is_connected())
        return;

    auto localPath = context->localDirectory + "/" + filename;

    if (!file_exists(localPath))
    {
        fprintf(stderr, "Arquivo não existe.");
        return;
    }

    std::string fileBasename = basename(filename);

    FILE* fh = fopen(localPath.c_str(), "rb");

    // Determinar o tamanho do arquivo movendo o ponteiro para o fim do arquivo, determinando o offset, e movendo para o
    // começo https://stackoverflow.com/a/238607
    fseek(fh, 0L, SEEK_END);
    size_t fileSize = ftell(fh);
    rewind(fh);

    // Serão enviados tantos pacotes de dados
    size_t chunkCount = (fileSize / FRAME_BODY_LENGTH) + 1;

    // Enviar mensagem falando que há uma transmissão iminente, de forma que todos os quadros serão fragmentos dos
    // arquivos
    auto request = ImminentFileTransfer{
        .kind = ProtocolMessageKind::IMMINENT_FILE_TRANSFER,
        .size = fileSize,
        .chunkCount = chunkCount,
        .name = {0},
    };
    std::memcpy(request.name, fileBasename.c_str(), std::min(fileBasename.size(), sizeof(request.name)));
    rr_client_send(context->clientHandle.value(), (const char*)&request, sizeof(request));

    auto startTime = std::chrono::steady_clock::now();

    size_t sentBytes = 0;
    for (size_t i = 0; i < chunkCount; i++)
    {
        char blob[FRAME_BODY_LENGTH];
        size_t blobSize = fread(blob, 1, sizeof(blob), fh);

        rr_client_send(context->clientHandle.value(), blob, blobSize);

        sentBytes += blobSize;

        double progress = ((double)sentBytes / fileSize) * 100;
        printf("Enviando... %zu / %zu bytes, chunk #%zu, %.2f%%\n", sentBytes, fileSize, i, progress);
    }

    fclose(fh);

    auto endTime = std::chrono::steady_clock::now();

    // Diferença de tempo entre início e final
    auto elapsed = endTime - startTime;
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

    int hours = elapsed_seconds / 3600;
    elapsed_seconds %= 3600;
    int minutes = elapsed_seconds / 60;
    int seconds = elapsed_seconds % 60;

    printf("Transferência levou %d horas, %d minutos e %d segundos.\n", hours, minutes, seconds);
}

void cmd_rlist()
{
    if (!check_if_client_is_connected())
        return;

    printf("Listando diretório remoto...\n");

    auto request = RemoteListRequest{.kind = ProtocolMessageKind::REMOTE_LIST_FOLDER_REQUEST};
    rr_client_send(context->clientHandle.value(), (const char*)&request, sizeof(request));

    RemoteListResponseStart res;
    rr_client_receive(context->clientHandle.value(), (char*)&res, sizeof(res));

    printf("(remoto) %s\n", res.remotePath);
    for (size_t i = 0; i < res.itemCount; i++)
    {
        RemoteListResponseItem res;
        rr_client_receive(context->clientHandle.value(), (char*)&res, sizeof(res));
        printf("  %24s FILE %8zu bytes\n", res.name, res.size);
    }

    printf("%d itens\n", (int)res.itemCount);
}

void cmd_llist()
{
    printf("Listando diretório local...\n");
    auto items = get_directory_files(context->localDirectory);

    printf("(local) %s\n", context->localDirectory.c_str());
    for (auto item : items)
    {
        printf("  %24s FILE %8zu bytes\n", item.name.c_str(), item.size);
    }

    printf("%d itens\n", (int)items.size());
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
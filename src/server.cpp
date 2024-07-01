#include "server.h"
#include "context.h"
#include "fs.h"
#include "protocol.h"
#include "railroad/transport.h"
#include <cstring>

void sb_server_client_thread_loop(rr_server_handle serverHandle, rr_sock_handle clientHandle)
{

    printf("sb_server_client_thread_loop: Iniciando\n");

    while (true)
    {
        char frameBody[FRAME_BODY_LENGTH];

        rr_server_receive(serverHandle, clientHandle, frameBody, sizeof(frameBody));

        switch ((ProtocolMessageKind)frameBody[0])
        {
            case ProtocolMessageKind::REMOTE_LIST_FOLDER_REQUEST: {
                printf("sb_server_client_thread_loop: enviando listagem para cliente\n");

                auto items = get_directory_files(context->localDirectory);

                auto response = RemoteListResponseStart{.itemCount = items.size(), .remotePath = {0}};
                std::memcpy(response.remotePath, context->localDirectory.c_str(),
                            std::min(context->localDirectory.size(), sizeof(response.remotePath)));
                rr_server_send(serverHandle, clientHandle, (const char*)&response, sizeof(response));

                for (auto item : items)
                {
                    auto response = RemoteListResponseItem{.size = item.size, .name = {0}};
                    std::memcpy(response.name, item.name.c_str(), std::min(item.name.size(), sizeof(response.name)));
                    rr_server_send(serverHandle, clientHandle, (const char*)&response, sizeof(response));
                }

                break;
            }
            case ProtocolMessageKind::IMMINENT_FILE_TRANSFER: {
                printf("sb_server_client_thread_loop: recebendo arquivo\n");

                ImminentFileTransfer transfer = *(ImminentFileTransfer*)&frameBody;

                std::string targetFile = context->localDirectory + "/" + transfer.name;
                FILE* fh = fopen(targetFile.c_str(), "wb");

                size_t receivedBytes = 0;
                for (size_t i = 0; i < transfer.chunkCount; i++)
                {
                    char blob[FRAME_BODY_LENGTH];
                    size_t blobSize = rr_server_receive(serverHandle, clientHandle, blob, sizeof(blob));
                    fwrite(blob, blobSize, 1, fh);
                    receivedBytes += blobSize;
                    printf("Recebido %zu / %zu bytes, chunk #%zu, tamanho chunk %zu\n", receivedBytes, transfer.size, i,
                           blobSize);
                }

                fclose(fh);

                break;
            }
            case ProtocolMessageKind::DOWNLOAD_REQUEST: {
                DownloadRequest req;
                std::memcpy(&req, frameBody, sizeof(req));

                printf("sb_server_client_thread_loop: cliente pediu um arquivo: %s\n", req.name);

                std::string localPath = context->localDirectory + "/" + std::string(req.name);

                if (!file_exists(localPath))
                {
                    fprintf(stderr, "Arquivo não existe\n");
                    auto response = DownloadResponseDenied{
                        .kind = ProtocolMessageKind::DOWNLOAD_RESPONSE_DENIED,
                        .message = {0},
                    };
                    std::string message = "Arquivo não existe no servidor.";
                    std::memcpy(response.message, message.c_str(), message.size());
                    rr_server_send(serverHandle, clientHandle, (const char*)&response, sizeof(response));
                    break;
                }

                FILE* fh = fopen(localPath.c_str(), "rb");

                // Determinar o tamanho do arquivo movendo o ponteiro para o fim do arquivo, determinando o offset, e
                // movendo para o começo https://stackoverflow.com/a/238607
                fseek(fh, 0L, SEEK_END);
                size_t fileSize = ftell(fh);
                rewind(fh);

                // Serão enviados tantos pacotes de dados
                size_t chunkCount = (fileSize / FRAME_BODY_LENGTH) + 1;

                // Enviar mensagem falando que há uma transmissão iminente, de forma que todos os quadros serão
                // fragmentos dos arquivos
                auto fileTransferResponse = ImminentFileTransfer{
                    .kind = ProtocolMessageKind::IMMINENT_FILE_TRANSFER,
                    .size = fileSize,
                    .chunkCount = chunkCount,
                    .name = {0},
                };
                std::string fileBasename = basename(localPath);
                std::memcpy(fileTransferResponse.name, fileBasename.c_str(),
                            std::min(fileBasename.size(), sizeof(fileTransferResponse.name)));
                rr_server_send(serverHandle, clientHandle, (const char*)&fileTransferResponse,
                               sizeof(fileTransferResponse));

                size_t sentBytes = 0;
                for (size_t i = 0; i < chunkCount; i++)
                {
                    char blob[FRAME_BODY_LENGTH];
                    size_t blobSize = fread(blob, 1, sizeof(blob), fh);

                    rr_server_send(serverHandle, clientHandle, blob, blobSize);

                    sentBytes += blobSize;
                    printf("Enviando... %zu / %zu bytes, chunk #%zu, tamanho chunk %zu\n", sentBytes, fileSize, i,
                           blobSize);
                }

                printf("Enviado.\n");
                fclose(fh);

                break;
            }
            default:
                fprintf(stderr, "sb_server_client_thread_loop: mensagem com ID inválido: %d\n", frameBody[0]);
                break;
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    printf("sb_server_client_thread_loop: Finalizando\n");
}

void sb_server_thread_loop()
{
    using namespace std::chrono_literals;

    while (true)
    {
        if (!context->server.has_value())
        {
            break;
        }

        auto server = context->server.value();

        rr_sock_handle newClient = rr_server_accept_client(server->handle);
        printf("sb_server_thread_loop: Novo cliente (%lu)\n", newClient);

        server->clientsLock.lock();
        server->clients.push_back(newClient);
        std::thread clientThread = std::thread(&sb_server_client_thread_loop, server->handle, newClient);
        clientThread.detach();
        server->clientsLock.unlock();

        std::this_thread::sleep_for(1ms);
    }

    printf("sb_server_thread_loop: Finalizando\n");
}
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
        char request[FRAME_BODY_LENGTH];

        rr_server_receive(serverHandle, clientHandle, request, sizeof(request));

        switch ((ProtocolMessageKind)request[0])
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
            default:
                fprintf(stderr, "sb_server_client_thread_loop: mensagem com ID inválido: %d\n", request[0]);
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
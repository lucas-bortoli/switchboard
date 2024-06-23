#include "server.h"
#include "context.h"
#include "railroad/transport.h"

void sb_server_client_thread_loop(rr_server_handle serverHandle, rr_sock_handle clientHandle)
{
    using namespace std::chrono_literals;

    printf("sb_server_client_thread_loop: Iniciando\n");

    while (true)
    {
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
#include "context.h"
#include "server.h"
#include <cstdio>
#include <railroad/transport.h>
#include <string>
#include <thread>

Context::~Context()
{
    // Fechar cliente
    if (this->clientHandle.has_value())
    {
        rr_client_close(this->clientHandle.value());
        this->clientHandle.reset();
    }

    // Fechar servidor
    if (this->server.has_value())
    {
        auto server = this->server.value();
        rr_server_close(server->handle);
        delete server->thread;
        delete server;
        this->server.reset();
    }
}

void Context::startServer(std::string address, unsigned short port)
{
    if (this->server.has_value())
    {
        fprintf(stderr, "Context::startServer: servidor já existe, ignorando...\n");
        return;
    }

    auto server = new ServerContext();
    server->handle = rr_server_bind(address, port);
    server->thread = new std::thread(&sb_server_thread_loop);
    server->thread->detach();
    this->server = server;
}

void Context::connectClient(std::string address, unsigned short port)
{
    if (this->clientHandle.has_value())
    {
        fprintf(stderr, "Context::connectClient: já conectado a um servidor\n");
        return;
    }

    this->clientHandle = rr_client_connect(address, port);
}
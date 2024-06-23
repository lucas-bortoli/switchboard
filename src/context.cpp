#include "context.h"
#include <cstdio>
#include <railroad/transport.h>
#include <string>

Context::~Context()
{
    // Fechar servidor
    if (this->server.has_value())
    {
        auto server = this->server.value();
        rr_server_close(server->handle);
        delete this->server.value();
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
    this->server = server;
}
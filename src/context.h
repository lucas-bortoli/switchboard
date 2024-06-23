#pragma once

#include "railroad/transport.h"
#include <optional>
#include <string>
#include <unordered_map>

class ServerContext
{
  public:
    rr_server_handle handle;
    std::thread* thread;
    std::vector<rr_sock_handle> clients;
};

// Representa o estado atual da aplicação
class Context
{
  public:
    std::string localDirectory;
    std::optional<ServerContext*> server;
    std::optional<rr_sock_handle> clientHandle;
    ~Context();
    void startServer(std::string address, unsigned short port);
};

// Contexto global da aplicação, instanciado no main.cpp
extern Context* context;
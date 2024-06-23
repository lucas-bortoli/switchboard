#pragma once
#include <string>

void cmd_hospedar(const std::string& hostname, unsigned short port);
void cmd_conectar(const std::string& hostname, unsigned short port);
void cmd_download(const std::string& filename);
void cmd_upload(const std::string& filename);
void cmd_rlist();
void cmd_llist();
void cmd_ajuda();

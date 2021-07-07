#pragma once
#include <string>

struct IOController
{
    static uint32_t clientPort;
    static uint32_t serverPort;

    static uint32_t valread;
    static std::string serverMsg;

    static void clientSend(const std::string& data);
    static void serverListen(uint32_t newSocket);
};
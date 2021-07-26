#pragma once
#include <string>

struct IOController
{
    static uint32_t clientPort;
    static uint32_t serverPort;

    static std::string serverMessage;

    static void clientListen(uint32_t newSocket);
    static void clientSend(const std::string& data);
    static void serverListen(uint32_t newSocket);
    static void serverSend(const std::string& data);
};
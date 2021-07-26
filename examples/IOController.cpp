#include "IOController.h"
#include "ConsoleVariableSystem.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

#define CLIENTPORT 8080
#define SERVERPORT 8000

uint32_t IOController::clientPort = CLIENTPORT;
uint32_t IOController::serverPort = SERVERPORT;
std::string IOController::serverMessage = "{}";

void IOController::clientListen(uint32_t newSocket)
{
    char buffer[1024] { 0 };
    uint32_t valread = read(newSocket, buffer, 1024);

    std::string message { buffer };

    if (message == "stop_alarm")
    {
        // pass
    }

    else if (message == "reset")
    {
        ConsoleVariableSystem::get()->reset();
    }

    else if (auto pos = message.find(' '))
    {
        std::string key = message.substr(0, pos);
        std::string value = message.substr(pos, valread);

        ConsoleVariableSystem::get()->setFloatVariableCurrentByHash(key, value);
    }

    close(newSocket);

    std::cout << buffer << std::endl;
}

void IOController::clientSend(const std::string& data)
{
    uint32_t sock = 0, valread;
    struct sockaddr_in servAddr;
    
    // Creating socket file descriptor
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == 0)
    {
        throw std::runtime_error("Socket creation error!");
    }
   
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(IOController::serverPort);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) 
    {
        throw std::runtime_error("Invalid address!");
    }
   
    if (connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    {
        throw std::runtime_error("Connection failed!");
    }

    send(sock, data.data(), data.size(), 0);

    close(sock);
}

void IOController::serverListen(uint32_t newSocket)
{
    char buffer[1024] { 0 };
    uint32_t valread = read(newSocket, buffer, 1024);

    std::string message { buffer };

    // not ios message
    if (message[0] == '{')
    {
        serverMessage = buffer;
    }

    if (message == "get_location")
    {
        send(newSocket, serverMessage.c_str(), serverMessage.length(), 0);
        std::string serverMessage = "{}";
    }

    else if (message == "stop_alarm" || message == "reset")
    {
        serverSend(message);
    }

    else if (auto pos = message.find(' '))
    {
        serverSend(message);
    }

    close(newSocket);

    std::cout << buffer << std::endl;
}

void IOController::serverSend(const std::string& data)
{
    uint32_t sock = 0, valread;
    struct sockaddr_in servAddr;
    
    // Creating socket file descriptor
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == 0)
    {
        throw std::runtime_error("Socket creation error!");
    }
   
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(IOController::clientPort);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) 
    {
        throw std::runtime_error("Invalid address!");
    }
   
    if (connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    {
        throw std::runtime_error("Connection failed!");
    }

    send(sock, data.data(), data.size(), 0);

    close(sock);
}
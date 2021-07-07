#include "IOController.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

#define CLIENTPORT 8000
#define SERVERPORT 8000

uint32_t IOController::clientPort = CLIENTPORT;
uint32_t IOController::serverPort = SERVERPORT;
uint32_t IOController::valread = 0;
std::string IOController::serverMsg = "";

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
    valread += read(newSocket, buffer, 1024);

    if (buffer[0] == '{')
    {
        serverMsg += buffer;
    }
    else if (buffer[0] == '1')
    {
        if (serverMsg.size() == 0)
        {
            send(newSocket, "{}", 2, 0);
        }
        else
        {
            send(newSocket, &serverMsg, valread, 0);
        }
        serverMsg = "";
        valread = 0;
    }
    std::cout << buffer << std::endl;
}
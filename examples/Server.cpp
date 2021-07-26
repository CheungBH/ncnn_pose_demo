#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <future>
#include <iostream>
#include <map>

#include "IOController.h"
#include "TimeLocBbox.h"

int main()
{
    uint32_t serverFd = 0, newSocket, valRead;
    struct sockaddr_in address;
    uint32_t opt = 1;
    uint32_t addrlen = sizeof(address);

    // Creating socket file descriptor
	serverFd = socket(AF_INET, SOCK_STREAM, 0);

	if (serverFd == 0)
	{
		std::cerr << "Failed to create socket!" << std::endl;
		return 1;
	}

	std::cout << "Created socket!" << std::endl;

    // Forcefully attaching socket to the port 8080
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cerr << "Failed to create socket!" << std::endl;
		return 1;
    }

    address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(IOController::serverPort);

	if (bind(serverFd, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		std::cerr << "Socket bind error." << std::endl;
		return 1;
	}

	std::cout << "Socket binded." << std::endl;

	if (listen(serverFd , 8) < 0)
    {
        std::cerr << "Socket listen error." << std::endl;
		return 1;
    }

	while (newSocket = accept(serverFd, (struct sockaddr*) &address, (socklen_t*) &addrlen))
	{
        auto worker = std::async(std::launch::async, [=]()
        {
            IOController::serverListen(newSocket);
        });
	}

    close(serverFd);

    return 0;
}
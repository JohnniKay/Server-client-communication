#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Server.h"
#include "Client.h"
#include "ClientStruct.h"
#include "Definitions.h"

void ClientCreation();
void ServerCreation();

int wsaError = -1;

SOCKET communicationSocket;

int main()
{
	WSADATA wsadata;
	wsaError = WSAStartup(WINSOCK_VERSION, &wsadata);

	int userChoice;
	
	do
	{
		printf("Would you like to make a Server or a Client?\n");
		printf("1: Server\n");
		printf("2: Client\n");
		printf("3: Quit\n");
		std::cin >> userChoice;

	} while (userChoice != 1 && userChoice != 2 && userChoice != 3);

	if (userChoice == 1)
	{
		ServerCreation();
	}
	if (userChoice == 2)
	{
		ClientCreation();
	}
	if (userChoice == 3)
	{
		return SHUTDOWN;
	}

	return WSACleanup();
}

void ClientCreation()
{
	Client newClient;

	printf("\nAttempting Connection...\n\n");

	wsaError = newClient.init();
}

void ServerCreation()
{
	Server newServer;
	
	newServer.init();  // Goes to Server.cpp
	
}


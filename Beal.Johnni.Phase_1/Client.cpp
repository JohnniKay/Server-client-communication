#include "Client.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <iostream>
#include "Server.h"
#include <string>
#include <fstream>

SOCKET userSocket;
//SOCKET bcReceiverSocket;
sockaddr_in userAddress;

int wsaError2 = -1;

int Client::init()
{
	SOCKET bcReceiverSocket;
	u_short UDPPort = 31337;

	bcReceiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (bcReceiverSocket == INVALID_SOCKET)
	{
		wsaError2 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError2 << "\n";
		return SETUP_ERROR;
	}
	int optVal = 1;
	setsockopt(bcReceiverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));
	sockaddr_in bcReceiveAddr;
	bcReceiveAddr.sin_family = AF_INET;
	bcReceiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bcReceiveAddr.sin_port = htons(UDPPort);

	int result = bind(bcReceiverSocket, (sockaddr*)&bcReceiveAddr, sizeof(bcReceiveAddr));
	if (result == SOCKET_ERROR)
	{
		wsaError2 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError2 << "\n";
		return BIND_ERROR;                                            //Checking to see if there was a error when binding to the socket
	}

	const int bufferLength = 14;
	char buffer[bufferLength];
	int addressSize = sizeof(bcReceiveAddr);

	result = recvfrom(bcReceiverSocket, (char*)buffer, bufferLength, 0,
			(sockaddr*)&bcReceiveAddr, &addressSize);
	if (result == SOCKET_ERROR)
	{
		wsaError2 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError2 << "\n";
		return MESSAGE_ERROR;
	}
	
	std::string bufferInfo;

	for (int i = 0; i < bufferLength; i++)
	{
		bufferInfo = bufferInfo + buffer[i];
	}

	std::string IpAddress = bufferInfo.substr(0, 9);

	const char* charAddress = IpAddress.c_str();
	std::cout << charAddress;

	std::string portNumber = bufferInfo.substr(9, 14);

	const char* charPort = portNumber.c_str();
	std::cout << portNumber;

	userSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                             //Create Socket
	if (userSocket == INVALID_SOCKET)
	{
		wsaError2 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError2 << "Error while creating socket\n";	//Error Checking Socket
		return SETUP_ERROR;
	}
	userAddress.sin_family = AF_INET;                                                   //Converting address/port into socl_addr structure
	userAddress.sin_port = htons(UDPPort);                                                
	userAddress.sin_addr.S_un.S_addr = inet_addr(charAddress);

	result = connect(userSocket, (SOCKADDR*) &userAddress, sizeof(userAddress));    //Connect to server	
	if (result == SOCKET_ERROR)
	{
		wsaError2 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError2 << "Connection Refused\n";         //Error Checking the Connection 
		return CONNECT_ERROR;
	}
	else if (result == 0)
	{
		SHUTDOWN;
	}
	else
	{
		printf("Connection Successful!!!\n\n");
	}
	if (wsaError2 != 0)
	{
		printf("Make sure server is running\n\n");
	}

	Client newClient;
	uint8_t size = 4069;
	char sendBuffer[30];
	std::string sendBufferString = " ";
	const char* $register = "$register";
	const char* $exit = "$exit";
	
		std::cout << "Enter Client name: ";
		std::cin >> newClient.name;
		
	do
	{
		std::cout << "\n($register)\n";
		std::cout << "\n"<< newClient.name << ": ";
		std::cin >> sendBuffer;
		sendBufferString = sendBuffer;

	} while (sendBufferString != $register);

	Server serverSize;
	
		if (sendBufferString == "$register")
		{
			sendBufferString.append(" " + newClient.name);

			const int stringLength = sendBufferString.length();

			strcpy_s(sendBuffer, sendBufferString.c_str());

			result = newClient.tcp_send_whole(userSocket, (char*)&size, 1);
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";		
				return MESSAGE_ERROR;
			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}

			result = tcp_send_whole(userSocket, sendBuffer, size);
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return MESSAGE_ERROR;
			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}
			else
			{
				printf("\nDEBUG: Registration information was sent to Server\n");
			}

			uint8_t size = 0;

			result = tcp_recv_whole(userSocket, (char*)&size, 1);            // Receiving message from server
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return MESSAGE_ERROR;
			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}

			char* buffer = new char[size];

			result = tcp_recv_whole(userSocket, (char*)buffer, size);
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return MESSAGE_ERROR;

			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}
			else
			{
				std::string messageBuffer = buffer;
				std::cout << messageBuffer;
			}

			//std::cout << "\nUserName: " << newClient.name << " Port: " << port << "\n";
			std::cout << "Special Actions: $getlist, $getlog, $exit\n\n";
		}
		else if (sendBufferString == "$exit")
		{
			newClient.stop();
		}
	

	bool connected = true;
	do
	{
		std::string messageBuffer;
		//int messageSize = 0;
		int bufferSize = 4069;


		std::cout << "\n\nSend Message: ";
		std::getline(std::cin, sendBufferString);
		//std::cin >> sendBufferString;
		//std::cout << "\n";

		const char* inputBuffer = sendBufferString.c_str();

		if (sendBufferString.size() > 0)
		{
			result = tcp_send_whole(userSocket, (char*)&size, 1);
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return MESSAGE_ERROR;
			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}

			result = tcp_send_whole(userSocket, inputBuffer, size);
			if ((result == SOCKET_ERROR))
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return MESSAGE_ERROR;
			}
			else if (result == 0)
			{
				wsaError2 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError2 << "n";
				return SHUTDOWN;
			}
			else
			{
				printf("DEBUG: Command sent to Server..\n");
			}

			if (sendBufferString.substr(0, 9) == "$getlog")
			{


				uint8_t size = 0;

				result = tcp_recv_whole(userSocket, (char*)&size, 1);            // Receiving message size from server
				if ((result == SOCKET_ERROR))
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return MESSAGE_ERROR;
				}
				else if (result == 0)
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return SHUTDOWN;
				}

				char* buffer = new char[size];

				result = tcp_recv_whole(userSocket, (char*)buffer, size);
				if ((result == SOCKET_ERROR))
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return MESSAGE_ERROR;

				}
				else if (result == 0)
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return SHUTDOWN;
				}

				int incomingFileSize = result;

				std::fstream newFile;
				newFile.open("ClientFile.txt");
				if (newFile.is_open())
				{
					std::string logMessage = "~CLIENT LOG~\n";
					newFile << logMessage;
					newFile.close();
				}

				newFile.open("ClientFile.txt", std::ios::app);
				if (newFile.is_open())
				{

					size = 0;

					result = tcp_recv_whole(userSocket, (char*)&size, 1);            // Receiving message from server
					if ((result == SOCKET_ERROR))
					{
						wsaError2 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError2 << "n";
						return MESSAGE_ERROR;
					}
					else if (result == 0)
					{
						wsaError2 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError2 << "n";
						return SHUTDOWN;
					}

					int incomingSize = 0;

					buffer = new char[size];
					//Issue might be buffer size isnt right. Check Client log after getlog

					std::string endBuffer = "END";

					do
					{
						result = tcp_recv_whole(userSocket, (char*)buffer, size);
						if ((result == SOCKET_ERROR))
						{
							wsaError2 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError2 << "n";
							return MESSAGE_ERROR;

						}
						else if (result == 0)
						{
							wsaError2 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError2 << "n";
							return SHUTDOWN;
						}
						else
						{
							std::string logString = buffer;
							newFile << logString << std::endl;

						}

					} while (buffer != endBuffer);
				}
				newFile.close();
			}
			else if (sendBufferString.substr(0, 5) == "$exit")
			{
				return SHUTDOWN;
			}
			else
			{
				size = 0;

				result = tcp_recv_whole(userSocket, (char*)&size, 1);            // Receiving message from server
				if ((result == SOCKET_ERROR))
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return MESSAGE_ERROR;
				}
				else if (result == 0)
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return SHUTDOWN;
				}

				char* buffer = new char[size];
				printf("\nEcho: ");

				result = tcp_recv_whole(userSocket, (char*)buffer, size);
				if ((result == SOCKET_ERROR))
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return MESSAGE_ERROR;

				}
				else if (result == 0)
				{
					wsaError2 = WSAGetLastError();
					std::cout << "Error Code: " << wsaError2 << "n";
					return SHUTDOWN;
				}
				else
				{
					std::string messageBuffer = buffer;
					std::cout << messageBuffer;
				}
			}
		}
			
	} while (connected);
}

int Client::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;
	
		do
		{
			int ret = recv(s, buf + total, len - total, 0);
			if (ret < 1)
				return ret;
			else
				total += ret;

		} while (total < len);

	return total;
}

int Client::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

void Client::stop()
{
	shutdown(userSocket, SD_BOTH);
}
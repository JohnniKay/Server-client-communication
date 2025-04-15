#include "Server.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <iostream>
#include <vector>
#include "ClientStruct.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>

SOCKET listenSocket;
SOCKET newClientSocket ;
SOCKET bcSenderSocket ;

int wsaError3 = -1;

int Server::init()
{
	std::string address;
	
	int portNumber = 31337;
	bool invalidAddress = true;
	bool invalidPort = true;

	do
	{
		std::cout << "Enter Servers IP Address: ";                                               //User Enters IP Address
		std::cin >> address;

		char testBuffer[4]{};
		int testAddress = inet_pton(AF_INET, (char*)&address[0], testBuffer);

		if (testAddress <= 0)
		{
			wsaError3 = WSAGetLastError();

			if (testAddress == -1)
			{
				std::cout << "Error Code: " << wsaError3 << ": Address is Invalid\n";
			}
			else if (testAddress == 0)
			{
				std::cout << "Error Code: " << wsaError3 << ": Address is not in the right format\n";
			}
			else
			{
				std::cout << "Error Code: " << wsaError3 << ": Unknown Error\n";
			}
		}
		else
		{
			invalidAddress = false;
		}

	} while (invalidAddress);

	std::cout << "Enter Servers Port Number: ";                                             //User enters Port Number
	std::cin >> portNumber;

	std::string portString = std::to_string(portNumber);

	const char* charAddress = address.c_str();

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(charAddress);                      //Converting address and port into socl_addr structure
	serverAddr.sin_port = htons(portNumber);

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);           // Creating socket
	if (listenSocket == INVALID_SOCKET)
	{
		wsaError3 = WSAGetLastError();
		return SETUP_ERROR;                                            // Checking to see if socket was created successfully
	}

	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		wsaError3 = WSAGetLastError();
		std::cout << "Error Code: " << wsaError3 << "\n";
		return BIND_ERROR;                                            //Checking to see if there was a error when binding to the socket
	}

	newClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newClientSocket == INVALID_SOCKET)
	{
		wsaError3 = WSAGetLastError();
		return SETUP_ERROR;
	}
	
	//Broadcast Socket Setup
	bcSenderSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (bcSenderSocket == INVALID_SOCKET)
	{
		wsaError3 = WSAGetLastError();
		return SETUP_ERROR;
	}
	int optVal = 1;
	setsockopt(bcSenderSocket, SOL_SOCKET, SO_BROADCAST, (char*) & optVal, sizeof(optVal));
	sockaddr_in bcSendAddr;
	bcSendAddr.sin_family = AF_INET;
	bcSendAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	bcSendAddr.sin_port = htons(portNumber);

	std::string broadCast = address + portString;

	Server currentServer;
	
		result = listen(listenSocket, 1);
		if (result == SOCKET_ERROR)
		{
			wsaError3 = WSAGetLastError();
			std::cout << "Error Code: " << wsaError3 << "\n";
			return SETUP_ERROR;                                          //Checking to see if socket was created successfully
		}


		fd_set masterSet{}, readySet{};
		FD_ZERO(&masterSet);											//Creating the sets
		FD_SET(listenSocket, &masterSet);

		bool activeServer = true;
		std::string messageBuffer;
		int messageSize = 0;
		struct timeval timeout;
		timeout.tv_sec = 1;

		std::vector<SOCKET> clientSocketList;
		std::vector<std::string> clientNameList;

		std::fstream outfile;
		outfile.open("ServerComm.txt", std::ios::out);
		if (outfile.is_open())
		{
			std::string logMessage = "SERVER LOG\n";
			outfile << logMessage;
			outfile.close();
		}

		do
		{
			FD_ZERO(&readySet);
			readySet = masterSet;

			int socketCount = select(0, &readySet, NULL, NULL, &timeout);                        //Select method

			//I may need to convert broadcast message to char before sending
			int addressSize = sizeof(bcSendAddr);

			result = sendto(bcSenderSocket, broadCast.c_str(), broadCast.length(), 0,
				(SOCKADDR*)&bcSendAddr, addressSize);
			if (result == SOCKET_ERROR)
			{
				wsaError3 = WSAGetLastError();
				std::cout << "Error Code: " << wsaError3 << "\n";
				return MESSAGE_ERROR;
			}
			/*else
			{
				std::cout << "Server is Broadcasting..\n";
			}*/

			for (int i = 0; i < socketCount; i++)
			{
				SOCKET copySocket = readySet.fd_array[i];

				if (FD_ISSET(listenSocket, &readySet))
				{
					newClientSocket = accept(listenSocket, NULL, NULL);                            //Accepting new Client socket

					if (newClientSocket == INVALID_SOCKET)
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "\n";
						return CONNECT_ERROR;
					}
					else if (newClientSocket == 0)
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "\n";
						return SHUTDOWN;
					}
					else
					{
						printf("\nDEBUG: Client was connected...\n");                                           //Client was connected			
					}

					uint8_t size = 0;

					result = currentServer.tcp_recv_whole(newClientSocket, (char*)&size, 1);      //Receive Registration message from client
					if ((result == SOCKET_ERROR))
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "n";
						return MESSAGE_ERROR;
					}
					else if (result == 0)
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "n";
						return SHUTDOWN;
					}

					char* buffer = new char[size];

					result = currentServer.tcp_recv_whole(newClientSocket, (char*)buffer, size);
					if ((result == SOCKET_ERROR))
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "n";
						return MESSAGE_ERROR;

					}
					else if (result == 0)
					{
						wsaError3 = WSAGetLastError();
						std::cout << "Error Code: " << wsaError3 << "n";
						return SHUTDOWN;
					}
					else
					{
						messageBuffer = buffer;
						printf("DEBUG: Client Registration Received\n");
						outfile.open("ServerComm.txt", std::ios::app);
						std::string logMessage = "Client Registration: " + messageBuffer + "\n";
						outfile << logMessage;
						outfile.close();
					}

					if (messageBuffer.substr(0, 9) == "$register")								//Fully registering the Client
					{
						if (currentServer.Clients.size() < 3)
						{
							ClientStruct newClient;

							std::string clientName = messageBuffer.substr(10);

							newClient.Name = clientName;
							newClient.Socket = newClientSocket;
							Clients.push_back(newClient);
							clientNameList.push_back(newClient.Name);
							clientSocketList.push_back(newClientSocket);
							FD_SET(newClientSocket, &masterSet);

							std::cout << "\n" << newClient.Name << " has joined the chat\n";
							std::cout << newClient.Name << "'s Socket: " << newClient.Socket << "\n";

							uint8_t size = 60;
							char sendbuffer[60];
							memset(sendbuffer, 0, 60);
							strcpy_s(sendbuffer, "\nServer: You have been successfully connected...\n");  //Connection message sent to client

							result = tcp_send_whole(newClientSocket, (char*)&size, 1);
							if ((result == SOCKET_ERROR))
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return MESSAGE_ERROR;

							}
							else if (result == 0)
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return SHUTDOWN;
							}

							result = tcp_send_whole(newClientSocket, sendbuffer, size);
							if ((result == SOCKET_ERROR))
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return MESSAGE_ERROR;

							}
							else if (result == 0)
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return SHUTDOWN;
							}
							else
							{
								printf("\nDEBUG: Client Fully Registered\n");
							}
						}
						else
						{
							uint8_t size = 57;
							char sendbuffer[57];
							memset(sendbuffer, 0, 57);
							strcpy_s(sendbuffer, "\nServer: Too many Clients Connected! Unable to connect..\n");        //Disconenctiong the client due to too max clients

							result = tcp_send_whole(newClientSocket, (char*)&size, 1);
							if ((result == SOCKET_ERROR))
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return MESSAGE_ERROR;

							}
							else if (result == 0)
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return SHUTDOWN;
							}

							result = tcp_send_whole(newClientSocket, sendbuffer, size);
							if ((result == SOCKET_ERROR))
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return MESSAGE_ERROR;

							}
							else if (result == 0)
							{
								wsaError3 = WSAGetLastError();
								std::cout << "Error Code: " << wsaError3 << "n";
								return SHUTDOWN;
							}
							else
							{
								printf("\nDEBUG: Client couldn't be connected due to too many connections already..\n");
							}
						}
					}
				}

				for (int i = 0; i < Clients.size(); i++)
				{
					int currentSocket1 = clientSocketList[i];

					if (FD_ISSET(currentSocket1, &readySet))
					{
						uint8_t size = 0;

						result = currentServer.tcp_recv_whole(copySocket, (char*)&size, 1);
						if ((result == SOCKET_ERROR))
						{
							wsaError3 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError3 << "\n";
							return MESSAGE_ERROR;

						}
						else if (result == 0)
						{
							wsaError3 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError3 << "\n";
							return SHUTDOWN;
						}

						char* buffer = new char[size];

						result = currentServer.tcp_recv_whole(copySocket, (char*)buffer, size);
						if ((result == SOCKET_ERROR))
						{
							wsaError3 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError3 << "\n";
							return MESSAGE_ERROR;

						}
						else if (result == 0)
						{
							wsaError3 = WSAGetLastError();
							std::cout << "Error Code: " << wsaError3 << "\n";
							return SHUTDOWN;
						}
						else
						{
							printf("\nDEBUG: Received message from Client\n");
							printf("Client: ");
							;								messageBuffer = buffer;
							std::cout << messageBuffer << "\n";
						}

						if (messageBuffer.substr(0, 8) == "$getlist")
						{
							if (Clients.size() <= 1)
							{
								uint8_t size = 53;
								char sendbuffer[53];
								memset(sendbuffer, 0, 53);
								strcpy_s(sendbuffer, "\nThere are no other Clients connected to the Server\n");  //Connection message sent to client

								//I need to only send to Client requestiong the data

								result = tcp_send_whole(currentSocket1, (char*)&size, 1);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}

								result = tcp_send_whole(currentSocket1, sendbuffer, size);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}
								else
								{
									printf("\nDEBUG: Sent Empty Client getlist\n");

								}
							}
							else
							{
								std::ostringstream nameStringStream;

								if (!clientNameList.empty())
								{
									std::copy(clientNameList.begin(), clientNameList.end() - 1, std::ostream_iterator<std::string>(nameStringStream, ","));
									nameStringStream << clientNameList.back();
								}

								std::string nameStringConvert = nameStringStream.str();
								const char* clientNameChar = nameStringConvert.c_str();

								uint8_t size = 53;
								char sendbuffer[53];
								memset(sendbuffer, 0, 53);
								strcpy_s(sendbuffer, clientNameChar);

								//I need to only send to Client requestiong the data

								result = tcp_send_whole(copySocket, (char*)&size, 1);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}

								result = tcp_send_whole(copySocket, sendbuffer, size);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}
								else
								{
									printf("\nDEBUG: Sent Full Client getlist\n");
								}
							}

							outfile.open("ServerComm.txt", std::ios::app);
							std::string logMessage = "Command: " + messageBuffer + "\n";
							outfile << logMessage;
							outfile.close();
						}
						else if (messageBuffer.substr(0, 5) == "$exit")
						{
							outfile.open("ServerComm.txt", std::ios::app);
							std::string logMessage = "Command: " + messageBuffer + "\n";
							outfile << logMessage;
							outfile.close();

							shutdown(readySet.fd_array[i], SD_BOTH);
							closesocket(readySet.fd_array[i]);

							FD_CLR(readySet.fd_array[i], &masterSet);

							for (auto cl : currentServer.Clients)
							{
								int count = 0;
								if (cl.Socket == readySet.fd_array[i])
								{
									currentServer.Clients.erase(currentServer.Clients.begin() + count);
									std::string log = "Client " + currentServer.Clients[count].Name + " has left the chat!";
									std::cout << log;

									//Append Message here
								}
								count++;
							}
						}
						else if (messageBuffer.substr(0, 7) == "$getlog")
						{

							outfile.open("ServerComm.txt", std::ios::app);
							std::string logMessage = "Command: " + messageBuffer + "\n";
							outfile << logMessage;
							std::string endRecv = "END";
							outfile << endRecv;
							outfile.close();

							outfile.open("ServerComm.txt", std::ios::in);   //Opening file

							if (outfile.is_open())
							{

								outfile.seekg(0, std::ios::end);  // Getting size of file
								int fileSize = outfile.tellg();

								std::string sizeString = std::to_string(fileSize);
								const char* sizeString2 = sizeString.c_str();
								outfile.seekg(0, std::ios::beg);
								std::string fileSize2 = std::to_string(fileSize);

								uint8_t size = fileSize;
								char sendbuffer[4069];
								memset(sendbuffer, 0, fileSize);
								strcpy_s(sendbuffer, sizeString2);

								//I need to only send to Client requestiong the data

								result = tcp_send_whole(copySocket, (char*)&size, 1);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}

								result = tcp_send_whole(copySocket, sendbuffer, size);
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}
								else
								{
									printf("\nDEBUG: Sent Full Client getlist\n");
								}

								size = sizeof(fileSize2);

								result = tcp_send_whole(copySocket, (char*)&size, 1);   // Sending file size to client
								if ((result == SOCKET_ERROR))
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return MESSAGE_ERROR;

								}
								else if (result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error Code: " << wsaError3 << "n";
									return SHUTDOWN;
								}

								std::string fileLine;

								while (std::getline(outfile, fileLine))                // Trying to send file to client line by line
								{
									const char* fileLineChar = fileLine.c_str();										

									result = tcp_send_whole(copySocket, fileLineChar, size);
									if ((result == SOCKET_ERROR))
									{
										wsaError3 = WSAGetLastError();
										std::cout << "Error Code: " << wsaError3 << "n";
										return MESSAGE_ERROR;

									}
									else if (result == 0)
									{
										wsaError3 = WSAGetLastError();
										std::cout << "Error Code: " << wsaError3 << "n";
										return SHUTDOWN;
									}
									else
									{
										printf("\nDEBUG: Sent getlog\n");
									}
								}

								outfile.close();
							}
						}
						else
						{
							outfile.open("ServerComm.txt", std::ios::app);
							std::string logMessage = "Client Message: " + messageBuffer + "\n";
							outfile << logMessage;
							outfile.close();

							std::string nameStringConvert = messageBuffer;

							const char* clientMessageEcho = nameStringConvert.c_str();

							uint8_t size = 57;
							char sendbuffer[57];
							memset(sendbuffer, 0, 57);
							strcpy_s(sendbuffer, clientMessageEcho);

							for (int j = 0; j < clientSocketList.size(); j++)
							{
								SOCKET targetSocket = clientSocketList[j];

								// Skip the sender
								if (targetSocket == currentSocket1)
									continue;

								// Send size
								result = tcp_send_whole(targetSocket, (char*)&size, 1);
								if ((result == SOCKET_ERROR) || result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error sending size to client: " << wsaError3 << "\n";
									continue; // Don't return, just skip this client
								}

								// Send message
								result = tcp_send_whole(targetSocket, sendbuffer, size);
								if ((result == SOCKET_ERROR) || result == 0)
								{
									wsaError3 = WSAGetLastError();
									std::cout << "Error sending message to client: " << wsaError3 << "\n";
									continue;
								}
							}
						}
					}
				}
			}
			
		} while (activeServer);

	return SUCCESS;
}

int Server::tcp_recv_whole(SOCKET s, char* buf, int len)
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

int Server::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
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

void Server::stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	shutdown(newClientSocket, SD_BOTH);
	closesocket(newClientSocket);
}



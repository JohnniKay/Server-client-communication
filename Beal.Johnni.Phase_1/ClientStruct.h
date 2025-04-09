#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
//#pragma comment(lib,"Ws2_32.lib")
#include <iostream>
#include <vector>

struct ClientStruct
{
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);           // Creating socket;
	std::string Name;
	std::string Port;
	std::string Address;
};

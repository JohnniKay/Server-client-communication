#pragma once
#include <cstdint>
#include "Definitions.h"
#include "ClientStruct.h"
#include <iostream>
#include <vector>

class Server
{
	public:

		int maxClients = 3;
		std::vector<ClientStruct> Clients;
		int init();
		int tcp_recv_whole(SOCKET s, char* buf, int len);
		int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
		void stop();		
};


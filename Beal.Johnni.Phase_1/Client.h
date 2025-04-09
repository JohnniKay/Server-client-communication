#pragma once
#include <cstdint>
#include "Definitions.h"
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>


class Client
{
	public:

		std::string name;
		int init();
		int tcp_recv_whole(SOCKET s, char* buf, int len);
		int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
		void stop();
};


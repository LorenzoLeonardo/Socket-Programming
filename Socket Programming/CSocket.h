#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include <vector>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE 1024

class CSocket
{
protected:
	SOCKET m_socket;
	struct sockaddr m_addr;
public:
	CSocket(SOCKET s)
	{
		m_socket = s;
		memset(&m_addr, 0, sizeof(m_addr));
	}

	SOCKET GetSocket()
	{
		return m_socket;
	}
	void SetClientAddr(struct sockaddr addr)
	{
		m_addr = addr;
	}

	string GetIP()
	{
		struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&m_addr;
		struct in_addr ipAddr = pV4Addr->sin_addr;

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

		string ip = str;
		return ip;
	}
    string Receive();
	void Send(string sendbuf);
};


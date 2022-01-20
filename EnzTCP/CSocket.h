#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "pch.h"
#include "EnzTCP.h"
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

class ENZTCPLIBRARY_API CSocket
{
private:
	SOCKET m_socket;
	struct sockaddr m_addr;
	string m_hostname;
	string m_ipAddress;
private:
	void SetHostname()
	{
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		int iResult = 0;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(m_ipAddress.c_str(), NULL, &hints, &result);
		{
			char host[512];
			memset(host, 0, sizeof(host));
			int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);
			m_hostname = host;
			freeaddrinfo(result);
		}
	}
	void SetIP()
	{
		struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&m_addr;
		struct in_addr ipAddr = pV4Addr->sin_addr;

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);

		m_ipAddress = str;

	}
	void EraseAllSubStr(std::string& mainStr, const std::string& toErase);
	void EraseSubStringsPre(std::string& mainStr, const std::vector<std::string>& strList);

public:
	CSocket(SOCKET s)
	{
		m_socket = s;
		memset(&m_addr, 0, sizeof(m_addr));
		m_hostname = "";
		m_ipAddress = "";
	}

	SOCKET  GetSocket()
	{
		return m_socket;
	}

	void  SetClientAddr(struct sockaddr addr)
	{
		m_addr = addr;
		SetIP();
		SetHostname();
	}
	 string GetIP()
	{
		return m_ipAddress;
	}
	string GetHostName()
	{
		return m_hostname;
	}

	string Receive();
	void Send(string sendbuf);
};


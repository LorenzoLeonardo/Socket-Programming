#pragma once
#include "EnzTCP.h"
#include "ISocket.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include <windows.h>


using namespace std;

class ENZTCPLIBRARY_API CSocket : ISocket
{
private:
	SOCKET m_socket;
	struct sockaddr m_addr;
	string m_hostname;
	string m_ipAddress;
	string dataRecv;
private:
	void SetHostname();
	void SetIP();

public:
	CSocket(SOCKET s);
	~CSocket();
	SOCKET  GetSocket();
	void  SetClientAddr(struct sockaddr addr);
	const char* GetIP();
	const char* GetHostName();
	const char* Receive();
	void Send(char* sendbuf);
};


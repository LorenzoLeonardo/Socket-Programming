#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include "EnzTCP.h"
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE 1024

class ENZTCPLIBRARY_API ISocket
{
private:
	void SetHostname();
	void SetIP();
public:
	virtual SOCKET GetSocket() = 0;
	virtual void   SetClientAddr(struct sockaddr addr) = 0;
	virtual string GetIP() = 0;
	virtual string GetHostName() = 0;
	virtual string Receive() = 0;
	virtual void   Send(string sendbuf) = 0;
};

class ENZTCPLIBRARY_API CSocket : ISocket
{
private:
	SOCKET m_socket;
	struct sockaddr m_addr;
	string m_hostname;
	string m_ipAddress;
private:
	void SetHostname();
	void SetIP();

public:
	CSocket(SOCKET s);
	~CSocket();
	SOCKET  GetSocket();
	void  SetClientAddr(struct sockaddr addr);
	string GetIP();
	string GetHostName();
	string Receive();
	void Send(string sendbuf);
};


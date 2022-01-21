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

#define MAX_BUFFER_SIZE 1024

using namespace std;
#pragma comment(lib, "Ws2_32.lib")
class CSocketClient
{
private:
	string m_serverIP;
	string m_serverPort;
	SOCKET m_socket;

public:
	CSocketClient(string ipAddress, string port);
	~CSocketClient();

	void Connect();
	void Disconnect();
	string Receive();
	void Send(string sendbuf);

};


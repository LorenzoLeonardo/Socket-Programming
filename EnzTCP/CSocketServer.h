#pragma once

#include "CSocket.h"
#include <process.h>

class CSocketServer
{
private:
	vector<CSocket> v_clientSocket;
	
	SOCKET m_ListenSocket;
	string m_serverPort;
public:
	CSocketServer()
	{
		m_ListenSocket = INVALID_SOCKET;
	}
	CSocketServer(string serverPort);
	~CSocketServer();

	bool Initialize(string port);
	bool Cleanup();
	CSocket* Accept();
};


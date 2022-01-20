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
	CSocketServer(string serverPort);
	~CSocketServer();

	bool Initialize();
	bool Cleanup();
	CSocket* Accept();
};


#pragma once

#include"CSocket.h"
#include "CSocketServer.h"
#include <thread>
#include <stdio.h>
#include <string>
#include <iostream>
#include "ISocket.h"

using namespace std;

typedef void (*NewConnection)(void*);

class CTCPListener
{
private:
	string m_ipAddress;
	string m_port;
	
	CSocketServer*  m_socketServer;
	bool m_bIsRunning;
	NewConnection m_pfnMessageReceived;

public:
	
	CTCPListener(string ipAddress, string port, NewConnection handler);
	CTCPListener(string port, NewConnection handler);
	~CTCPListener();

	
	void Run();
	void Stop();
};


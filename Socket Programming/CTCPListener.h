#pragma once
#include"CSocket.h"
#include "CSocketServer.h"
#include <thread>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

typedef void (*MessageReceived)(CSocket* pClientConnected, string sMessage);


class CTCPListener
{
private:
	string m_ipAddress;
	string m_port;
	
	CSocketServer*  m_socketServer;
	bool m_bIsRunning;
	vector<HANDLE> v_threads;
	vector<CSocket *> v_socketClients;
	
	thread t;
public:
	
	CTCPListener(string ipAddress, string port, MessageReceived handler);
	~CTCPListener();

	MessageReceived m_pfnMessageReceived;
	void Run();
	void Stop();
	vector<CSocket*> GetListClients()
	{
		return v_socketClients;
	}
	vector<HANDLE> GetListClientsThreads()
	{
		return v_threads;
	}
	CSocketServer* GetSocketServerObj()
	{
		return m_socketServer;
	}
	void Remove(CSocket* socket);
};

static CTCPListener* g_pfnListener;
unsigned _stdcall HandleClientThreadFunc(void *);
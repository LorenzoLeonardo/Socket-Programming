#pragma once
#include "CSocket.h"
#include <string>
using namespace std;

class CSocketClient : CSocket
{
public:
	CSocketClient();
	CSocketClient(string ipServer);
	~CSocketClient();

	bool ConnectToServer(string ipServer, string, int *pLastError);
	bool ConnectToServer(string ipServer, string sPort);
};


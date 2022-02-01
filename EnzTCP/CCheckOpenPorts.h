#pragma once
#include "EnzTCP.h"
#include <string>
#include <map>
#include <thread>

using namespace std;

typedef struct
{
	string sPort;
}THREADMON_t;
class CCheckOpenPorts
{
public :
	CCheckOpenPorts();
	CCheckOpenPorts(string ipTargetIPAddress, int nNumberOfPorts, FuncFindOpenPort* pfnPtr);
	~CCheckOpenPorts();
	FuncFindOpenPort m_pfnFindOpenPort;
	void StartSearchingOpenPorts();
	string GetIPAddress();
	map<thread*, int> GetThreads();

	bool IsPortOpen(string ipAddress, string port);
private:
	string m_ipAddressTarget;
	int m_nNumPorts;
	map<thread*, int> m_mapThreads;
	thread* m_tMonitor;

	
};


#include "pch.h"
#include "CLocalAreaListener.h"
#include "CSocketClient.h"
mutex ntx;

CLocalAreaListener* g_pCLocalAreaListener = NULL;

CLocalAreaListener::CLocalAreaListener(const char* szStartingIPAddress, CallbackLocalAreaListener pFncPtr)
{
	m_fnptrCallbackLocalAreaListener = pFncPtr;
	m_szStartingIP = szStartingIPAddress;
	m_threadMain = NULL;
	m_bHasStarted = false;
	
}

CLocalAreaListener::~CLocalAreaListener()
{
	
}
map<thread*, int>* CLocalAreaListener::GetThreads()
{
	return &m_mapThreads;
}

void MultiQueryingThread(void* args)
{
	string* p = (string*)args;
	string hostName;

	if (g_pCLocalAreaListener->CheckIPDeviceConnected(*p, hostName))
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*) (*p).c_str(), (const char*)hostName.c_str(), true);
	//else
//		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener((const char*)(*p).c_str(), (const char*)hostName.c_str(), false);

	delete p;
}

void MainThread(void* args)
{
	CLocalAreaListener* pCLocalAreaListener = (CLocalAreaListener*)args;
	
	string ipAddressStart = pCLocalAreaListener->GetStartingIPAddress();
	ipAddressStart = ipAddressStart.substr(0, ipAddressStart.rfind('.', ipAddressStart.size()) + 1);
	while (pCLocalAreaListener->HasNotStopped())
	{
		
		for (int i = 1; i <= 254; i++)
		{
			string* str = new string;
			*str = ipAddressStart + to_string(i);
			(*pCLocalAreaListener->GetThreads())[new thread(MultiQueryingThread, str)] = i;
		}
		map<thread*, int>::iterator it = pCLocalAreaListener->GetThreads()->begin();

		while (it != pCLocalAreaListener->GetThreads()->end())
		{
			it->first->join();
			it++;
		}
		it = pCLocalAreaListener->GetThreads()->begin();
		while (it != pCLocalAreaListener->GetThreads()->end())
		{
			delete it->first;
			it++;
		}
		pCLocalAreaListener->GetThreads()->clear();
		g_pCLocalAreaListener->m_fnptrCallbackLocalAreaListener(NULL, NULL, false);
		Sleep(5000);
	}
	return;

}
string CLocalAreaListener::GetStartingIPAddress()
{
	return m_szStartingIP;
}
void CLocalAreaListener::Start()
{
	m_bHasStarted = true;
	g_pCLocalAreaListener = this;
	if(m_threadMain != NULL)
	{
		delete m_threadMain;
		m_threadMain = NULL;
	}
	m_threadMain = new thread(MainThread, this);
	m_threadMain->detach();
}

void CLocalAreaListener::Stop()
{
	m_bHasStarted = false;
}
bool CLocalAreaListener::CheckIPDeviceConnected(string ipAddress,string &hostName)
{
	//EnterCriticalSection(&CriticalSection);
	CSocketClient* clientSock = new CSocketClient(ipAddress);
	bool bRet = clientSock->CheckDevice(ipAddress, hostName);
	delete clientSock;

	return bRet;
}

#include "pch.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"

CCheckOpenPorts* g_objPtrCCheckOpenPorts = NULL;
CCheckOpenPorts::CCheckOpenPorts()
{
	m_pfnFindOpenPort = NULL;
	m_nNumPorts = 0;
	m_ipAddressTarget = "";
	g_objPtrCCheckOpenPorts = this;
}
CCheckOpenPorts::CCheckOpenPorts(string ipTargetIPAddress, int nNumberOfPorts, FuncFindOpenPort pfnPtr)
{
	m_pfnFindOpenPort = pfnPtr;
	m_nNumPorts = nNumberOfPorts;
	m_ipAddressTarget = ipTargetIPAddress;
	g_objPtrCCheckOpenPorts = this;
}
CCheckOpenPorts::~CCheckOpenPorts()
{

}
string CCheckOpenPorts::GetIPAddress()
{
	return m_ipAddressTarget;
}
map<thread*, int> CCheckOpenPorts::GetThreads()
{
	return m_mapThreads;
}
thread* CCheckOpenPorts::GetThreadMonitoring()
{
	return m_tMonitor;
}
void ThreadMonitorThreads(LPVOID pParam)
{
	map<thread*, int>* PDlg = (map<thread*, int>*)pParam;
	map<thread*, int>::iterator it = PDlg->begin();

	while (it != PDlg->end())
	{
		it->first->join();
		it++;
	}
	it = PDlg->begin();
	while (it != PDlg->end())
	{
		delete it->first;
		it++;
	}
	PDlg->clear();
	delete g_objPtrCCheckOpenPorts->GetThreadMonitoring();
	return;
}
bool CCheckOpenPorts::IsPortOpen(string ipAddress, string port)
{
	CSocketClient clientSock(ipAddress);

	return clientSock.ConnectToServer(ipAddress, port);
}
void ThreadMultiFunc(LPVOID pParam)
{
	string cs;
	THREADMON_t* pTmon = (THREADMON_t*)pParam;

	cs = g_objPtrCCheckOpenPorts->GetIPAddress();

	if (g_objPtrCCheckOpenPorts->IsPortOpen(cs, pTmon->sPort))
	{
		//csRes = _T("Port (") + csPort + _T(") Of (") + cs + _T(") is open.\r\n");
		g_objPtrCCheckOpenPorts->m_pfnFindOpenPort((char*)cs.c_str(), stoi(pTmon->sPort), true);
	}
	else
	{
		g_objPtrCCheckOpenPorts->m_pfnFindOpenPort((char*)cs.c_str(), stoi(pTmon->sPort), false);
	}
	return;
}

void CCheckOpenPorts::StartSearchingOpenPorts()
{
	for (int i = 0; i < m_nNumPorts; i++)
	{
		THREADMON_t *ptmon= new THREADMON_t;
		ptmon->sPort = to_string(i);
		m_mapThreads[new thread(ThreadMultiFunc, ptmon)] = i;
	}
	m_tMonitor = new thread(ThreadMonitorThreads, &m_mapThreads);
	m_tMonitor->detach();
}
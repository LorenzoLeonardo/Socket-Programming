#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"
#include "CLocalAreaListener.h"
#include "CSNMP.h"

CCheckOpenPorts* g_pOpenPorts = NULL;
CLocalAreaListener* g_pLocalAreaListener = NULL;
CSNMP*   g_SNMP = NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
        if(g_pOpenPorts != NULL)
        {
            delete g_pOpenPorts;
            g_pOpenPorts = NULL;
        }
        if (g_pLocalAreaListener != NULL)
        {
            delete g_pLocalAreaListener;
            g_pLocalAreaListener = NULL;
        }
        if (g_SNMP != NULL)
        {
            delete g_SNMP;
            g_SNMP = NULL;
        }
        break;
    }
    return TRUE;
}

HANDLE ENZTCPLIBRARY_API OpenServer(const char* sport, FuncNewConnection pfnPtr)
{
    CTCPListener* Ptr = new CTCPListener(sport, pfnPtr);
    return (HANDLE)Ptr;
}

void ENZTCPLIBRARY_API RunServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Run();
}
void ENZTCPLIBRARY_API CloseClientConnection(HANDLE hHandle)
{
    CSocket* clientSocket = (CSocket*)hHandle;

    if (clientSocket != NULL)
        delete clientSocket;
}
void ENZTCPLIBRARY_API CloseServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    if (listener != NULL)
    {
        listener->Stop();
        delete listener;
    }
}

void ENZTCPLIBRARY_API EnumOpenPorts(char* ipAddress, int nNumPorts, FuncFindOpenPort pfnPtr)
{
    if(g_pOpenPorts != NULL)
    {
        delete g_pOpenPorts;
        g_pOpenPorts = NULL;
    }
    string sAddress(ipAddress);

    g_pOpenPorts = new CCheckOpenPorts(sAddress, nNumPorts, pfnPtr);
    g_pOpenPorts->StartSearchingOpenPorts();
    return;
}


bool ENZTCPLIBRARY_API IsPortOpen(char* ipAddress, int nNumPorts, int *pnlastError)
{
   string sAddress(ipAddress);

   CSocketClient port;
    return port.ConnectToServer(sAddress, to_string(nNumPorts), pnlastError);
}

void ENZTCPLIBRARY_API StartLocalAreaListening(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int nPollingTimeMS)
{
    if (g_pLocalAreaListener != NULL)
    {
        delete g_pLocalAreaListener;
        g_pLocalAreaListener = NULL;
    }
    g_pLocalAreaListener = new CLocalAreaListener(ipAddress, fnpPtr, nPollingTimeMS);
    g_pLocalAreaListener->Start();
}
void ENZTCPLIBRARY_API StopLocalAreaListening()
{
    if (g_pLocalAreaListener != NULL)
    {
        g_pLocalAreaListener->Stop();
    }
}

bool ENZTCPLIBRARY_API StartSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD& dwLastError)
{
    if (g_SNMP != NULL)
    {
        delete g_SNMP;
        g_SNMP = NULL;
    }
    g_SNMP = new CSNMP();

    return g_SNMP->InitSNMP(szAgentIPAddress, szCommunity, nVersion, dwLastError);
}
smiVALUE ENZTCPLIBRARY_API SNMPGet(const char* szOID, DWORD& dwLastError)
{
    smiVALUE value;
    memset(&value, 0, sizeof(value));

    if (g_SNMP == NULL)
    {
        return value;
    }
    return g_SNMP->Get(szOID, dwLastError);
}
void ENZTCPLIBRARY_API EndSNMP()
{
    g_SNMP->EndSNMP();
    if (g_SNMP != NULL)
    {
        delete g_SNMP;
        g_SNMP = NULL;
    }
}

bool ENZTCPLIBRARY_API GetDefaultGateway(char* szDefaultIPAddress)
{
    WSADATA wsaData;
    IPAddr* pDefaultGateway = 0;
  
    ULONG PhysAddrLen = 6;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult = 0;
    
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
        return false;
 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo("localhost", NULL, &hints, &result);
    if (iResult == 0)
    {
        char host[512];
        memset(host, 0, sizeof(host));
        int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);
        freeaddrinfo(result);
        iResult = getaddrinfo(host, NULL, &hints, &result);
        pDefaultGateway = (IPAddr*)(result->ai_addr->sa_data + 2);
        char szIPAddress[32];
        memset(szIPAddress, 0, sizeof(szIPAddress));

        inet_ntop(AF_INET, (const void*)pDefaultGateway, szIPAddress, sizeof(szIPAddress));
        string sTemp = szIPAddress;

        sTemp= sTemp.substr(0,sTemp.rfind('.', sTemp.length())+1);
        sTemp += "1";
        
        memcpy_s(szDefaultIPAddress, sizeof(char)*sTemp.length(), sTemp.c_str(), sizeof(char) * sTemp.length());
        WSACleanup();
        return true;
    }
    else
    {
        WSACleanup();
        return false;
    }
}
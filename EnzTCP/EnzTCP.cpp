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
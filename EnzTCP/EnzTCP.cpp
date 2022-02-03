#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"
#include "CLocalAreaListener.h"

CCheckOpenPorts* g_pOpenPorts = NULL;
CLocalAreaListener* g_pLocalAreaListener = NULL;

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


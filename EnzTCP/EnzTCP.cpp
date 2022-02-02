#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"
#include "CCheckOpenPorts.h"
#include "CSocketClient.h"

CCheckOpenPorts* g_pOpenPorts = NULL;

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


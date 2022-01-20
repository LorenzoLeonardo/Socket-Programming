#include "CSocketServer.h"
#include "CTCPListener.h"
#include <vector>

using namespace std;


vector < CSocket*> g_vSocket;



HANDLE OpenServer(string sport, NewConnection fncPtr)
{
   return (HANDLE) new CTCPListener(sport, fncPtr);
}

void RunServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Run();
}

void CloseServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Stop();
    delete listener;
}

unsigned _stdcall HandleClientThreadFunc(void* pArguments)
{
    CSocket* pSocket = (CSocket*)pArguments;

    g_vSocket.erase(std::remove(g_vSocket.begin(), g_vSocket.end(), pSocket), g_vSocket.end());
    
    delete pSocket;
    _endthreadex(0);
    return 0;
}

void NewClientConnection(void* pData)
{
    g_vSocket.push_back((CSocket*)pData);
    _beginthreadex(NULL, 0, &HandleClientThreadFunc, (CSocket*)pData, 0, 0);
}

int main()
{
    HANDLE hHandle = OpenServer("0611", NewClientConnection);

    RunServer(hHandle);
    CloseServer(hHandle);
    return 0;
}


#include "CSocketServer.h"
#include "CTCPListener.h"
#include <vector>

using namespace std;


vector < CSocket*> g_vSocket;


unsigned _stdcall HandleClientThreadFunc(void*);

unsigned _stdcall HandleClientThreadFunc(void* pArguments)
{
 
    _endthreadex(0);
    return 0;
}

void NewClientConnection(void* pData)
{
    g_vSocket.push_back((CSocket*)pData);
    _beginthreadex(NULL, 0, &HandleClientThreadFunc, socket, 0, 0);
}

int main()
{
    CTCPListener listener("192.168.0.101","0611", NewClientConnection);

    listener.Run();

    return 0;
}


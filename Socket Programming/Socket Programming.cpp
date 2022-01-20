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
    string sData = "";
    
    for (int i = 0; i < g_vSocket.size(); i++)
    {
        if (g_vSocket[i] != pSocket)
        {
            g_vSocket[i]->Send("\r\n"+ pSocket->GetIP() + " has joined the chat.");
        }
    }
    while (!(sData = pSocket->Receive()).empty())
    {
        for (int i = 0; i < g_vSocket.size(); i++)
        {
            if (g_vSocket[i] != pSocket)
            {
                g_vSocket[i]->Send("\r\n" + pSocket->GetIP()+" : "+ sData);
            }
        }
    }
    for (int i = 0; i < g_vSocket.size(); i++)
    {
        if (g_vSocket[i] != pSocket)
        {
            g_vSocket[i]->Send("\r\n" + pSocket->GetIP() + " has left the chat.");
        }
    }
    g_vSocket.erase(std::remove(g_vSocket.begin(), g_vSocket.end(), pSocket), g_vSocket.end());
    
    delete pSocket;
    _endthreadex(0);
    return 0;
}

void NewClientConnection(void* pData)
{
    ((CSocket*)pData)->Send("Welcome to Lorenzo Leonardo's Awesome Chat\r\n");

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


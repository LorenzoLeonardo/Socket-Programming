#include "CSocketServer.h"
#include "CTCPListener.h"
#include <vector>

using namespace std;

/*vector<CSocket*> vSocket;

HANDLE hThread;
unsigned int threadID;

unsigned __stdcall HandleClientThreadFunc(void* pArguments)
{
    CSocket* pSock = (CSocket*)pArguments;
    bool isConnected = true;
    string sData;

    while (isConnected)
    {
        try
        {
            sData = pSock->Receive();
            cout <<"FROM "<< pSock->GetIP() << "(" << pSock->GetHostName() << ") " << sData;
            cout << endl;
        }
        catch (int nError)
        {
            cout << "Exception : " << nError <<endl;
            isConnected = false;
        }

        try
        {
            sData = "Acknowledge (" + pSock->GetIP() + ")";
            pSock->Send(sData);
        }
        catch (int nError)
        {
            cout << "Exception : " << nError << endl;
            isConnected = false;
        }
        
    }
    cout << pSock->GetIP() << "(" << pSock->GetHostName() << ")" << " is disconnected";
    cout << endl;

    delete pSock;
    pSock = NULL;
    _endthreadex(0);
    return 0;
}

unsigned __stdcall HandleServerExit(void* pArguments)
{
    CSocketServer* server = (CSocketServer*)pArguments;
    bool bFlag = true;
    while (bFlag)
    {
        if (GetAsyncKeyState(VK_ESCAPE))
        {
            bFlag = false;
        }
        Sleep(1);
    }
    server->Cleanup();
    _endthreadex(0);
    return 0;
}*/

void Listen_MessageReceived(CSocket* socket, string message)
{
    cout << message << endl;
    return;
}

int main()
{
    CTCPListener listener("192.168.0.101","0611", Listen_MessageReceived);

    listener.Run();
/*    CSocketServer  server("0611");
   // hThread = (HANDLE)_beginthreadex(NULL, 0, &HandleServerExit, &server, 0, &threadID);

    printf("Starting Server...\r\n");
    server.Initialize();
    bool bServerAlive = true;
    printf("The Server is now starting to accept connections...\r\n");
    while (bServerAlive)
    {
        try
        {
            CSocket* socket = server.Accept();

            if (socket != NULL)
                hThread = (HANDLE)_beginthreadex(NULL, 0, &HandleClientThreadFunc, socket, 0, &threadID);
        }
        catch (int nError)
        {
            printf("Accept Error : %d\r\n", nError);
            bServerAlive = false;
        }
    }

    server.Cleanup();*/
    return 0;
}


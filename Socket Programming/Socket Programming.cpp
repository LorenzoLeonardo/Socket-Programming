#include "CSocketServer.h"

#include <vector>

using namespace std;

vector<CSocket*> vSocket;

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
            printf("FROM (%s) : %s\r\n", pSock->GetIP().c_str(), sData.c_str());
        }
        catch (int nError)
        {
            printf("Receive Error : %d\r\n", nError);
            isConnected = false;
        }

        try
        {
            sData = "Acknowledge (" + pSock->GetIP() + ")";
            pSock->Send(sData);
        }
        catch (int nError)
        {
            printf("Send Error : %d\r\n", nError);
            isConnected = false;
        }
        
    }

    delete pSock;
    _endthreadex(0);
    return 0;
}

int main()
{
  

    CSocketServer  server("1234");

    printf("Starting Server...\r\n");
    server.Initialize();

    while (1)
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
          //  server.Initialize();
        }
    }

  

    server.Cleanup();


    return 0;
}


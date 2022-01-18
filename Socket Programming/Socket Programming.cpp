#include "CSocketServer.h"

#include <vector>

using namespace std;

vector<CSocket*> vSocket;

HANDLE hThread;
unsigned int threadID;

unsigned __stdcall HandleClientThreadFunc(void* pArguments)
{
    while (1)
    {
        for (int i = 0; i < vSocket.size(); i++)
        {
            printf("Message From (%s) : %s", vSocket[i]->GetIP().c_str(), vSocket[i]->Receive().c_str());
            printf("\r\n");
        }
    }


    _endthreadex(0);
    return 0;
}

int main()
{
    hThread = (HANDLE)_beginthreadex(NULL, 0, &HandleClientThreadFunc, NULL, 0, &threadID);


    CSocketServer  server("1234");
    try
    {
       
        server.Initialize();
     
        while (true)
        {
            CSocket* socket = server.Accept();
            vSocket.push_back(socket);
        }

    }
    catch(int nError)
    {
        printf("Socket Error : %d", nError);
    }

    server.Cleanup();


    return 0;
}


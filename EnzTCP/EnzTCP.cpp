#include "pch.h"
#include "EnzTCP.h"
#include "CTCPListener.h"



HANDLE OpenServer(string sport, NewConnection fncPtr)
{
    return (HANDLE) new CTCPListener(sport, fncPtr);
}

void RunServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Run();
}
void CloseClientConnection(HANDLE hHandle)
{
    CSocket* clientSocket = (CSocket*)hHandle;

    delete clientSocket;
}
void CloseServer(HANDLE hHandle)
{
    CTCPListener* listener = (CTCPListener*)hHandle;

    listener->Stop();
    delete listener;
}
// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <process.h>
#include <mutex>

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define DEFAULT_PORT "0611"
#define DEFAULT_BUFLEN 512
bool g_bFlag;

HANDLE m_console;
COORD m_cursorPosition;
mutex m_mutex;

inline void GotoXY(int x, int y)
{
    m_cursorPosition.X = x; // Locates column
    m_cursorPosition.Y = y; // Locates Row
    SetConsoleCursorPosition(m_console, m_cursorPosition); // Sets position for next thing to be printed 
}

unsigned __stdcall HandleClientExit(void* pArguments)
{
    SOCKET* ConnectSocket = (SOCKET*)pArguments;
   
    while (g_bFlag)
    {
        if (GetAsyncKeyState(VK_ESCAPE))
        {
            g_bFlag = false;
        }
        Sleep(1);
    }
    
    closesocket(*ConnectSocket);
    WSACleanup();
    g_bFlag = false;
   // _endthreadex(0);
    return 0;
}
HANDLE hThread_reply = NULL;
HANDLE hThread_send = NULL;
HANDLE hThread_exit = NULL;
unsigned int threadID_reply;
unsigned int threadID_send;
unsigned int threadID_exit;

unsigned __stdcall HandleServerReply(void* pArguments)
{
    SOCKET* ConnectSocket = (SOCKET*)pArguments;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    memset(recvbuf, 0, sizeof(recvbuf));
    int iResult = 0;

    do {
       
    iResult = recv(*ConnectSocket, recvbuf, recvbuflen, 0);
    m_mutex.lock();
   // GotoXY(100, 5);
    if (iResult > 0)
        printf("From Server : %s\n", recvbuf);
    else if (iResult == 0)
        printf("Connection closed\n");
    else
    {
        printf("recv failed: %d\n", WSAGetLastError());
        iResult = -1;
    }
    m_mutex.unlock();
    } while (iResult > 0);
    //_endthreadex(0);
    return 0;
}
unsigned __stdcall HandleSendMessage(void* pArguments)
{
    SOCKET* ConnectSocket = (SOCKET*)pArguments;
    int iResult = 0;

    string sendbuf = "begin";

    while (g_bFlag)
    {
        m_mutex.lock();
       // GotoXY(1, 15);
        cout <<"Type your message: ";
       
        getline(cin, sendbuf);
        m_mutex.unlock();
        //cout << endl;
       
        iResult = send(*ConnectSocket, sendbuf.c_str(), (int)(sendbuf.length()+1), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(*ConnectSocket);
            WSACleanup();
            g_bFlag = false;
        }
    }
    return 0;
}
int main()
{
    string ipAdd;
    m_console = GetStdHandle(STD_OUTPUT_HANDLE);
    g_bFlag = true;
    printf("Starting Client!\r\n");
    printf("Input IP Address :");
    cin >> ipAdd;

   WSADATA wsaData;
    int iResult;
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Initialize Winsock

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("WSAStartup\r\n");

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    // Resolve the server address and port
    iResult = getaddrinfo(ipAdd.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    printf("getaddrinfo\r\n");
    // Attempt to connect to the first address returned by
// the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);
    
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    printf("socket\r\n");

    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }
    printf("connect\r\n");
    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    hThread_exit = (HANDLE)_beginthreadex(NULL, 0, &HandleClientExit, &ConnectSocket, 0, &threadID_exit);
    hThread_reply = (HANDLE)_beginthreadex(NULL, 0, &HandleServerReply, &ConnectSocket, 0, &threadID_reply);
    hThread_send = (HANDLE)_beginthreadex(NULL, 0, &HandleSendMessage, &ConnectSocket, 0, &threadID_send);
  
    

    // Send an initial buffer
    WaitForSingleObjectEx(hThread_send, INFINITE, true);
    WaitForSingleObjectEx(hThread_reply, INFINITE, true);
    WaitForSingleObjectEx(hThread_exit, INFINITE, true);
    return 0;
}


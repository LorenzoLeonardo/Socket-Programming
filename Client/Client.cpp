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

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define DEFAULT_PORT "1234"
#define DEFAULT_BUFLEN 512

int main()
{
    printf("Starting Client!\r\n");
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
    iResult = getaddrinfo("192.168.0.101", DEFAULT_PORT, &hints, &result);
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

    int recvbuflen = DEFAULT_BUFLEN;

    string sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];

    memset(recvbuf, 0, sizeof(recvbuf));
    // Send an initial buffer

    while (1)
    {
        cin >> sendbuf;

        iResult = send(ConnectSocket, sendbuf.c_str(), sendbuf.length() + 1, 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);


        // shutdown the connection for sending since no more data will be sent
        // the client can still use the ConnectSocket for receiving data


        // Receive data until the server closes the connection
        //do {
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0)
                printf("From Server : %s\n", recvbuf);
            else if (iResult == 0)
                printf("Connection closed\n");
            else
                printf("recv failed: %d\n", WSAGetLastError());
       // } while (iResult > 0);
    }
}


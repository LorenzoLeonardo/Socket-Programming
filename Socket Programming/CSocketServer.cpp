#include "CSocketServer.h"


CSocketServer::CSocketServer( string serverPort)
{
    m_serverPort = serverPort;
    m_ListenSocket = INVALID_SOCKET;
}
CSocketServer::~CSocketServer()
{

}

bool CSocketServer::Initialize()
{
    WSADATA wsaData;
    int iResult;
    int nError = 0;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        nError = WSAGetLastError();
        throw nError;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, m_serverPort.c_str(), &hints, &result);
    if (iResult != 0) {
        nError = WSAGetLastError();
        WSACleanup();
        throw nError;
    }

    m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_ListenSocket == INVALID_SOCKET) {
        nError = WSAGetLastError();
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        throw nError;
    }

    // Setup the TCP listening socket
    iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        freeaddrinfo(result);
        closesocket(m_ListenSocket);
        WSACleanup();
        throw nError;
       }

    if (listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        nError = WSAGetLastError();
        closesocket(m_ListenSocket);
        WSACleanup();
        throw nError;
    }

    return true;
}
CSocket* CSocketServer::Accept()
{
    struct sockaddr client_addr;
    int nError = 0;
    int nSize = sizeof(client_addr);
    CSocket* pSocket = NULL;

    memset(&client_addr, 0, sizeof(client_addr));
    SOCKET ClientSocket = accept(m_ListenSocket, (struct sockaddr*)&client_addr,&nSize);
    if (ClientSocket == INVALID_SOCKET) {
        nError = WSAGetLastError();
        closesocket(m_ListenSocket);
        throw nError;
    }
    else
    {
        pSocket = new CSocket(ClientSocket);
        pSocket->SetClientAddr(client_addr);
        string ip = pSocket->GetIP();
    }
    return pSocket;
}
bool CSocketServer::Cleanup()
{
    closesocket(m_ListenSocket);
    WSACleanup();
    return true;
}

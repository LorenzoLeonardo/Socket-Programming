#include "CSocketClient.h"

CSocketClient::CSocketClient(string ipAddress, string port)
{
    m_socket = INVALID_SOCKET;
    m_serverIP = ipAddress;
    m_serverPort = port;
}
CSocketClient::~CSocketClient()
{

}

void CSocketClient::Connect()
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

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    // Resolve the server address and port
    iResult = getaddrinfo(m_serverIP.c_str(), m_serverPort.c_str(), &hints, &result);
    if (iResult != 0) {
        nError = WSAGetLastError();
        WSACleanup();
        throw nError;
    }

    // Attempt to connect to the first address returned by
// the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    m_socket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (m_socket == INVALID_SOCKET) {
        nError = WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        throw nError;
    }
}
void CSocketClient::Disconnect()
{
    int iResult = 0;
    int nError = 0;

    iResult = shutdown(m_socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        throw nError;
    }

    closesocket(m_socket);
}
string CSocketClient::Receive()
{
    int iResult = 0;
    int nError = 0;

    char recvbuf[MAX_BUFFER_SIZE];

    memset(recvbuf, 0, sizeof(recvbuf));

    iResult = recv(m_socket, recvbuf, sizeof(recvbuf), 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(m_socket);
        WSACleanup();
        throw nError;
    }

    string data(recvbuf);
    return data;
}
void CSocketClient::Send(string sendbuf)
{
    int iResult = 0;
    int nError = 0;

    iResult = send(m_socket, sendbuf.c_str(), (int)sendbuf.length() + 1, 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(m_socket);
        WSACleanup();
        throw nError;
    }
}
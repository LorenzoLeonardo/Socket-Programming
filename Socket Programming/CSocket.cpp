#include "CSocket.h"


string CSocket::Receive()
{
    int iResult = 0;
    int nError = 0;

    char recvbuf[MAX_BUFFER_SIZE];

    memset(recvbuf, 0, sizeof(recvbuf));

    iResult = recv(m_socket, recvbuf, sizeof(recvbuf), 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
       // printf("recv failed: %d\n", WSAGetLastError());
       // closesocket(m_socket);
       // WSACleanup();
        throw nError;
    }

    string data(recvbuf);
    return data;
}
void CSocket::Send(string sendbuf)
{
    int iResult = 0;
    int nError = 0;

    iResult = send(m_socket, sendbuf.c_str(), (int)sendbuf.length() + 1, 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
       // printf("send failed: %d\n", WSAGetLastError());
      //  closesocket(m_socket);
       // WSACleanup();
        throw nError;
    }
}
#include "pch.h"
#include "CSocket.h"

void CSocket::EraseAllSubStr(std::string& mainStr, const std::string& toErase)
{
    size_t pos = std::string::npos;
    // Search for the substring in string in a loop untill nothing is found
    while ((pos = mainStr.find(toErase)) != std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

void CSocket::EraseSubStringsPre(std::string& mainStr, const std::vector<std::string>& strList)
{
    // Iterate over the given list of substrings. For each substring call eraseAllSubStr() to
    // remove its all occurrences from main string.
    for (std::vector<std::string>::const_iterator it = strList.begin(); it != strList.end(); it++)
    {
        EraseAllSubStr(mainStr, *it);
    }
}
string CSocket::Receive()
{
    int iResult = 0;
    int nError = 0;

    char recvbuf[MAX_BUFFER_SIZE];

    memset(recvbuf, 0, sizeof(recvbuf));

    iResult = recv(m_socket, recvbuf, sizeof(recvbuf), 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        throw nError;
    }

    string data(recvbuf);

  //  EraseSubStringsPre(data, { "/r/n" });
    return data;
}
void CSocket::Send(string sendbuf)
{
    int iResult = 0;
    int nError = 0;

    iResult = send(m_socket, sendbuf.c_str(), (int)sendbuf.length() + 1, 0);
    if (iResult == SOCKET_ERROR) {
        nError = WSAGetLastError();
        throw nError;
    }
}
#include "pch.h"
#include "CSocketClient.h"

CSocketClient::CSocketClient()
{
}
CSocketClient::CSocketClient(string ipServer)
{
	m_ipAddress = ipServer;
}
CSocketClient::~CSocketClient()
{
}

bool CSocketClient::ConnectToServer(string ipServer, string sPort)
{
	WSADATA wsaData;
	int iResult;
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return false;
	}
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port

	iResult = getaddrinfo(ipServer.c_str(), sPort.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		//mtx.unlock();
		return false;
	}

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
		return false;
	}

	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;

		return false;
	}
	m_socket = ConnectSocket;
	freeaddrinfo(result);
	closesocket(ConnectSocket);
	WSACleanup();
	return true;
}
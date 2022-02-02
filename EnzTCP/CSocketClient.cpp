#include "pch.h"
#include "CSocketClient.h"
#include <mutex>

mutex mtx;
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
bool CSocketClient::CheckDevice(string ipAddress, string &hostname)
{
	WSADATA wsaData;
	int iResult;
	SOCKET ConnectSocket = INVALID_SOCKET;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		mtx.unlock();
		return false;
	}

	int packet_size = DEFAULT_PACKET_SIZE;
	int ttl = DEFAULT_TTL;

	ConnectSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ConnectSocket == INVALID_SOCKET)
	{
		closesocket(ConnectSocket);
		WSACleanup();
		mtx.unlock();
		return false;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_RETURN_TTL;

	iResult = getaddrinfo(ipAddress.c_str(), NULL, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return false;
	}

	char host[512];
	memset(host, 0, sizeof(host));
	int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);
	hostname = host;
	ICMPHeader* send_buf = 0;
	IPHeader* recv_buf = 0;


	packet_size = max(sizeof(ICMPHeader),
		min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));

	allocate_buffers(send_buf, recv_buf, packet_size);

	init_ping_packet(send_buf, packet_size, 0);
	int bwrote = sendto(ConnectSocket, (char*)send_buf, packet_size, 0, result->ai_addr,(int) result->ai_addrlen);
	if (bwrote == SOCKET_ERROR)
	{
		delete[]	send_buf;
		delete[] 	recv_buf;

		//freeaddrinfo(result);
		closesocket(ConnectSocket);
		WSACleanup();

		return false;
	}

	int fromlen = (int)result->ai_addrlen;
	int bread = recvfrom(ConnectSocket, (char*)recv_buf,
		packet_size + sizeof(IPHeader), 0,
		(sockaddr*)&result->ai_addr, &fromlen);
	if (bread == SOCKET_ERROR)
	{
		delete[]	send_buf;
		delete[] 	recv_buf;

		closesocket(ConnectSocket);
		WSACleanup();

		return false;
	}
	delete[]	send_buf;
	delete[] 	recv_buf;

	
	closesocket(ConnectSocket);
	WSACleanup();

	return true;
}
bool CSocketClient::ConnectToServer(string ipServer, string sPort, int *pLastError)
{
	WSADATA wsaData;
	int iResult;
	SOCKET ConnectSocket = INVALID_SOCKET;
	*pLastError = 0;
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
		*pLastError = WSAGetLastError();
		WSACleanup();
		return false;
	}

	// Attempt to connect to the first address returned by
// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		*pLastError = WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		*pLastError = WSAGetLastError();

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


int CSocketClient::allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf, int packet_size)
{
	// First the send buffer
	send_buf = (ICMPHeader*)new char[packet_size + 1];
	if (send_buf == 0) {
		//cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	// And then the receive buffer
	recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE + 1];
	if (recv_buf == 0) {
		//cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	return 0;
}

USHORT CSocketClient::ip_checksum(USHORT* buffer, int size)
{
	unsigned long cksum = 0;

	// Sum all the words together, adding the final byte if size is odd
	while (size > 1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size) {
		cksum += *(UCHAR*)buffer;
	}

	// Do a little shuffling
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	// Return the bitwise complement of the resulting mishmash
	return (USHORT)(~cksum);
}


void CSocketClient::init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no)
{
	// Set up the packet's fields
	icmp_hdr->type = ICMP_ECHO_REQUEST;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->id = (USHORT)GetCurrentProcessId();
	icmp_hdr->seq = seq_no;
	icmp_hdr->timestamp = GetTickCount();

	// "You're dead meat now, packet!"
	const unsigned long int deadmeat = 0xDEADBEEF;
	char* datapart = (char*)icmp_hdr + sizeof(ICMPHeader);
	int bytes_left = packet_size - sizeof(ICMPHeader);
	while (bytes_left > 0) {
		memcpy(datapart, &deadmeat, min(int(sizeof(deadmeat)),
			bytes_left));
		bytes_left -= sizeof(deadmeat);
		datapart += sizeof(deadmeat);
	}

	// Calculate a checksum on the result
	icmp_hdr->checksum = ip_checksum((USHORT*)icmp_hdr, packet_size);
}
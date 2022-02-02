// TestEnzTCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "..\EnzTCP\EnzTCP.h"
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <mutex>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <string>
using namespace std;
#ifdef _MSC_VER
// The following two structures need to be packed tightly, but unlike
// Borland C++, Microsoft C++ does not do this by default.
#pragma pack(1)
#endif

// The IP header
struct IPHeader {
	BYTE h_len : 4;           // Length of the header in dwords
	BYTE version : 4;         // Version of IP
	BYTE tos;               // Type of service
	USHORT total_len;       // Length of the packet in dwords
	USHORT ident;           // unique identifier
	USHORT flags;           // Flags
	BYTE ttl;               // Time to live
	BYTE proto;             // Protocol number (TCP, UDP etc)
	USHORT checksum;        // IP checksum
	ULONG source_ip;
	ULONG dest_ip;
};

// ICMP header
struct ICMPHeader {
	BYTE type;          // ICMP packet type
	BYTE code;          // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
	ULONG timestamp;    // not part of ICMP, but we need it
};
#define DEFAULT_PACKET_SIZE 32
#define DEFAULT_TTL 30
#define MAX_PING_DATA_SIZE 1024
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPHeader))
#define ICMP_ECHO_REPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,
	int packet_size)
{
	// First the send buffer
	send_buf = (ICMPHeader*)new char[packet_size];
	if (send_buf == 0) {
		cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	// And then the receive buffer
	recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
	if (recv_buf == 0) {
		cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	return 0;
}
USHORT ip_checksum(USHORT* buffer, int size)
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


void init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no)
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

int main()
{
	string m_ipAddress = "192.168.0.104";

	WSADATA wsaData;
	int iResult;
	SOCKET ConnectSocket = INVALID_SOCKET;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return false;
	}

	int packet_size = DEFAULT_PACKET_SIZE;
	int ttl = DEFAULT_TTL;

	ConnectSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);


	struct addrinfo* result = NULL, * ptr = NULL, hints;
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_RETURN_TTL;

	iResult = getaddrinfo(m_ipAddress.c_str(), NULL, &hints, &result);
	char host[512];
	memset(host, 0, sizeof(host));
	int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);

	ICMPHeader* send_buf = 0;
	IPHeader* recv_buf = 0;

	
	packet_size = max(sizeof(ICMPHeader),
		min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));
	
	allocate_buffers(send_buf, recv_buf, packet_size);

	init_ping_packet(send_buf, packet_size, 0);
	int bwrote = sendto(ConnectSocket, (char*)send_buf, packet_size, 0, result->ai_addr, result->ai_addrlen);

	int fromlen = result->ai_addrlen;
	int bread = recvfrom(ConnectSocket, (char*)recv_buf,
		packet_size + sizeof(IPHeader), 0,
		(sockaddr*)&result->ai_addr, &fromlen);
	if (bread == SOCKET_ERROR) {
		cerr << "read failed: ";
		if (WSAGetLastError() == WSAEMSGSIZE) {
			cerr << "buffer too small" << endl;
		}
		else {
			cerr << "error #" << WSAGetLastError() << endl;
		}
		return -1;
	}

	cout << bwrote;
	freeaddrinfo(result);
	closesocket(ConnectSocket);
	WSACleanup();
	// Resolve the server address and port


	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

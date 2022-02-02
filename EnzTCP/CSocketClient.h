#pragma once
#include "CSocket.h"
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


class CSocketClient : CSocket
{
public:
	CSocketClient();
	CSocketClient(string ipServer);
	~CSocketClient();

	bool ConnectToServer(string ipServer, string, int *pLastError);
	bool ConnectToServer(string ipServer, string sPort);
	bool CheckDevice(string ipAddress,string &hostName);
private:
	void init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no);
	USHORT ip_checksum(USHORT* buffer, int size);
	int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf, int packet_size);
};


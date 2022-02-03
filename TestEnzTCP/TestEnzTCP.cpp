// TestEnzTCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "..\EnzTCP\EnzTCP.h"
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <mutex>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
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
bool Test()
{
	string m_ipAddress = "192.168.0.1";

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
}

typedef void (* FNStartLocalAreaListening)(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int);
typedef void (*FNStopLocalAreaListening)();

FNStartLocalAreaListening pfnPtrStartLocalAreaListening;
FNStopLocalAreaListening pfnPtrStopLocalAreaListening;

mutex mtx;

/* ICMP types */
#define ICMP_ECHOREPLY 0 /* ICMP type: echo reply */
#define ICMP_ECHOREQ 8   /* ICMP type: echo request */

/* definition of ICMP header as per RFC 792 */
typedef struct icmp_hdr {
	u_char icmp_type;      /* type of message */
	u_char icmp_code;      /* type sub code */
	u_short icmp_cksum;    /* ones complement cksum */
	u_short icmp_id;       /* identifier */
	u_short icmp_seq;      /* sequence number */
	char icmp_data[1];     /* data */
} ICMP_HDR, * PICMPHDR, FAR* LPICMPHDR;
#define ICMP_HDR_LEN sizeof(ICMP_HDR) 

/* definition of IP header version 4 as per RFC 791 */
#define IPVERSION 4 
typedef struct ip_hdr {
	u_char ip_hl;          /* header length */
	u_char ip_v;           /* version */
	u_char ip_tos;         /* type of service */
	short ip_len;          /* total length */
	u_short ip_id;         /* identification */
	short ip_off;          /* fragment offset field */
	u_char ip_ttl;         /* time to live */
	u_char ip_p;           /* protocol */
	u_short ip_cksum;      /* checksum */
	struct in_addr ip_src; /* source address */
	struct in_addr ip_dst; /* destination address */
} IP_HDR, * PIP_HDR, * LPIP_HDR;
#define IP_HDR_LEN sizeof(IP_HDR) 
#define PNGBUFSIZE 8192+ICMP_HDR_LEN+IP_HDR_LEN 

/* external functions */
extern void WSAErrMsg(LPSTR);

/* private data */
static ICMP_HDR FAR* lpIcmpHdr; /* pointers into our I/O buffer */
static IP_HDR FAR* lpIpHdr;
static char achIOBuf[PNGBUFSIZE];
static SOCKADDR_IN stFromAddr;
static DWORD lCurrentTime, lRoundTripTime;
/*
 * Function icmp_open()
 *
 * Description: opens an ICMP "raw" socket.
 */
SOCKET icmp_open(void) {
	SOCKET s;
	s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s == SOCKET_ERROR) {
		//WSAErrMsg((LPSTR)"socket(type=SOCK_RAW, protocol=IPROTO_ICMP)");
		return (INVALID_SOCKET);
	}
	return (s);
} /* end icmp_open() */

u_short cksum(u_short FAR* lpBuf, int nLen);
/*
 * Function: icmp_sendto()
 *
 * Description: Initializes an ICMP header, inserts the current
 * time in the ICMP data and initializes the data, then sends
 * the ICMP Echo Request to destination address.
 */
int icmp_sendto(SOCKET s,
	HWND hwnd,
	LPSOCKADDR_IN lpstToAddr,
	int nIcmpId,
	int nIcmpSeq,
	int nEchoDataLen) {
	int nAddrLen = sizeof(SOCKADDR_IN);
	int nRet;
	u_short i;
	char c;
	/*--------------------- init ICMP header -----------------------*/

	lpIcmpHdr = (ICMP_HDR FAR*)achIOBuf;
	lpIcmpHdr->icmp_type = ICMP_ECHOREQ;
	lpIcmpHdr->icmp_code = 0;
	lpIcmpHdr->icmp_cksum = 0;
	lpIcmpHdr->icmp_id = nIcmpId++;
	lpIcmpHdr->icmp_seq = nIcmpSeq++;

	/*--------------------put data into packet------------------------
	 * insert the current time, so we can calculate round-trip time
	 * upon receipt of echo reply (which will echo data we sent)
	 */
	lCurrentTime = GetCurrentTime();
	memcpy(&(achIOBuf[ICMP_HDR_LEN]), &lCurrentTime, sizeof(long));

	/* data length includes the time (but not icmp header) */
	c = ' ';   /* first char: space, right after the time */

	for (i = ICMP_HDR_LEN + sizeof(long);
		((i < (nEchoDataLen + ICMP_HDR_LEN)) && (i < PNGBUFSIZE)); i++) {
		achIOBuf[i] = c;
		c++;
		if (c > '~') /* go up to ASCII 126, then back to 32 */
			c = ' ';
	}

	/*----------------------assign ICMP checksum ----------------------
	 * ICMP checksum includes ICMP header and data, and assumes current
	 * checksum value of zero in header
	 */
	lpIcmpHdr->icmp_cksum =
		cksum((u_short FAR*)lpIcmpHdr, nEchoDataLen + ICMP_HDR_LEN);

	/*--------------------- send ICMP echo request -------------------*/
	nRet = sendto(s,                              /* socket */
		(LPSTR)lpIcmpHdr,                      /* buffer */
		nEchoDataLen + ICMP_HDR_LEN + sizeof(long),/* length */
		0,                                     /* flags */
		(LPSOCKADDR)lpstToAddr,                /* destination */
		sizeof(SOCKADDR_IN));                  /* address length */
	if (nRet == SOCKET_ERROR) {
		return nRet;
	}
	return (nRet);
} /* end icmp_sendto() */

/*
 * Function: icmp_recvfrom()
 *
 * Description:
 * receive icmp echo reply, parse the reply packet to remove the
 * send time from the ICMP data.
 */
u_long icmp_recvfrom(SOCKET s,
	LPINT lpnIcmpId,
	LPINT lpnIcmpSeq,
	LPSOCKADDR_IN lpstFromAddr) {
	u_long lSendTime;
	int nAddrLen = sizeof(struct sockaddr_in);
	int nRet, i;
	/*-------------------- receive ICMP echo reply ------------------*/

	stFromAddr.sin_family = AF_INET;
	stFromAddr.sin_addr.s_addr = INADDR_ANY; /*not used on input anyway*/
	stFromAddr.sin_port = 0; /* port not used in ICMP */

	nRet = recvfrom(s,                                 /* socket */
		(LPSTR)achIOBuf,                                 /* buffer */
		PNGBUFSIZE + ICMP_HDR_LEN + sizeof(long) + IP_HDR_LEN, /* length */
		0,                                               /* flags  */
		(LPSOCKADDR)lpstFromAddr,                        /* source */
		&nAddrLen);                                      /* addrlen*/
	if (nRet == SOCKET_ERROR) {
	//	WSAErrMsg((LPSTR)"recvfrom()");
	}

	/*------------------------- parse data ---------------------------
	 * remove the time from data for return.
	 * NOTE: the data received and sent may be asymmetric, as they
	 * are in Berkeley Sockets. As a reusult, we may receive
	 * the IP header, although we didn't send it. This subtlety is
	 * not often implemented so we do a quick check of the data
	 * received to see if it includes the IP header (we look for 0x45
	 * value in first byte of buffer to check if IP header present).
	 */

	 /* figure out the offset to data */
	if (achIOBuf[0] == 0x45) { /* IP header present? */
		i = IP_HDR_LEN + ICMP_HDR_LEN;
		lpIcmpHdr = (LPICMPHDR) & (achIOBuf[IP_HDR_LEN]);
	}
	else {
		i = ICMP_HDR_LEN;
		lpIcmpHdr = (LPICMPHDR)achIOBuf;
	}

	/* pull out the ICMP ID and Sequence numbers */
	*lpnIcmpId = lpIcmpHdr->icmp_id;
	*lpnIcmpSeq = lpIcmpHdr->icmp_seq;

	/* remove the send time from the ICMP data */
	memcpy(&lSendTime, (&achIOBuf[i]), sizeof(u_long));

	return (lSendTime);
} /* end icmp_recvfrom() */

/*
 * Function: cksum()
 *
 * Description:
 * Calculate Internet checksum for data buffer and length (one's
 * complement sum of 16-bit words). Used in IP, ICMP, UDP, IGMP.
 */
u_short cksum(u_short FAR* lpBuf, int nLen) {
	register long lSum = 0L; /* work variables */

	/* note: to handle odd number of bytes, last (even) byte in
	 * buffer have a value of 0 (we assume that it does)
	 */
	while (nLen > 0) {
		lSum += *(lpBuf++); /* add word value to sum */
		nLen -= 2; /* decrement byte count by 2 */
	}

	/* put 32-bit sum into 16-bits */
	lSum = (lSum & 0xffff) + (lSum >> 16);
	lSum += (lSum >> 16);

	/* return Internet checksum. Note:integral type
	 * conversion warning is expected here. It's ok.
	 */
	return (~lSum);
}

void CallbackLocalArea(const char* ip, const char* host, bool bOpen)
{
	if (bOpen)
		cout << ip << " " << host << " " << bOpen << endl;

}
int main()
{
/*	int i = 1;
	string m_ipAddress;
	m_ipAddress = "192.168.0.";
	while (i <= 254)
	{
		WSADATA wsaData;
		int iResult;
		SOCKET socket = INVALID_SOCKET;
		
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			return false;
		}
		memset(achIOBuf, 0, sizeof(achIOBuf));

		socket = icmp_open();
		
		struct addrinfo* result = NULL, * ptr = NULL, hints;
		string ip = m_ipAddress + to_string(i);
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_RAW;
		hints.ai_protocol = IPPROTO_ICMP;
		hints.ai_flags = AI_PASSIVE;

		iResult = getaddrinfo(ip.c_str(), NULL, &hints, &result);

		if (icmp_sendto(socket, NULL, (LPSOCKADDR_IN)result->ai_addr, i-1, i-1, result->ai_addrlen) != SOCKET_ERROR)
		{
			int id, seq;

			icmp_recvfrom(socket, &id, &seq, (LPSOCKADDR_IN)result->ai_addr);

			if (lpIcmpHdr->icmp_data[0] != NULL)
				cout << ip << " is Connected" << endl;
			else
				cout << ip << " is not Connected" << endl;
		}
		else
		{
			cout << ip << " is not Connected" << endl;
		}
		


		freeaddrinfo(result);
		result = NULL;
		

		i++;
	
	closesocket(socket);
	WSACleanup();
	}*/
 /* end cksum() */
	//Test();
	//_getch();
	//_getch();
	HMODULE dll_handle = LoadLibraryA("EnzTCP.dll");
	if (dll_handle)
	{


		pfnPtrStartLocalAreaListening = (FNStartLocalAreaListening)GetProcAddress(dll_handle, "StartLocalAreaListening");
		pfnPtrStopLocalAreaListening = (FNStopLocalAreaListening)GetProcAddress(dll_handle, "StopLocalAreaListening");
	}
	pfnPtrStartLocalAreaListening("192.168.0.1", CallbackLocalArea, 5000);
	_getch();

	pfnPtrStopLocalAreaListening();
	_getch();
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

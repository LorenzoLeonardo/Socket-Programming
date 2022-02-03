#include "pch.h"
#include "CSocketClient.h"
#include <mutex>
#include <iostream>
CRITICAL_SECTION CriticalSection;
CSocketClient::CSocketClient()
{
	memset(achIOBuf, 0, sizeof(achIOBuf));
}
CSocketClient::CSocketClient(string ipServer)
{
	m_ipAddress = ipServer;
	InitializeCriticalSection(&CriticalSection);

}
CSocketClient::~CSocketClient()
{
	DeleteCriticalSection(&CriticalSection);
}
bool CSocketClient::CheckDevice(string ipAddress, string &hostname)
{
	WSADATA wsaData;
	int iResult;
	SOCKET socket = INVALID_SOCKET;
	bool bRet = false;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return false;
	}
	memset(achIOBuf, 0, sizeof(achIOBuf));

	socket = icmp_open();

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(ipAddress.c_str(), NULL, &hints, &result);

	char host[512];
	memset(host, 0, sizeof(host));
	int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);
	hostname = host;

	if (icmp_sendto(socket, NULL, (LPSOCKADDR_IN)result->ai_addr, 0, 0, result->ai_addrlen) != SOCKET_ERROR)
	{
		int id, seq;

		icmp_recvfrom(socket, &id, &seq, (LPSOCKADDR_IN)result->ai_addr);

		if (lpIcmpHdr->icmp_data[0] != NULL)
			bRet = true;
		else
			bRet = false;
	}
	else
	{
		bRet = false;
	}

	freeaddrinfo(result);

	closesocket(socket);
	WSACleanup();
	return bRet;
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

int CSocketClient::decode_reply(IPHeader* reply, int bytes)
{
	// Skip ahead to the ICMP header within the IP packet
	unsigned short header_len = reply->h_len * 4;
	ICMPHeader* icmphdr = (ICMPHeader*)((char*)reply + header_len);

	// Make sure the reply is sane
	if (bytes < header_len + ICMP_MIN) 
	{
		return -1;
	}
	else if (icmphdr->type != ICMP_ECHO_REPLY) 
	{
		if (icmphdr->type != ICMP_TTL_EXPIRE) 
		{
			return -1;
		}
	}
	else if (icmphdr->id != (USHORT)GetCurrentProcessId()) 
	{
		return -2;
	}
	return 0;
}
int CSocketClient::allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf, int packet_size)
{
	// First the send buffer
	send_buf = (ICMPHeader*)new char[packet_size ];
	memset(send_buf, 0, packet_size);
	if (send_buf == 0) {
		//cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	// And then the receive buffer
	recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
	memset(recv_buf, 0, MAX_PING_PACKET_SIZE);
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

/*
 * Function icmp_open()
 *
 * Description: opens an ICMP "raw" socket.
 */
SOCKET CSocketClient::icmp_open(void) 
{
	SOCKET s;
	s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (s == SOCKET_ERROR) 
	{
		//WSAErrMsg((LPSTR)"socket(type=SOCK_RAW, protocol=IPROTO_ICMP)");
		return (INVALID_SOCKET);
	}
	return (s);
} /* end icmp_open() */
/*
 * Function: icmp_sendto()
 *
 * Description: Initializes an ICMP header, inserts the current
 * time in the ICMP data and initializes the data, then sends
 * the ICMP Echo Request to destination address.
 */
int CSocketClient::icmp_sendto(SOCKET s,
	HWND hwnd,
	LPSOCKADDR_IN lpstToAddr,
	int nIcmpId,
	int nIcmpSeq,
	int nEchoDataLen) 

{
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

u_long CSocketClient::icmp_recvfrom(SOCKET s, LPINT lpnIcmpId, LPINT lpnIcmpSeq, LPSOCKADDR_IN lpstFromAddr) 
{
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
u_short CSocketClient::cksum(u_short FAR* lpBuf, int nLen) 
{
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
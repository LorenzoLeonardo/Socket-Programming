#pragma once
#include "CSocket.h"
#include <string>
using namespace std;


/* ICMP types */
#define ICMP_ECHOREPLY 0 /* ICMP type: echo reply */
#define ICMP_ECHOREQ 8   /* ICMP type: echo request */
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11


/* definition of ICMP header as per RFC 792 */
typedef struct icmp_hdr 
{
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

typedef struct ip_hdr 
{
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

	ICMP_HDR FAR* lpIcmpHdr; /* pointers into our I/O buffer */
	IP_HDR FAR* lpIpHdr;
	char achIOBuf[PNGBUFSIZE];
	SOCKADDR_IN stFromAddr;
	DWORD lCurrentTime, lRoundTripTime;

	u_short cksum(u_short FAR* lpBuf, int nLen);
	u_long icmp_recvfrom(SOCKET s, LPINT lpnIcmpId, LPINT lpnIcmpSeq, LPSOCKADDR_IN lpstFromAddr);
	int icmp_sendto(SOCKET s, HWND hwnd, LPSOCKADDR_IN lpstToAddr, int nIcmpId, int nIcmpSeq, int nEchoDataLen);
	SOCKET icmp_open(void);
};


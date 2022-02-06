#include "pch.h"
#include "CICMP.h"

CICMP::CICMP()
{

}
CICMP::~CICMP()
{

}

bool CICMP::CheckDevice(string ipAddress, string& hostname, string& sMacAddress)
{
    HANDLE hIcmpFile;
    WSADATA wsaData;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[32] = "Data Buffer";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;
    bool bRet = false;
    struct in_addr ReplyAddr;
    int iResult = 0;
    char szhostname[512];
    // Validate the parameters

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
        return false;

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE)
          return false;

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*)malloc(ReplySize);
    if (ReplyBuffer == NULL)
    {
        IcmpCloseHandle(hIcmpFile);
        return false;
    }
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(ipAddress.c_str(), NULL, &hints, &result);
    if(iResult != 0)
    {
        free(ReplyBuffer);
        IcmpCloseHandle(hIcmpFile);
        return false;
    }
   
    memset(szhostname, 0, sizeof(szhostname));
    iResult = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, szhostname, 512, 0, 0, 0);
    if (iResult != 0)
    {
        free(ReplyBuffer);
        freeaddrinfo(result);
        IcmpCloseHandle(hIcmpFile);
        return false;
    }
    hostname = szhostname;
   

    if(inet_pton(AF_INET, ipAddress.c_str(), &ipaddr)!=1)
    {
        free(ReplyBuffer);
        freeaddrinfo(result);
        IcmpCloseHandle(hIcmpFile);
        return false;
    }

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
        NULL, ReplyBuffer, ReplySize, 1000);
    
    if (dwRetVal != 0) 
    {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        IPAddr *pSource = 0;
        
        LPBYTE bPhysAddr;
        
        ULONG MacAddr[2];       /* for 6-byte hardware addresses */
        ULONG PhysAddrLen = 6;

        struct addrinfo* result = NULL, * ptr = NULL, hints;
        int iResult = 0;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        iResult = getaddrinfo("localhost", NULL, &hints, &result);
        {
            char host[512];
            memset(host, 0, sizeof(host));
            int status = getnameinfo(result->ai_addr, (socklen_t)result->ai_addrlen, host, 512, 0, 0, 0);
            
           
            
            freeaddrinfo(result);
            iResult = getaddrinfo(host, NULL, &hints, &result);
            pSource = (IPAddr*)(result->ai_addr->sa_data+2);
        }
       

      
        dwRetVal = SendARP(ReplyAddr.S_un.S_addr, *pSource, MacAddr, &PhysAddrLen);
        if (dwRetVal == NO_ERROR)
        {
            bPhysAddr = (BYTE*)&MacAddr;
            if (PhysAddrLen)
            {
                
                char szMac[32];
                memset(szMac, 0, sizeof(szMac));
                for (int i = 0; i < 6; i++)
                {
                    if (i < 5)
                    {
                        memset(szMac, 0, sizeof(szMac));
                        sprintf_s(szMac, sizeof(szMac), "%02X-", bPhysAddr[i]);
                        sMacAddress += szMac;
                    }
                    else
                    {
                        memset(szMac, 0, sizeof(szMac));
                        sprintf_s(szMac, sizeof(szMac), "%02X", bPhysAddr[i]);
                        sMacAddress += szMac;
                    }
                }
             }
        }
        bRet = true;
    }
    else
        bRet = false;

    free(ReplyBuffer);
    freeaddrinfo(result);
    IcmpCloseHandle(hIcmpFile);
    WSACleanup();
	return bRet;
}
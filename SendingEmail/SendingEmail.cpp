// SendingEmail.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")


using namespace std;

void sendEmail(char* server, char* to, char* from, char* subject, char* message) 
{


    SOCKET sockfd = NULL;
    WSADATA wsaData;
    hostent* host;
    sockaddr_in dest;



    char szSmtpServerName[64] = "";
   int sent;
    char line[256];
    char receive[256];
    memset(receive, 0, sizeof(receive));

    int iResult = 0;
    if (WSAStartup(0x202, &wsaData) != SOCKET_ERROR) 
    {
        struct addrinfo* result = NULL,
            * ptr = NULL,
            hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;


        // Resolve the server address and port
        iResult = getaddrinfo(server, "587", &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed: %d\n", iResult);
            WSACleanup();
            return;
        }

        ptr = result;
        sockfd = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);

        if (sockfd == INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return;
        }
        printf("socket\r\n");

        iResult = connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
        }


            strcpy_s(line, "EHLO ");
            strncat_s(line, server, strlen(server));
            strncat_s(line, "\n",1);

            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" <<WSAGetLastError()<< endl;
                closesocket(sockfd);
                return;
            }
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
            Sleep(500);

            strcpy_s(line, "STARTTLS\n");
            sent = send(sockfd, line, strlen(line), 0);
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
            Sleep(500);


            strcpy_s(line, "EHLO ");
            strncat_s(line, server, strlen(server));
            strncat_s(line, "\n", 1);

            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
            Sleep(500);

            strcpy_s(line, "ENHANCEDSTATUSCODES\n");

            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
            Sleep(500);

            strcpy_s(line, "MAIL FROM:<");
            strncat_s(line, from, strlen(from));
            strncat_s(line, ">\n", 3);
            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            Sleep(500);
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
           


            strcpy_s(line, "RCPT TO:<");
            strncat_s(line, to, strlen(to));
            strncat_s(line, ">\n", 3);
            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            Sleep(500);

            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
        
            strcpy_s(line, "DATA\n");
            sent = (send(sockfd, line, strlen(line), 0));
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            Sleep(500);
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;
  


            strcpy_s(line, "To: ");
            strcat_s(line, to);
            strcat_s(line, "\n");
            strcat_s(line, "From: ");
            strcat_s(line, from);
            strcat_s(line, "\n");
            strcat_s(line, "Subject: ");
            strcat_s(line, subject);
            strcat_s(line, "\n");
            strcat_s(line, message);
            strcat_s(line, "\r\n.\r\n");
            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            Sleep(500);
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;


            strcpy_s(line, "QUIT\n");
            sent = send(sockfd, line, strlen(line), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "mail failed to send. :" << WSAGetLastError() << endl;
                closesocket(sockfd);
                return;
            }
            Sleep(500);
            recv(sockfd, receive, sizeof(receive), 0);
            cout << receive << endl;

    }
    closesocket(sockfd);
    WSACleanup();
    cout << "mail was sent" << endl;
}
int main()
{
    sendEmail((char *)"smtp.gmail.com", (char*)"Lorenzo.Leonardo.IMG@gmail.com", (char*)"enzotechcomputersolutions@gmail.com", (char*)"Test Email", (char*)"Hello Lorenzo");


    std::cout << "Hello World!\n";
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

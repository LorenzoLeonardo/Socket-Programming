#pragma once
#ifdef ENZTCPLIBRARY_EXPORTS
#define ENZTCPLIBRARY_API __declspec(dllexport)
#else
#define ENZTCPLIBRARY_API __declspec(dllimport)
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#define MAX_BUFFER_SIZE 1024

#pragma comment(lib, "Ws2_32.lib")

class  ISocket
{
public:
	virtual SOCKET GetSocket() = 0;
	virtual void   SetClientAddr(struct sockaddr addr) = 0;
	virtual const char* GetIP() = 0;
	virtual const char* GetHostName() = 0;
	virtual const char* Receive() = 0;
	virtual void   Send(char*) = 0;
};



typedef void (*NewConnection)(void*);

extern "C" ENZTCPLIBRARY_API HANDLE  OpenServer(const char * sport, NewConnection);
extern "C" ENZTCPLIBRARY_API  void	 RunServer(HANDLE);
extern "C" ENZTCPLIBRARY_API  void   CloseServer(HANDLE);
extern "C" ENZTCPLIBRARY_API  void   CloseClientConnection(HANDLE);

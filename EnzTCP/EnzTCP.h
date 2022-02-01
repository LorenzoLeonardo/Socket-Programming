#pragma once

#ifdef ENZTCPLIBRARY_EXPORTS
#define ENZTCPLIBRARY_API __declspec(dllexport)
#else
#define ENZTCPLIBRARY_API __declspec(dllimport)
#endif

#include <string>
using namespace std;

typedef void (*NewConnection)(void*);

extern "C" ENZTCPLIBRARY_API HANDLE  OpenServer(const char * sport, NewConnection);
extern "C" ENZTCPLIBRARY_API  void	 RunServer(HANDLE);
extern "C" ENZTCPLIBRARY_API  void   CloseServer(HANDLE);
extern "C" ENZTCPLIBRARY_API  void   CloseClientConnection(HANDLE);

// TestEnzTCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "..\EnzTCP\EnzTCP.h"
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <mutex>
HMODULE dll_handle;
using namespace std;

typedef  HANDLE(*LPEnumOpenPorts)(const char* , int , FuncFindOpenPort);
typedef  void(*LPCleanEnumOpenPorts)(HANDLE hHandle);

LPEnumOpenPorts pfnPtrEnumOpenPorts;
LPCleanEnumOpenPorts pfnPtrCleanEnumOpenPorts;

WCHAR* convert_to_wstring(const char* str);
char* convert_from_wstring(const WCHAR* wstr);

char* convert_from_wstring(const WCHAR* wstr)
{
	int wstr_len = (int)wcslen(wstr);
	int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, NULL, 0, NULL, NULL);
	CHAR* strTo = (CHAR*)malloc((num_chars + 1) * sizeof(CHAR));
	if (strTo)
	{
		WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, strTo, num_chars, NULL, NULL);
		strTo[num_chars] = '\0';
	}
	return strTo;
}
WCHAR* convert_to_wstring(const char* str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), NULL, 0);
	WCHAR* wstrTo = (WCHAR*)malloc(size_needed);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)strlen(str), wstrTo, size_needed);
	return wstrTo;
}
#ifdef _UNICODE
wstring GetLastErrorMessageString(int nGetLastError)
{
	DWORD dwSize = 0;
	TCHAR lpMessage[0xFF];

	dwSize  = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		NULL,                // lpsource
		nGetLastError,                 // message id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		lpMessage,              // output buffer
		sizeof(lpMessage),     // size of msgbuf, bytes
		NULL);

	wstring str(lpMessage);
	return str;
}
#else
sstring GetLastErrorMessageString(int nGetLastError)
{
	DWORD dwSize = 0;
	TCHAR lpMessage[0xFF];

	dwSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		NULL,                // lpsource
		nGetLastError,                 // message id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		lpMessage,              // output buffer
		sizeof(lpMessage),     // size of msgbuf, bytes
		NULL);

	sstring str(lpMessage);
	return str;
}
#endif



mutex mt;
void CallBackFindOpenPort(char* ipAddress, int nPort, bool isOpen, int nGetLastError)
{
	mt.lock();
	if(isOpen)
		wcout << convert_to_wstring(ipAddress) << " port:" << nPort << " is " << isOpen << endl;
	else
		wcout << convert_to_wstring(ipAddress) << L" port:" << nPort << L" "<< GetLastErrorMessageString(nGetLastError);

	mt.unlock();
}
int main()
{

	dll_handle = LoadLibrary(L"EnzTCP.dll");
	if (dll_handle)
	{

		pfnPtrEnumOpenPorts = (LPEnumOpenPorts)GetProcAddress(dll_handle, "EnumOpenPorts");
		pfnPtrCleanEnumOpenPorts = (LPCleanEnumOpenPorts)GetProcAddress(dll_handle, "CleanEnumOpenPorts");
	}
	HANDLE h = pfnPtrEnumOpenPorts("192.168.0.1", 5000, CallBackFindOpenPort);
	
	_getch();

	pfnPtrCleanEnumOpenPorts(h);
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

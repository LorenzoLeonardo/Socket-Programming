// TestEnzTCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "..\EnzTCP\EnzTCP.h"
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <mutex>
HMODULE dll_handle;
using namespace std;

typedef  void(*LPCheckOpenPorts)(const char* , int , FuncFindOpenPort);


LPCheckOpenPorts pfnPtrCheckOpenPorts;

mutex mt;
void CallBackFindOpenPort(char* ipAddress, int nPort, bool isOpen)
{
	mt.lock();
	cout << ipAddress << " port:" << nPort << " is " << isOpen << endl;
	mt.unlock();
}
int main()
{

	dll_handle = LoadLibrary(L"EnzTCP.dll");
	if (dll_handle)
	{
		pfnPtrCheckOpenPorts = (LPCheckOpenPorts)GetProcAddress(dll_handle, "CheckOpenPorts");
	}
	pfnPtrCheckOpenPorts("192.168.0.1", 5000, CallBackFindOpenPort);
	
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

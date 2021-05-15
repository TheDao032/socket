// MasterServer.cpp : This file contains the '_tmain' function. Program execution begins and ends there.
//

#include <iostream>

#include "stdafx.h"
#include "MasterServer.h"
#include "afxsock.h"
#include "FileInfo.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW

#endif

// The one and only application object

CWinApp theApp;



static FILESERVER* file_server = new FILESERVER;
static int Number_Of_File = 0;

DWORD WINAPI clientThread(LPVOID arg)
{
	SOCKET* hConnected = (SOCKET*)arg;
	CSocket mysock;

	mysock.Attach(*hConnected);

	int number_continue = 0;

	do {
		mysock.Receive(&number_continue, sizeof(number_continue), 0);
	} while (number_continue);
	delete hConnected;
	return 0;
}

DWORD WINAPI fileServerThread(LPVOID arg)
{

	cout << "Client Is Connected" << endl;

	SOCKET* hConnected = (SOCKET*)arg;
	CSocket mysock;

	mysock.Attach(*hConnected);

	int NumberOfFile;
	FILESERVER file;

	mysock.Receive(&NumberOfFile, sizeof(NumberOfFile), 0);


	file.fileName = new string[NumberOfFile];

	mysock.Receive(&file, sizeof(file), 0);

	file_server[Number_Of_File] = file;
	Number_Of_File++;

	delete hConnected;
	return 0;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			AfxSocketInit(NULL);
			CSocket server, s;
			DWORD threadID;
			HANDLE threadStatus;

			server.Create(4567);
			do {
				cout << "Server Is Listening From client" << endl;
				server.Listen();
				server.Accept(s);
				
				int checkThread;

				s.Receive(&checkThread, sizeof(checkThread), 0);

				SOCKET* hConnected = new SOCKET();
				
				*hConnected = s.Detach();

				if (checkThread == 2)
				{
					threadStatus = CreateThread(NULL, 0, fileServerThread, hConnected, 0, &threadID);
				}
				else
				{
					threadStatus = CreateThread(NULL, 0, clientThread, hConnected, 0, &threadID);
				}
				
			} while (1);
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
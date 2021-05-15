// MasterServer.cpp : This file contains the '_tmain' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

#include "stdafx.h"
#include "FileServer.h"
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
			CSocket masterClient;
			masterClient.Create();
			masterClient.Connect(_T("127.0.0.1"), 4567);

			// Ket noi den Server
			int confirmThread = 2;

			masterClient.Send(&confirmThread, sizeof(confirmThread), 0);

			
			int NumberOfFile;
			cout << "Input Number Of File: ";
			cin >> NumberOfFile;
			masterClient.Send(&NumberOfFile, sizeof(NumberOfFile), 0);

			FILESERVER* file = new FILESERVER[NumberOfFile];

			int size = 200;

			char* fileName = (char*)malloc(sizeof(char) * size);
			int numberOfFile = 0;
			do {
				cout << "Input File Name: ";
				cin >> fileName;
				if (fileName == "0")
				{
					break;
				}
				cin.clear();
				file[numberOfFile].fileName = fileName;
				numberOfFile++;
				//fstream fileServer("C:\\ticket\\ticket.txt");
			} while (numberOfFile < NumberOfFile);
			
			masterClient.Send(&file, sizeof(file), 0);

			AfxSocketInit(NULL);
			CSocket server, s;
			DWORD threadID;
			HANDLE threadStatus;

			server.Create(1234);
			do {
				cout << "Server Is Listening From client" << endl;
				server.Listen();
				server.Accept(s);

				SOCKET* hConnected = new SOCKET();

				*hConnected = s.Detach();

				int checkThread;

				s.Receive(&checkThread, sizeof(checkThread), 0);

				threadStatus = CreateThread(NULL, 0, clientThread, hConnected, 0, &threadID);
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
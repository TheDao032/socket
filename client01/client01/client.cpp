// MasterServer.cpp : This file contains the '_tmain' function. Program execution begins and ends there.
//
#pragma warning(disable : 4996)
#define _XOPEN_SOURCE 700

#include <iostream>
#include <fstream>

#include "stdafx.h"
#include "FileServer.h"
#include "afxsock.h"
#include "FileInfo.h"
#include "dirent.h"
#include <stdio.h>
#include <sys/types.h>
#include <vector>
#include <string>




using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW

#endif

// The one and only application object

CWinApp theApp;

static vector<int> PORTFILE;
static FILESERVER* file_server = new FILESERVER;
static int Number_Of_File = 0;

int64_t GetFileSize(const std::string& fileName) {
	// no idea how to get filesizes > 2.1 GB in a C++ kind-of way.
	// I will cheat and use Microsoft's C-style file API
	FILE* f;
	if (fopen_s(&f, fileName.c_str(), "rb") != 0) {
		return -1;
	}
	_fseeki64(f, 0, SEEK_END);
	const int64_t len = _ftelli64(f);
	fclose(f);
	return len;
}

///
/// Recieves data in to buffer until bufferSize value is met
///
int RecvBuffer(SOCKET s, char* buffer, int bufferSize, int chunkSize = 4 * 1024) {
	int i = 0;
	while (i < bufferSize) {
		const int l = recv(s, &buffer[i], __min(chunkSize, bufferSize - i), 0);
		if (l < 0) { return l; } // this is an error
		i += l;
	}
	return i;
}

///
/// Sends data in buffer until bufferSize value is met
///
int SendBuffer(SOCKET s, const char* buffer, int bufferSize, int chunkSize = 4 * 1024) {

	int i = 0;
	while (i < bufferSize) {
		const int l = send(s, &buffer[i], __min(chunkSize, bufferSize - i), 0);
		if (l < 0) { return l; } // this is an error
		i += l;
	}
	return i;
}

//
// Sends a file
// returns size of file if success
// returns -1 if file couldn't be opened for input
// returns -2 if couldn't send file length properly
// returns -3 if file couldn't be sent properly
//
int64_t SendFile(SOCKET s, const std::string& fileName, int chunkSize = 64 * 1024) {

	const int64_t fileSize = GetFileSize(fileName);
	if (fileSize < 0) { return -1; }

	std::ifstream file(fileName, std::ifstream::binary);
	if (file.fail()) { return -1; }

	if (SendBuffer(s, reinterpret_cast<const char*>(&fileSize),
		sizeof(fileSize)) != sizeof(fileSize)) {
		return -2;
	}

	char* buffer = new char[chunkSize];
	bool errored = false;
	int64_t i = fileSize;
	while (i != 0) {
		const int64_t ssize = __min(i, (int64_t)chunkSize);
		if (!file.read(buffer, ssize)) { errored = true; break; }
		const int l = SendBuffer(s, buffer, (int)ssize);
		if (l < 0) { errored = true; break; }
		i -= l;
	}
	delete[] buffer;

	file.close();

	return errored ? -3 : fileSize;
}

int CountFile()
{
	struct dirent* d;
	int i = -2;
	DIR* dr;
	dr = opendir("./file");
	if (dr != NULL)
	{
		cout << "List of Files & Folders:-\n";
		for (d = readdir(dr); d != NULL; d = readdir(dr))
		{
			cout << d->d_name << endl;
			i++;
		}
		closedir(dr);
	}
	else
		cout << "\nError Occurred!";
	
	cout << endl;

	return i;
}

vector<string> GetNameFile()
{
	struct dirent* d;
	vector<string> str;
	int i = -2;
	DIR* dr;
	dr = opendir("./file");
	if (dr != NULL)
	{
		for (d = readdir(dr); d != NULL; d = readdir(dr))
		{
			i++;
			if (i > 0) {
				str.push_back(d->d_name);
			}
		}
		closedir(dr);
	}
	else
		cout << "\nError Occurred!";

	cout << endl;

	return str;
}

//
// Receives a file
// returns size of file if success
// returns -1 if file couldn't be opened for output
// returns -2 if couldn't receive file length properly
// returns -3 if couldn't receive file properly
//
int64_t RecvFile(SOCKET s, const std::string& fileName, int chunkSize = 64 * 1024) {
	std::ofstream file(fileName, std::ofstream::binary);
	if (file.fail()) { return -1; }

	int64_t fileSize;
	if (RecvBuffer(s, reinterpret_cast<char*>(&fileSize),
		sizeof(fileSize)) != sizeof(fileSize)) {
		return -2;
	}

	char* buffer = new char[chunkSize];
	bool errored = false;
	int64_t i = fileSize;
	while (i != 0) {
		const int r = RecvBuffer(s, buffer, (int)__min(i, (int64_t)chunkSize));
		if ((r < 0) || !file.write(buffer, r)) { errored = true; break; }
		i -= r;
	}
	delete[] buffer;

	file.close();

	return errored ? -3 : fileSize;
}



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
			// connect to master server 2
			AfxSocketInit(NULL);
			
			do {
				cout << "=====================MAIN=========================" << endl;
				cout << "Get Info port ip file server in MasterServer" << endl;
				cout << "input: 1" << endl;
				cout << "Connect to file server" << endl;
				cout << "input 2" << endl;
				int a;
				cout << "You input: ";
				cin >> a;

				CSocket FileServer;
				CSocket masterClient;
				string ip;
				int portF;
				int numberPort;
				int portNumber;
				int NumberOfFile;
				if (a == 1) {

					masterClient.Create();
					masterClient.Connect(_T("127.0.0.1"), 4567);
					// Ket noi den Server
					int confirmThread = 1;

					masterClient.Send(&confirmThread, sizeof(confirmThread), 0);


					masterClient.Receive(&numberPort, sizeof(numberPort), 0);

					for (int i = 0; i < numberPort; i++) {
						masterClient.Receive(&portNumber, sizeof(portNumber), 0);
						PORTFILE.push_back(portNumber);
					}
					cout << "---List ip and port of file server---" << endl;
					for (int i = 0; i < PORTFILE.size(); i++) {
						cout << "ip: 127.0.0.1 | port :" << PORTFILE[i] << endl;
					}

				}
				if (a == 2) {
					cout << "Input ip and port to connect File Servet" << endl;
					cout << "ip: ";
					cin >> ip;
					cout << "port:";
					cin >> portF;
					FileServer.Create();
					FileServer.Connect(_T("127.0.0.1"), portF);

					// nhan so luong file
					FileServer.Receive(&NumberOfFile, sizeof(NumberOfFile), 0);

					char* Question;
					int len;
					string name;
					cout << "Name file" << endl;
					for (int i = 0; i < NumberOfFile; i++) {
						FileServer.Receive((char*)&len, sizeof(int), 0);
						Question = new char[len + 1];
						Question[len] = '\0';
						FileServer.Receive((char*)Question, len, 0);
						cout << Question << endl;
					}

					cout << "input name file download: " << endl;
					cout << "name: ";
					cin >> name;

					std::vector<char> chars(name.begin(), name.end());
					chars.push_back('\0');

					char* nameok = &chars[0];

					FileServer.Send(&len, sizeof(int), 0);
					FileServer.Send(nameok, len, 0);

					int linefile;

					FileServer.Receive((char*)&linefile, sizeof(linefile), 0);

					cout << "File download:" << endl;

					ofstream myfile;
					myfile.open(name +".txt");
					for (int i = 0; i < linefile; i++) {
						FileServer.Receive((char*)&len, sizeof(int), 0);
						Question = new char[len + 1];
						Question[len] = '\0';
						FileServer.Receive((char*)Question, len, 0);
						cout << Question << endl;
						myfile << Question << "\n";
					}
					myfile.close();					
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


#pragma once

#include <iostream>
using namespace std;

#ifndef _FILE_INFO_
#define _FILE_INFO_

typedef struct fileServer {
	const int serverPort = 1234;
	const char* serverIP = "127.0.0.1";
	char* fileName;
} FILESERVER;

#endif
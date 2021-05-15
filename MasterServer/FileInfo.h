#pragma once
#include <iostream>
using namespace std;

#ifndef _FILE_INFO_
#define _FILE_INFO

typedef struct fileServer {
	int serverPort;
	char* serverIP;
	char* fileName;
} FILESERVER;

#endif
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

typedef struct SockInterfaceStruct {
	SOCKET      hSock;
	SOCKADDR_IN hServAddr;
	int         iLenServAddr;

	int    iPortNum;
	char   cIPAddr[16]; // IP4 xxx.xxx.xxx.xxx
} SockObject;


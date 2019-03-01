#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Network related includes
//#undef UNICODE

// Need older API support e.g. IPV4. Else result in "Error E0040"
// E0040 expected an identifier ws2def.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <winsock2.h>
#include <ws2tcpip.h> // getaddrinfo, includes #include <winsock2.h>
#include <stdio.h>

#include "SocketUDP.h"

#pragma comment (lib, "Ws2_32.lib")

// Network macros
//
#ifndef MAX_UDP_DATA_SIZE 
#define MAX_UDP_DATA_SIZE (65000u)
#endif

#define TAG_SOCK "LocSock: "

// Socket static variable
static SOCKET s;
static SOCKADDR_IN servaddr;

int SocketUDP_RecvFrom(SOCKET *phSock, char *pDataBuf, int iDataSize,
    sockaddr *pSockCliAddr, int *pSockSize) 
{
  int iRetVal;

  // Packet details
  char *pRecvBuf;
  int iRxLen;
  int iAccRxLen;

  pRecvBuf = pDataBuf;
  iAccRxLen = 0;

  while(iAccRxLen < iDataSize) {

    iRxLen = iDataSize - iAccRxLen;

    if (iRxLen > MAX_UDP_DATA_SIZE) { iRxLen = MAX_UDP_DATA_SIZE; }

    //printf(TAG_SOCK "B4 RecvFrom: len:%d AccLen:%d\n", iRxLen, iAccRxLen);
    iRetVal = recvfrom(*phSock, pRecvBuf, iRxLen, 0, pSockCliAddr, pSockSize);
    if (iRetVal < 0) { return -1; }

    pRecvBuf  += iRetVal;
    iAccRxLen += iRetVal;
    //printf(TAG_SOCK "A4 RecvFrom: len:%d AccLen:%d\n", iRxLen, iAccRxLen);

    if (iRetVal < iRxLen) { break; }
  }

  return iAccRxLen;
}

int SocketUDP_SendTo(SOCKET *phSock, char *pDataBuf, int iDataSize, 
    sockaddr *pSockDestAddr, int iSockSize) 
{

  int iRetVal;

  // Packet details
  const char *pSendBuf;
  int iTxLen;
  int iAccTxLen;

  pSendBuf    = pDataBuf;
  iAccTxLen = 0;

  while(iAccTxLen < iDataSize) {

    iTxLen = iDataSize - iAccTxLen;

    if (iTxLen > MAX_UDP_DATA_SIZE) { iTxLen = MAX_UDP_DATA_SIZE; }

    //printf(TAG_SOCK "SendTo %l\n", iTxLen);
    iRetVal = sendto(*phSock, pSendBuf, iTxLen, 0, pSockDestAddr, iSockSize);
    if (iRetVal < 0) { return -1; }

    pSendBuf  += iRetVal;
    iAccTxLen += iRetVal;

    if (iRetVal < iTxLen) { 
      // Should not happend, Track it!!. 
      printf(TAG_SOCK "WARNING: Sending %d / %d\n", iAccTxLen, iDataSize);
      break; 
    }
  }

  return iAccTxLen;
}

//int SocketUDP_ClientRecv(SOCKET *phSock, SOCKADDR_IN *phServAddr, int *iLenAddr, char *pDataBuf, int iDataSize)
//int SocketUDP_ClientRecv(SOCKET *phSock, SOCKADDR_IN *phServAddr, int *iLenAddr, char *pDataBuf, int iDataSize)
//{  //return SocketUDP_RecvFrom(phSock, pDataBuf, iDataSize, NULL, NULL);}

int SocketUDP_ClientRecv(SockObject *pSockObj, char *pDataBuf, int iDataSize)
{

	SOCKET *phSock = &pSockObj->hSock;
	SOCKADDR_IN *phServAddr = &pSockObj->hServAddr;
	int *pSockSize = &pSockObj->iLenServAddr;
	*pSockSize = sizeof(SOCKADDR_IN);

  return SocketUDP_RecvFrom(phSock, pDataBuf, iDataSize, (sockaddr *) phServAddr, pSockSize);
}


//int SocketUDP_ClientSend(SOCKET *phSock, char *pDataBuf, int iDataSize)
//int SocketUDP_ClientSend(SOCKET *phSock, SOCKADDR_IN *phServAddr, char *pDataBuf, int iDataSize)

int SocketUDP_ClientSend(SockObject *pSockObj, char *pDataBuf, int iDataSize)
{
	SOCKET *phSock = &pSockObj->hSock;
	SOCKADDR_IN *phServAddr = &pSockObj->hServAddr;
	int iSockSize = sizeof(SOCKADDR_IN); // TODO pSockObj->iLenServAddr;

  return SocketUDP_SendTo(phSock, pDataBuf, iDataSize, (sockaddr *) phServAddr, iSockSize);
}

int SocketUDP_Deinit(SOCKET *phSock)
{
  // deinit the network connection
  closesocket(*phSock);
  WSACleanup();

  printf("localize network_deinit ends\n");

  return 0;
}


int SocketUDP_InitClient(SockObject *pSockObj)
{
	WSADATA wsaData;
	int iRetVal;

	SOCKET *phSock;
	SOCKADDR_IN *phServAddr;

	// Initialize Winsock
	iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRetVal != 0) { goto ret_err; }

	phSock = &(pSockObj->hSock);
	phServAddr = &(pSockObj->hServAddr);

	// CREATE SOCKET and SAVE in module Object
	*phSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (pSockObj->hSock == INVALID_SOCKET) { goto ret_err; }

	memset((char *)&(pSockObj->hServAddr), 0, sizeof(SOCKADDR_IN)); // IMP: to clear server addr 

	phServAddr->sin_family = AF_INET;
	phServAddr->sin_port = htons(pSockObj->iPortNum); //Port to connect on
	phServAddr->sin_addr.s_addr = inet_addr(pSockObj->cIPAddr);

	// connect to server 
	iRetVal = connect(*phSock, (struct sockaddr *)phServAddr, sizeof(SOCKADDR_IN));
	if (iRetVal < 0) { goto ret_err; }
	printf(TAG_SOCK "Connected to Server: %s:%d\n", pSockObj->cIPAddr, pSockObj->iPortNum);

	return 0;

ret_err:
	printf(TAG_SOCK "Error in SocketUDP_ClientInit: %d \n", WSAGetLastError());
	return -1;
}

#if 0
int SocketUDP_InitClient(SOCKET *phSock, SOCKADDR_IN *phServAddr,
    int  iPortNum,   char *pServerIP)
{
  WSADATA wsaData;
  int iRetVal;

  // Initialize Winsock
  iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iRetVal != 0) { goto ret_err; }

  // CREATE SOCKET and SAVE in module Object
  *phSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (*phSock == INVALID_SOCKET) { goto ret_err; }

  // IMP: to clear server addr 
  memset(phServAddr, 0, sizeof(SOCKADDR_IN));

  phServAddr->sin_family      = AF_INET;
  phServAddr->sin_port        = htons(iPortNum); //Port to connect on
  phServAddr->sin_addr.s_addr = inet_addr(pServerIP);

  // connect to server 
  iRetVal = connect(*phSock, (struct sockaddr *)phServAddr, sizeof(SOCKADDR_IN));
  if (iRetVal < 0) { goto ret_err; }
  printf(TAG_SOCK "Connected to Server: %s:%d\n", pServerIP, iPortNum);

  return 0;

ret_err:
  printf(TAG_SOCK "Error in SocketUDP_ClientInit: %d \n", WSAGetLastError());
  return -1;
}
#endif
#if 0
int SocketUDP_ServerInit()
{
  WSADATA wsaData;
  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    printf(TAG_SOCK "WSAStartup failed with error: %d\n", iResult);
    return 1;
  }

  //Create socket
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s == INVALID_SOCKET) {
    printf(TAG_SOCK "Error localize_network_input_init socket %d\n", WSAGetLastError());
    return false; //Couldn't create the socket
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(INPUT_SERVER_PORT); //Port to connect on
  servaddr.sin_addr.s_addr = inet_addr(INPUT_SERVER_IP);
  servaddr.sin_port = IPPROTO_UDP;

  // Setup the UDP listening socket
  iResult = bind(s, (struct sockaddr*) &servaddr, (int)sizeof(servaddr));
  if (iResult < 0) {
    printf(TAG_SOCK "Error: bind failed with error: %d\n", WSAGetLastError());
    closesocket(s);
    WSACleanup();
    return 1;
  }

  return 0;
}
#endif


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

#pragma comment (lib, "Ws2_32.lib")

// Network macros
#define MAX_UDP_DATA_SIZE (65000u)

#define DEFAULT_BUFLEN 100
#define DEFAULT_PORT "27015"

#define TAG_SOCK "Sock: "
// 
// Server with UDP connection
//
#define INPUT_SERVER_PORT     27015 
#define INPUT_SERVER_IP     "127.0.0.1"

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
    sockaddr *pSockClientAddr, int iSockSize) 
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
    iRetVal = sendto(*phSock, pSendBuf, iTxLen, 0, pSockClientAddr, iSockSize);
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

int SocketUDP_ClientRecv(SOCKET *phSock, char *pDataBuf, int iDataSize)
{
  return SocketUDP_RecvFrom(phSock, pDataBuf, iDataSize, NULL, NULL);
}


int SocketUDP_ClientSend(SOCKET *phSock, char *pDataBuf, int iDataSize)
{
  return SocketUDP_SendTo(phSock, pDataBuf, iDataSize, NULL, 0);
}

int SocketUDP_Deinit(SOCKET *phSock)
{
  // deinit the network connection
  closesocket(*phSock);
  WSACleanup();

  printf("localize network_deinit ends\n");

  return 0;
}


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
  phServAddr->sin_port        = IPPROTO_UDP;

  // connect to server 
  iRetVal = connect(*phSock, (struct sockaddr *)phServAddr, sizeof(SOCKADDR_IN));
  if (iRetVal < 0) { goto ret_err; }

  return 0;

ret_err:
  printf(TAG_SOCK "Error in SocketUDP_ClientInit: %d \n", WSAGetLastError());
  return -1;
}

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



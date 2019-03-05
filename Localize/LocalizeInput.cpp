#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

// Threads: to run streaming server
#include <process.h>

#include <vector>
#include <iostream>
#include <iomanip>

// File operation
#include <fstream> 

// Threads: to run streaming server
//#include <process.h>

//#include <opencv2/highgui.hpp>

#include "DataFormat.h"
#include "LocalizeObject.h"


using namespace std;

// MACROS
#define TAG_LOUT "LocIn: " // Localize module OUTPUT

// GLOBAL VARIABLES

static ofstream myFile;


//// OUTPUT TO FILE /////
int file_open() {

  // File: out, binary, not appended, thus will reset size to zero
  myFile.open("..\\localize_out_right.mjpeg",
    ios::out | ios::binary); // | ios::app to append
  if (!myFile.is_open()) {
    std::cout << "Input operation not successful\n";
    return -1;
  }
  return 0;
}

int file_write(std::vector<unsigned char> &buf) {
  char* data = reinterpret_cast<char*>(buf.data());

  // Program assume the file is already opened.
  myFile.write(data, buf.size());
  return 0;
}

int LocalizeInput_GetGTData(LocalizeObject *pLocObj)
{
	int iRetVal;

	// Network related variables
	//sockaddr sockClientAddr;
	//int iLenSockClient = sizeof(sockaddr_in);
	SOCKET *phSock;
	SOCKADDR_IN *phServAddr;

	int iPktLen;
	char *pPktBuf;

	// REQUEST holder
	char ucReqMsg[11] = REQ_STREAM; // +1 to add null at last

									// Packet details
	StereoPacket   *pStereoPkt;
	SockObject     *phSockObj;

	LocalizePacket *pLocPkt;
	unsigned char  *pFrameL;
	unsigned char  *pFrameR;

	// Assign the object pointers
	pStereoPkt = pLocObj->pStereoPacket;
	pFrameL = pLocObj->pFrameLeft;
	pFrameR = pLocObj->pFrameRight;
	pLocPkt = pLocObj->pLocalizePacket;
	phSock = &(pLocObj->hSockObjStereo.hSock);
	phSockObj = &(pLocObj->hSockObjStereo);

	// streaming buffer address and its length
	pPktBuf = (char *)pStereoPkt;
	iPktLen = sizeof(StereoPacket);

	// SEND THE REQUEST FOR STEREO PACKET
	//iRetVal = SocketUDP_ClientSend(phSock, phServAddr, ucReqMsg, sizeof(ucReqMsg));
	iRetVal = SocketUDP_ClientSend(phSockObj, ucReqMsg, sizeof(ucReqMsg));
	if (iRetVal < 0) { goto ret_err; }

	// RECEIVE STEREO PACKET DATA
	//iRetVal = SocketUDP_ClientRecv(phSock, phServAddr, pPktBuf, iPktLen);
	iRetVal = SocketUDP_ClientRecv(phSockObj, pPktBuf, iPktLen);
	if (iRetVal < 0) { goto ret_err; }

	return 0;
ret_err:
	printf(TAG_LOUT "Error: sending %s, ret:%d, NetErr:%d\n", ucReqMsg, iRetVal, WSAGetLastError());
	return -1;

}


int LocalizeInput_GetStream(LocalizeObject *pLocObj)
{
  int iRetVal;
 
  // Network related variables
  //sockaddr sockClientAddr;
  //int iLenSockClient = sizeof(sockaddr_in);
  SOCKET *phSock;
  SOCKADDR_IN *phServAddr;

  int iPktLen;
  char *pPktBuf;

  // REQUEST holder
  char ucReqMsg[11] = REQ_STREAM; // +1 to add null at last

  // Packet details
  StereoPacket   *pStereoPkt;
  SockObject     *phSockObj;

  LocalizePacket *pLocPkt;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;

  // Assign the object pointers
  pStereoPkt = pLocObj->pStereoPacket;
  pFrameL    = pLocObj->pFrameLeft ;
  pFrameR    = pLocObj->pFrameRight;
  pLocPkt    = pLocObj->pLocalizePacket;
  phSock     = &(pLocObj->hSockObjStereo.hSock);
  phSockObj = &(pLocObj->hSockObjStereo);

  // streaming buffer address and its length
  pPktBuf = (char *)pStereoPkt;
  iPktLen = sizeof(StereoPacket);

  // SEND THE REQUEST FOR STEREO PACKET
  //iRetVal = SocketUDP_ClientSend(phSock, phServAddr, ucReqMsg, sizeof(ucReqMsg));
  iRetVal = SocketUDP_ClientSend(phSockObj, ucReqMsg, sizeof(ucReqMsg));
  if (iRetVal < 0) { goto ret_err; }

  // RECEIVE STEREO PACKET DATA
  //iRetVal = SocketUDP_ClientRecv(phSock, phServAddr, pPktBuf, iPktLen);
  printf(TAG_LOUT " Rx len:%d\n", iPktLen);
  iRetVal = SocketUDP_ClientRecv(phSockObj, pPktBuf, iPktLen);
  if (iRetVal < 0 ) { goto ret_err; }

  return 0;
ret_err:
  printf(TAG_LOUT "Error: sending %s, ret:%d, NetErr:%d\n", ucReqMsg, iRetVal, WSAGetLastError());
  return -1;

}

int LocalizeInput_GetMapObjects(LocalizeObject *pLocObj)
{
  int iRetVal;
 
  // Network related variables
  //sockaddr sockClientAddr;
  //int iLenSockClient = sizeof(sockaddr_in);
  SOCKET *phSock;
  SOCKADDR_IN *phServAddr;

  int iPktLen;
  char *pPktBuf;

  // REQUEST holder
  char ucReqMsg[13] = REQ_TRAFFISIGNS; // +1 to add null at last

  // Packet details
  GTMapPacket   *pGTMapPkt;
  SockObject     *phSockObj;

  LocalizePacket *pLocPkt;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;

  // Assign the object pointers
  pGTMapPkt  = pLocObj->pGTMapPacket;
  pLocPkt    = pLocObj->pLocalizePacket;
  phSock     = &(pLocObj->hSockObjMap.hSock);
  phSockObj = &(pLocObj->hSockObjMap);

 // Fill the packet
 pGTMapPkt->iRequestType = REQ_GTMAP_MARKS;

  // streaming buffer address and its length
  pPktBuf = (char *)pGTMapPkt;
  iPktLen = sizeof(GTMapPacket);

  // SEND THE REQUEST FOR GTMap PACKET
  //iRetVal = SocketUDP_ClientSend(phSock, phServAddr, ucReqMsg, sizeof(ucReqMsg));
  //iRetVal = SocketUDP_ClientSend(phSockObj, ucReqMsg, sizeof(ucReqMsg));
  iRetVal = SocketUDP_ClientSend(phSockObj, pPktBuf, iPktLen);
  if (iRetVal < 0) { goto ret_err; }

  // RECEIVE GTMap PACKET DATA
  //iRetVal = SocketUDP_ClientRecv(phSock, phServAddr, pPktBuf, iPktLen);
  printf(TAG_LOUT "Waiting to Recv from GTMAP. len:%d\n", iPktLen);
  iRetVal = SocketUDP_ClientRecv(phSockObj, pPktBuf, iPktLen);
  if (iRetVal < 0 ) { goto ret_err; }

  return 0;
ret_err:
  printf(TAG_LOUT "Error: sending %s, ret:%d, NetErr:%d\n", ucReqMsg, iRetVal, WSAGetLastError());
  return -1;

}


int LocalizeInput_Deinit(LocalizeObject *pLocObj)
{
  // Socket interfaces
  SOCKET      *phSock;

  phSock     = &pLocObj->hSockObjStereo.hSock;
  SocketUDP_Deinit(phSock);

#ifdef DEBUG_OUTPUT_FILE
  myFile.close();
#endif // DEBUG_OUTPUT_FILE

  return 0;
}

int LocalizeInput_Init(LocalizeObject *pLocObj)
{

  int iRetVal = 0;

  // INPUT StereoPacket.
  StereoPacket   *pStereoPkt;
  GTMapPacket    *pGTMapPkt;
  LocalizePacket *pLocPkt;

  SockObject *phSockObjStereo;
  SockObject *phSockObjMap;
  SockObject *phSockObjIMU;

  unsigned char  *pFrameL;
  unsigned char  *pFrameR;
 
  // Socket interfaces
  SOCKET      *phSock;
  SOCKADDR_IN *phServAddr;
  int         iPortNum;  
  char        cIPAddr[16];

  printf("In LocalizeInput_Init\n");

  // TODO: Ring buffer
  
  // INPUT Buffer
  // Allocate memory stereo packet, i.e. metadata + jpeg frames bytes
  pStereoPkt     = (StereoPacket *) malloc(sizeof(StereoPacket));
  
  // Allocate memory GTMap packet, i.e. request, metadata
  pGTMapPkt     = (GTMapPacket *) malloc(sizeof(GTMapPacket));

  // PROCESSING Buffer
  // Allocate memory for raw frame data
  pFrameL  = (uchar *) malloc(MAX_FRAME_SIZE);
  pFrameR  = (uchar *) malloc(MAX_FRAME_SIZE);
  
  // OUTPUT Buffer
  // Allocate memory the Output packet
  pLocPkt = (LocalizePacket *) malloc(sizeof(StereoPacket));

  // Check if memory allocated
  if ((pStereoPkt == NULL) 
	  || (pFrameL == NULL) 
	  || (pFrameR == NULL) 
	  || (pLocPkt == NULL)) { 
	printf("Error: malloc\n"); return -1; }

  // fill the object
  pLocObj->pStereoPacket   = pStereoPkt;
  pLocObj->pGTMapPacket    = pGTMapPkt;
  pLocObj->pFrameLeft      = pFrameL; 
  pLocObj->pFrameRight     = pFrameR;
  pLocObj->pLocalizePacket = pLocPkt;
  phSockObjStereo = &(pLocObj->hSockObjStereo);
  phSockObjMap = &(pLocObj->hSockObjMap);
  phSockObjIMU = &(pLocObj->hSockObjIMU);

#if 1
  phSockObjStereo->iPortNum = SOCK_PORT_STEREO;
  memcpy(phSockObjStereo->cIPAddr, SOCK_IP_STEREO, strlen(SOCK_IP_STEREO));
  phSockObjStereo->cIPAddr[strlen(SOCK_IP_STEREO)] = '\0';
  iRetVal = SocketUDP_InitClient(phSockObjStereo);
#endif

#if 1
  phSockObjMap->iPortNum = SOCK_PORT_GTMAP;
  memcpy(phSockObjMap->cIPAddr, SOCK_IP_GTMAP, strlen(SOCK_IP_GTMAP));
  phSockObjMap->cIPAddr[strlen(SOCK_IP_STEREO)] = '\0';
  iRetVal = SocketUDP_InitClient(phSockObjMap);
#endif

#if 0
  phSock     = &pLocObj->hSockStereo.hSock;
  phServAddr = &pLocObj->hSockStereo.hServAddr;
  iPortNum   = SOCK_PORT_STEREO;
  memcpy(cIPAddr, SOCK_IP_STEREO, strlen(SOCK_IP_STEREO));

  iRetVal = SocketUDP_InitClient(&pLocObj->hSockStereo, phServAddr, iPortNum, cIPAddr);
#endif

#if 0
  phSock = &pLocObj->hSockMap.hSock;
  phServAddr = &pLocObj->hSockMap.hServAddr;
  iPortNum = SOCK_PORT_GTMAP;
  memcpy(cIPAddr, SOCK_IP_GTMAP, strlen(SOCK_IP_GTMAP));

  iRetVal = SocketUDP_InitClient(phSock, phServAddr, iPortNum, cIPAddr);

#else
 // iRetVal = SocketUDP_ClientInit();

#endif 

  if (iRetVal != 0) {
    printf("Error: In SocketUDP_ClientInit\n");
    return -1;
  }

  printf("SocketUDP_ClientInit... OK\n");

  // Note: OUTPUT buffer is within StereoPacket Metadata
#ifdef DEBUG_OUTPUT_FILE
  file_open();
#endif // DEBUG_OUTPUT_FILE

  return 0;
}

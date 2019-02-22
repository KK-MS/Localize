#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include "DataFormat.h"

#pragma comment (lib, "Ws2_32.lib")


// MACROS
#define Localize_QUALITY_VALUE (95u) // Quality percentage

typedef struct SockInterfaceStruct {
  SOCKET      hSock;
  SOCKADDR_IN hServAddr;

  int    iPortNum;  
  char   cIPAddr[16]; // IP4 xxx.xxx.xxx.xxx
} SockObject;
//
// Localize needs:
// 1. Metadat from IMU and Stereo
// 2. JPEG streaming from Stere
// 3. Output: Calculated localize values along with metadata
// Thus Output is combine along with Metadata.
//
typedef struct LocalizeObjectStruct {
  // INPUT
  // Stereo packet containing meta + jpegs frames. 
  StereoPacket *pStereoPacket;

  // PROCESSING Variables
  unsigned char *pFrameLeft;  // Localize Left camera data 
  unsigned char *pFrameRight; // Localize Right camera data 

  // OUTPUt Variables
  // Pointer to Localize packet
  LocalizePacket *pLocalizePacket;
  
  // INTERFACING Variables
  // Socket static variable
  SockObject hSockStereo;
  SockObject hSockMap;
  SockObject hSockIMU;

} LocalizeObject;

// Localize main execution function declaration

int LocalizeExecute_Start(LocalizeObject *pLocalizeObject);
int LocalizeExecute_Termination(LocalizeObject *pLocalizeObject);

// Localize input processing function declarations
int LocalizeInput_GetStream(LocalizeObject *pLocalizeObject);
int LocalizeInput_Init(LocalizeObject *pLocalizeObject);
int LocalizeInput_Deinit(LocalizeObject *pLocalizeObject);
int LocalizeInput_FromCamera(LocalizeObject *pLocalizeObject);

// Localize output processing function declarations

int LocalizeOutput_Init(LocalizeObject *pLocalizeObject);
int LocalizeOutput_Deinit(LocalizeObject *pLocalizeObject);
int LocalizeOutput_Packet(LocalizeObject *pLocalizeObject);

// Localize data processing function declarations

int LocalizeProcess_ToJpeg(LocalizeObject *pLocalizeObject);
int LocalizeProcess_Request(LocalizeObject *pLocalizeObject);
int LocalizeProcess_JpegToRaw(LocalizeObject *pLocalizeObject);

// Network related API declarations
int SocketUDP_RecvFrom(char *pBuf, int iSize, sockaddr *pSockClientAddr,
	int *pSockSize);
int SocketUDP_SendTo(char *pBuf, int iSize, sockaddr *pSockClientAddr,
	int iSockSize);
int SocketUDP_Deinit(SOCKET*);
int SocketUDP_ClientInit();
int SocketUDP_ServerInit();
int SocketUDP_ClientRecv(SOCKET* , char* , int );
int SocketUDP_ClientSend(SOCKET* , char* , int );
int SocketUDP_InitClient(SOCKET* , SOCKADDR_IN* ,int ,char* );


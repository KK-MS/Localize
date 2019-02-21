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

int LocalizeInput_GetStream(LocalizeObject *pLocalizeObject)
{
  int iRetVal;
 
  // Network related variables
  sockaddr sockClientAddr;
  int iLenSockClient = sizeof(sockaddr_in);
  int iPktLen;
  char *pPktBuf;

  // REQUEST holder
  char ucReqMsg[11] = REQ_STREAM; // +1 to add null at last

  // Packet details
  StereoMetadata *pStereoMeta;
  StereoPacket   *pStereoPkt;
  LocalizePacket *pLocPkt;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;

  // Assign the object pointers
  pStereoPkt = pLocalizeObject->pStereoPacket;
  pFrameL    = pLocalizeObject->pFrameLeft ;
  pFrameR    = pLocalizeObject->pFrameRight;
  pLocPkt    = pLocalizeObject->pLocalizePacket;
  pStereoMeta = &(pStereoPkt->stMetadata.stStereoMetadata);
  pPktBuf = (char *)pStereoPkt;
  iPktLen = sizeof(StereoPacket);

  // SEND THE REQUEST FOR STEREO PACKET
  //iRetVal = SocketUDP_SendTo(ucReqMsg, sizeof(ucReqMsg),  &sockClientAddr, iLenSockClient);
  iRetVal = SocketUDP_ClientSend(ucReqMsg, sizeof(ucReqMsg));
  if (iRetVal < 0) { goto ret_err; }

  // RECEIVE STEREO PACKET DATA
  //iRetVal = SocketUDP_RecvFrom(pPktBuf, iPktLen, &sockClientAddr, &iLenSockClient);
  iRetVal = SocketUDP_ClientRecv(pPktBuf, iPktLen);
  if (iRetVal < 0 ) { goto ret_err; }

  return 0;
ret_err:
  printf(TAG_LOUT "Error: sending %s, ret:%d, NetErr:%d\n", ucReqMsg, iRetVal, WSAGetLastError());
  return -1;

}


int LocalizeInput_Deinit(LocalizeObject *ptr_localize_object)
{

#ifdef DEBUG_OUTPUT_FILE
  myFile.close();
#endif // DEBUG_OUTPUT_FILE

  SocketUDP_Deinit();

  return 0;
}

int LocalizeInput_Init(LocalizeObject *pLocalizeObject)
{
 // INPUT StereoPacket.
  StereoMetadata *pStereoMeta;
  StereoPacket   *pStereoPkt;
  LocalizePacket *pLocPkt;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;
 
  printf("In LocalizeInput_Init\n");

  // TODO: Ring buffer
  
  // INPUT Buffer
  // Allocate memory stereo packet, i.e. metadata + jpeg frames bytes
  pStereoPkt     = (StereoPacket *) malloc(sizeof(StereoPacket));
  pStereoMeta = &(pStereoPkt->stMetadata.stStereoMetadata);
  
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
  pLocalizeObject->pStereoPacket   = pStereoPkt;
  pLocalizeObject->pFrameLeft      = pFrameL; 
  pLocalizeObject->pFrameRight     = pFrameR;
  pLocalizeObject->pLocalizePacket = pLocPkt;

  // Note: OUTPUT buffer is within StereoPacket Metadata
#ifdef DEBUG_OUTPUT_FILE
  file_open();
#endif // DEBUG_OUTPUT_FILE

  if (SocketUDP_ClientInit() != 0) {
    printf("Error: In SocketUDP_ClientInit\n");
    return -1;
  }
  printf("SocketUDP_ClientInit... OK\n");

  return 0;
}



#include <opencv2/highgui.hpp>      //for imshow

#include "DataFormat.h"
#include "LocalizeObject.h"

using namespace std;
using namespace cv;

// Macros
#define TAG_SPRS "SPrs: "
#define DEBUG_LOCALIZE_JPEG_DB (0u)
#define WINDOW_JPEG_DB_RIGHT "Debug JPEG to RAW Right"
#define WINDOW_JPEG_DB_LEFT "Debug JPEG to RAW Left"


// FUNCTIONS

int LocalizeProcess_JpegToRaw(LocalizeObject *pLocalizeObject)
{
  // Packet details
  StereoMetadata *pStereoMeta;
  StereoPacket   *pStereoPkt;
  LocalizePacket *pLocPkt;
  unsigned char  *pFrameL;
  unsigned char  *pFrameR;

  unsigned char  *pJpegRead;
  int iJpegSize;

  // Opencv
  Mat mDecodedImageRight;
  Mat mDecodedImageLeft;
  int iDecodedSizeRight;
  int iDecodedSizeLeft;

  // Assign the object pointers
  // Get the required data locally
  pStereoPkt = pLocalizeObject->pStereoPacket;
  pFrameL    = pLocalizeObject->pFrameLeft ;
  pFrameR    = pLocalizeObject->pFrameRight;
  pLocPkt    = pLocalizeObject->pLocalizePacket;

  pStereoMeta = &(pStereoPkt->stMetadata.stStereoMetadata);
 
  //////////// RIGHT ////////////////////////
  pJpegRead    = pStereoPkt->ucJpepFrames;
  iJpegSize = static_cast<unsigned int> (pStereoMeta->uiRightJpegSize);
#if DEBUG_LOCALIZE_JPEG_DB
  if (iJpegSize % ALIGN_ADDRESS_BYTE) {
	  printf("Localize: JPET Size not aligned to address. Size:%d\n", iJpegSize);
	  cv::waitKey(0);
  }
#endif 

  // RIGHT: Create a MAT of JPEG data
  Mat rawDataRight(1, iJpegSize, CV_8UC1, (void*)pJpegRead);

  // RIGHT: Decode JPEG to RAW
  mDecodedImageRight = imdecode(rawDataRight, false);

  if (mDecodedImageRight.data == NULL) {
      // Error reading raw image data
      printf("Localize: Error: Could not decode\n");
      while (cv::waitKey(1) < 1);
  } else {
	  iDecodedSizeRight = mDecodedImageRight.total() * mDecodedImageRight.elemSize();
      if (pStereoMeta->uiFrameBytes != iDecodedSizeRight) {
          printf("Error: RIGHT decoded frame size: 0x%x, expected:%d\n", 
			  iDecodedSizeRight, pStereoMeta->uiFrameBytes);
          return -1;
      }
	  // RIGHT: Copy the RAW data to our buffer
	  memcpy(pFrameR, mDecodedImageRight.data, iDecodedSizeRight);
#if DEBUG_LOCALIZE_JPEG_DB
	  imshow(WINDOW_JPEG_DB_RIGHT, mDecodedImageRight);
#endif //DEBUG_LOCALIZE_JPEG_DB
  }

  //////////// LEFT ////////////////////////
  
  // Point to the left frame address
  pJpegRead = pStereoPkt->ucJpepFrames + pStereoMeta->uiRightJpegSize;
  iJpegSize = static_cast<unsigned int> (pStereoMeta->uiLeftJpegSize);

  // LEFT: Create a MAT of JPEG data
  Mat rawDataLeft(1, iJpegSize, CV_8UC1, (void*)pJpegRead);

  // LEFT: Decode JPEG to RAW
  mDecodedImageLeft = imdecode(rawDataLeft, false);

  if (mDecodedImageLeft.data == NULL) {
	  // Error reading raw image data
	  printf("Localize: Error: Could not decode\n");
	  while (cv::waitKey(1) < 1);
  } else {
	  // LEFT: Copy the JPEG data
	  iDecodedSizeLeft = mDecodedImageLeft.total() * mDecodedImageLeft.elemSize();
	  if (pStereoMeta->uiFrameBytes != iDecodedSizeLeft) {
		  printf("Error: LEFT decoded frame size: 0x%x, expected:%d\n",
			  iDecodedSizeLeft, pStereoMeta->uiFrameBytes);
		  return -1;
	  }
	  memcpy(pFrameL, mDecodedImageLeft.data, iDecodedSizeLeft);
#if DEBUG_LOCALIZE_JPEG_DB
	  imshow(WINDOW_JPEG_DB_LEFT, mDecodedImageLeft);
#endif //DEBUG_LOCALIZE_JPEG_DB
  }
	return 0;
}

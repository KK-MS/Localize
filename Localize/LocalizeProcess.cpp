

#include <opencv2/highgui.hpp>      //for imshow
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "DataFormat.h"
#include "LocalizeObject.h"

using namespace std;
using namespace cv;

// Macros
#define TAG_SPRS "SPrs: "
#define DEBUG_LOCALIZE_JPEG_DB (1u)
#define WINDOW_JPEG_DB_LEFT   "Debug JPEG to RAW Left"
#define WINDOW_JPEG_DB_RIGHT "Debug JPEG to RAW Right"

#define WINDOW_CIRCLE_DB_LEFT "Debug Circle Left"
#define WINDOW_CIRCLE_DB_RIGHT "Debug Circle Right"


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
  pJpegRead    = pStereoPkt->ucJpegFrames;
  iJpegSize = static_cast<unsigned int> (pStereoMeta->uiRightJpegSize);
#if DEBUG_LOCALIZE_JPEG_DB
  if (iJpegSize % ALIGN_ADDRESS_BYTE) {
	  printf("Localize: JPET Size not aligned to address. Size:%d\n", iJpegSize);
	  getchar(); //cv::waitKey(0);
  }
#endif 

  printf("Localize: right iJpegSize:%d\n", iJpegSize);
  // RIGHT: Create a MAT of JPEG data
  Mat rawDataRight(1, iJpegSize, CV_8UC1, (void*)pJpegRead);
  
  printf("JPEG: R Size:%d, Data: %x, %x, %x, %x, End: %x, %x, %x, %x\n",
	  iJpegSize,
	  pJpegRead[0], pJpegRead[1], pJpegRead[2], pJpegRead[3],
	  pJpegRead[iJpegSize - 4], pJpegRead[iJpegSize - 3], pJpegRead[iJpegSize - 2], pJpegRead[iJpegSize - 1]);


  // RIGHT: Decode JPEG to RAW
  mDecodedImageRight = imdecode(rawDataRight, false);
  
  if (mDecodedImageRight.data == NULL) {
      // Error reading raw image data
      printf("Localize: Error: Could not decode. press key to continue...\n");
	  getchar(); return -1;
      //while (cv::waitKey(1) < 1);
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
  pJpegRead = pStereoPkt->ucJpegFrames + pStereoMeta->uiRightJpegSize;
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
  printf("JPEG: L Size:%d, Data: %x, %x, %x, %x, End: %x, %x, %x, %x\n",
	  iJpegSize,
	  pJpegRead[0], pJpegRead[1], pJpegRead[2], pJpegRead[3],
	  pJpegRead[iJpegSize - 4], pJpegRead[iJpegSize - 3], pJpegRead[iJpegSize - 2], pJpegRead[iJpegSize - 1]);
  
  pLocalizeObject->uiWidthFrame = mDecodedImageLeft.cols;
  pLocalizeObject->uiHeightFrame = mDecodedImageLeft.rows;
  //pLocalizeObject->mFrameLeft = mDecodedImageLeft;
  //pLocalizeObject->mFrameRight = mDecodedImageRight;

	return 0;
}

int LocalizeProcess_CalcDist(LocalizeObject *pLocalizeObject)
{
	return 0;
}

int LocalizeProcess_FindMarks(LocalizeObject *pLocalizeObject)
{
	int isCircle = 1;
	unsigned char  *pFrameL;
	unsigned char  *pFrameR;
	
	pFrameL = pLocalizeObject->pFrameLeft;
	pFrameR = pLocalizeObject->pFrameRight;

	Mat mGrayScaleLeft(Size(pLocalizeObject->uiWidthFrame, pLocalizeObject->uiHeightFrame), CV_8UC1, pFrameL);
	Mat mGrayScaleRight(Size(pLocalizeObject->uiWidthFrame, pLocalizeObject->uiHeightFrame), CV_8UC1, pFrameR);
	Mat mCircle;

	// First: Circle or Square
	if (isCircle)
	/// Reduce the noise so we avoid false circle detection
	GaussianBlur(mGrayScaleRight, mCircle, Size(9, 9), 2, 2);

	vector<Vec3f> circles;

	/// Apply the Hough Transform to find the circles
	HoughCircles(mCircle, circles, CV_HOUGH_GRADIENT, 1, mCircle.rows / 8, 200, 100, 0, 0);
	
	printf("LOC PRO: Found circles: %d\n", circles.size());
	/// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// circle center
		circle(mGrayScaleRight, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		// circle outline
		circle(mGrayScaleRight, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}

	imshow(WINDOW_CIRCLE_DB_RIGHT, mGrayScaleRight);

	return 0;
}
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

// Threads: to run streaming server
#include <process.h>

#include <opencv2/highgui.hpp>

#include "DataFormat.h"
#include "LocalizeObject.h"

// MACROS
#define WINDOW_LOCALIZE_INPUT  "Localize Input"
#define WINDOW_LOCALIZE_OUTPUT "Localize Output"
#define WINDOW_LOCALIZE_JPEG   "Localize JPEG"
#define WINDOW_LOCALIZE_DEBUG  "Localize Debug"

// uncomment the require debug
#define DEBUG_LOCALIZE_INPUT  (1u)
#define DEBUG_LOCALIZE_OUTPUT (0u)
#define DEBUG_LOCALIZE_DEBUG  (0u)
#define DEBUG_LOCALIZE_JPEG   (0u)

#define TAG_LOC "LOC: "

// Namespace
using namespace std;
using namespace cv;

// Thread, scheduler
static HANDLE hLocalizeScheduler;


#if (DEBUG_LOCALIZE_INPUT | DEBUG_LOCALIZE_OUTPUT |DEBUG_LOCALIZE_JPEG | DEBUG_LOCALIZE_DEBUG )

void OpenDisplayWindows()
{
#ifdef DEBUG_LOCALIZE_INPUT
	namedWindow(WINDOW_LOCALIZE_INPUT, WINDOW_NORMAL);
#endif // DEBUG_LOCALIZE_INPUT

#if DEBUG_LOCALIZE_OUTPUT
	namedWindow(WINDOW_LOCALIZE_OUTPUT, WINDOW_NORMAL);
#endif // DEBUG_LOCALIZE_OUTPUT

#if DEBUG_LOCALIZE_JPEG
	namedWindow(WINDOW_LOCALIZE_JPEG, WINDOW_NORMAL);
#endif // DEBUG_LOCALIZE_JPEG

#if DEBUG_LOCALIZE_DEBUG
	namedWindow(WINDOW_LOCALIZE_DEBUG, WINDOW_NORMAL);
#endif // DEBUG_LOCALIZE_DEBUG
}
#else
void OpenDisplayWindows() {}
#endif

//void debug_mat(Mat &mat_obj, const char *mat_name);

#if (DEBUG_LOCALIZE_INPUT | DEBUG_LOCALIZE_OUTPUT |DEBUG_LOCALIZE_JPEG | DEBUG_LOCALIZE_DEBUG )
void ImageShowDebug(LocalizeObject *pLocalizeObject)
{
	Mat cam_frame;

	LocalizePacket *pLocPkt;
	Metadata       *pMeta;
	StereoPacket   *pStereoPkt;
	StereoMetadata *pStereoMeta;
	unsigned char  *pFrameL;
	unsigned char  *pFrameR;

	pLocPkt     = pLocalizeObject->pLocalizePacket;
	pStereoPkt  = pLocalizeObject->pStereoPacket;
	pMeta       = &pStereoPkt->stMetadata;
	pStereoMeta = &pMeta->stStereoMetadata;

	pFrameL = pLocalizeObject->pFrameLeft;
	pFrameR = pLocalizeObject->pFrameRight;

	printf("Ex1 %x %x %x %x i:%d\n", pFrameL[0], pFrameL[1], pFrameL[2], pFrameL[3], pStereoMeta->uiFrameBytes);
	
	Mat mGrayScaleLeft(Size(pStereoMeta->uiFrameWidth, pStereoMeta->uiFrameHeight), CV_8UC1, pFrameL);

#if DEBUG_LOCALIZE_INPUT
	imshow(WINDOW_LOCALIZE_INPUT, mGrayScaleLeft);
	//debug_mat(mGrayScaleLeft, "Exe_input");
#endif // DEBUG_LOCALIZE_INPUT
	
	
	//cv::waitKey(0);
}
#else
void ImageShowDebug(LocalizeObject *pLocalizeObject) {}
#endif

/**
* localize_scheduler
*
* This is the localize process scheduler.
* First it request the metadata, and process it.
* Secondly request for the image data as per metadata.
* The stereo data is stored locally in the struct stereo_data type.
*/
void LocalizeExecute_Scheduler(void *param)
{
	int iRetVal = 0;

	LocalizeObject *pLocalizeObject = (LocalizeObject *)param;

	printf(TAG_LOC "In localize_scheduler\n");

	while (1) {

		printf(TAG_LOC "LocalizeInput_GetStream\n");
		// INPUT: Metadata + JPEG Frames (Right & Left)
		iRetVal = LocalizeInput_GetStream(pLocalizeObject);
		if (iRetVal) { goto err_ret; }
		
		printf(TAG_LOC "LocalizeProcess_JpegToRaw\n");
		iRetVal = LocalizeProcess_JpegToRaw(pLocalizeObject);
		if (iRetVal) { goto err_ret; }
		
		printf(TAG_LOC "LocalizeInput_GetMapObjects\n");
		iRetVal = LocalizeInput_GetMapObjects(pLocalizeObject);
		if (iRetVal) { goto err_ret; }

#if 0
		// GET METADATA
		if (localize_input_request_metadata(req_meta, res_meta, len_meta) < 0) {
			goto end_client_thread;
		}

		// GET IMAGE FRAMES
		if (localize_input_request_images(req_imgs, res_imgs, len_imgs) < 0) {
			goto end_client_thread;
		}

		// PROCESS IMAGE FRAME

#endif
		ImageShowDebug(pLocalizeObject);

        //waitKey(0); // For debug each frame-by-frame
		//  wait until ESC key
		if (cv::waitKey(10) == 27) { // delay: Tune it.
			break;
		}
		
	}

err_ret:
	destroyAllWindows();
	printf("%s: thread closing: %d\n", __func__, iRetVal);
}

int LocalizeExecute_Terminate()
{
	printf(TAG_LOC "In localize_terminate\n");

	//localize_input_deinit();

	printf(TAG_LOC "In localize: WaitForSingleObject h_localize_scheduler\n");
	// WAIT for Server loop to end
	WaitForSingleObject(hLocalizeScheduler, INFINITE);

	return 0;
}


// 
// StereoExecute_Start
// 
// Starts the execution processing by starting the scheduler.
//
int LocalizeExecute_Start(LocalizeObject *pLocalizeObject)
{
  hLocalizeScheduler = (HANDLE)_beginthread(LocalizeExecute_Scheduler, 0, 
      (void *)pLocalizeObject);

	return 0;
}



int main()
{
	LocalizeObject *pLocalizeObject;

	pLocalizeObject = (LocalizeObject *)malloc(sizeof(LocalizeObject));
	if (!(pLocalizeObject)) { printf("Error: malloc\n"); return -1; }

	// Initialize the input interfaces
	LocalizeInput_Init(pLocalizeObject);

	// Initialize the output interface.
	// Output is a stream of IMU data + Localize camera frames
	//LocalizeOutput_Init(pLocalizeObject);

	LocalizeExecute_Start(pLocalizeObject);

	cv::waitKey(0);

	return 0;
}

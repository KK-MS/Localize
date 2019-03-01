#pragma once

#define FRAME_WIDTH    (1080u)
#define FRAME_HEIGHT   (720u)
#define FRAME_CHANNELS (1u) // 1 => Grayscale
#define FRAME_SIZE     (FRAME_WIDTH * FRAME_HEIGHT * FRAME_CHANNELS)
#define MAX_FRAME_SIZE FRAME_SIZE

#define ALIGN_ADDRESS_BYTE      (4u)
#define ALIGN(x,a)              __ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

// Network macros
#define MAX_UDP_DATA_SIZE (65000u)

// Network Socket details
#define SOCK_PORT_STEREO (27015)
#define SOCK_PORT_GTMAP  (27016)
#define SOCK_PORT_IMU    (27017)
#define SOCK_IP_STEREO   "127.0.0.1" 
#define SOCK_IP_GTMAP    "127.0.0.1" 
#define SOCK_IP_IMU      "127.0.0.1" 

// Request tags
#define REQ_STREAM       "meta+jpegs"
#define REQ_METADATA     "metadata"
#define REQ_IMAGES       "stereo_images"
#define REQ_FRAME_LEFT   "frame_left"
#define REQ_FRAME_RIGHT  "frame_right"
#define REQ_TRAFFISIGNS  "trafficSigns"
#define MAX_REQ_SIZE MAX_FRAME_SIZE

typedef unsigned char uchar;

typedef struct ImuMetadata {
	// IMU values. Written by IMU
	unsigned long  ulTimestamp;
	unsigned int   uiLatitude;
	unsigned int   uiLongitude;
	// TODO: other IMU variables should be appended
} ImuMetadata;

typedef struct StereoMetadata {
	// Image packet. Written by stereo
	unsigned int  uiLeftJpegSize;
	unsigned int  uiRightJpegSize;
	//unsigned int  uiJpegsSize;

	// Image features. Written by stereo
	unsigned int  uiFrameWidth;
	unsigned int  uiFrameHeight;
	unsigned int  uiNumOfChannels;
	unsigned int  uiFrameBytes;

} StereoMetadata;

typedef struct LocalizeMetadata {
	// Localized values.
	unsigned int   uiLocLat;
	unsigned int   uiLocLong;
	unsigned int   uiAccLat;
	unsigned int   uiAccLong;
} LocalizeMetadata;

typedef struct Metadata {
	ImuMetadata      stImuMetadata;
	StereoMetadata   stStereoMetadata;
	LocalizeMetadata stLocalizeMetadata;
} Metadata;


typedef struct StereoPacket
{
	// Metadata
	Metadata stMetadata;

	// stereo camera 2 x JPEG frames, each frame max size MAX_FRAME_SIZE
	unsigned char ucJpepFrames[MAX_FRAME_SIZE * 2];

}StereoPacket;

typedef struct LocalizePacket
{
	// Metadata with localized values stored
	Metadata stMetaPkt;
}LocalizePacket;

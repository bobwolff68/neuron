//!
//! \file V4L2Cap.h
//! 
//! \brief 
//! 
//! \author rwolff
//! \date Thu 30 Jun 2011 11:28:25 AM 
//! 

#ifndef V4L2CAP_H_
#define V4L2CAP_H_

#include <iostream>
#include <deque>
#include <sstream>
#include "ThreadSingle.h"

#include "RTBuffer.h"
#if 0
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>

// Apple Mac 'ifdef' for future reference.
#if defined(__APPLE__) & defined(__MACH__)
#import <Cocoa/Cocoa.h>
#import <QTKit/QTkit.h>
#import <Foundation/Foundation.h>
#endif

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef struct v4l2def {
	string device;
	int width;
	int height;
	string format;
} vDefaults;


//!
//! \class V4L2Cap
//! 
//! \brief Parent class from which we will derive specific capture type for the three methods of
//!	dealing with buffers in V4L2. 3 anticipated children for 'read', 'mmap', and 'userptr'.
//! 
//! \todo Create parent class for derivation -- generic 'VidCapGeneric' covering Linux, Windows, and Mac.
//! \todo Read prior preferences and set them via .cfg file or registry etc.
//! \todo Rip out all references to 'io' and its type. Discover capabilities and instantiate the right V4L2CapType::
//! \todo 
//! \todo 
//! 
class QTKitCap : public ThreadSingle {
public:
	QTKitCap(QTCaptureSession* capsess, QTCaptureDecompressedVideoOutput* vidout, int w=0, int h=0, const char* fmt=NULL);
	virtual ~QTKitCap(void);
	void start_capturing(void);
	void stop_capturing(void);
	void clear(void) { pRTBuffer->clear(); };
	
	enum errtype {
		OK = 0,
		NO_DEVICE,
		INVALID_DEVICE,
		CANNOTOPEN_DEVICE,
		NO_IMAGEFORMATS,
		FORMAT_REJECTED,	// Request to set a particular format failed.
		
		MAX_ERRTYPE
		};
	
	const char* szErrors[MAX_ERRTYPE+1];
	
protected:
	errtype InitDevice();
	void DeInitDevice();
	QTKitCap::errtype showerr(errtype err);
	int workerBee(void);

    QTCaptureDecompressedVideoOutput* mDecompVideo;
    QTCaptureSession* mCaptureSession;
    
	QTKitCapBuffer* pRTBuffer;
	vDefaults def;

public:
	QTKitCapBuffer* GetBufferPointer(void) { return pRTBuffer; };
	void GetCurrentFormat(int width, int height, string& colorspace) { width = def.width; height = def.height; colorspace=def.format; };
};

class TVidCap : public TempVidCapBase {
public:
    TVidCap(QTKitCap* pC) { pCap = pC; pRTBuffer = pCap->GetBufferPointer(); bIsReleased = false; };
    ~TVidCap() { };
    void start_capturing(void) { pCap->start_capturing(); };
    void stop_capturing(void) { pCap->stop_capturing(); };
    void release(void) { bIsReleased = true; };
    
    bool bIsReleased;

protected:
    QTKitCap *pCap;
};


#endif


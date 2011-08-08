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

static string& errnostr(string& st)
{
stringstream strm;

	strm << "Errno: " << errno << " (" << strerror(errno) << ")";
	st = strm.str();
	return st;
}

static void errno_exit(const char * s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

class RTBufferInfoBase {
public:
	RTBufferInfoBase(void) { bFinalSample=false; pBuffer=NULL; };
	virtual ~RTBufferInfoBase(void) { };
	void* pBuffer;      // Single pointer to a contiguous block of data regardless of planar or non-
	bool bFinalSample;
};


#if defined(__APPLE__) & defined(__MACH__)
class QTKitBufferInfo : public RTBufferInfoBase {
public:
	QTKitBufferInfo(void) { pY=NULL; pCb=NULL; pCr=NULL; };
	virtual ~QTKitBufferInfo(void) { };
    CVImageBufferRef pFrame;
    // TODO - consider making three distinct planar pointers in addition to a non-planar pointer
    void* pY;
    void* pCb;
    void* pCr;
};
#endif

#define MAX_QUEUE_SIZE 10

//! \todo During ReleaseOutputSide or clear, we need to lock the mutex, clean up the semaphore, and re-enqueue all driver buffers.
class RTBuffer {
public:
	RTBuffer(void);
	virtual ~RTBuffer(void);
	bool FullBufferEnQ(RTBufferInfoBase& BI);
	bool FullBufferDQ(RTBufferInfoBase& BI);
	void Shutdown(void);

	// \brief Used by stop_capture() (or the Dequeue side--not-preferred) for releasing its interest in the buffer.
	void ReleaseOutputSide(void) { bReleased = true; };
	//! \brief To be implemented by the capture side in case buffers need to be released back to the driver.
	virtual bool EmptyBufferRelease(RTBufferInfoBase& BI) = 0;

	int Qsize(void) { return bufferQ.size(); };
	void clear(void) { bufferQ.clear(); mFrameCount=0; mRefusedCount=0; };
    
    void startRunning(void) { bIsRunning=true; };
    void pauseRunning(void) { bIsRunning=false; };
	
    int mFrameCount;
    int mRefusedCount;

protected:
	deque<RTBufferInfoBase> bufferQ;
	pthread_mutex_t         mutex;
	sem_t sem_numbuffers;
	bool bReleased;
    bool bIsRunning;
};

class QTKitCapBuffer : public RTBuffer {
public:
	QTKitCapBuffer() { };
	~QTKitCapBuffer(void) { };
	bool EmptyBufferRelease(RTBufferInfoBase& BI);
};


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


#endif


#ifndef _RTBUF_H_
#define _RTBUF_H_

#include <iostream>
#include <deque>
#include <sstream>

#include <errno.h>
#include <semaphore.h>
#include <assert.h>

using namespace std;

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
    void* pVideoFrame; // Used to be ... CVImageBufferRef pFrame;
    void* pAudioSamples; // Used to be ... QTSampleBuffer pAudioSamples;
    
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

class TempVidCapBase {
public:
    TempVidCapBase() { };
    ~TempVidCapBase() { };
    virtual void start_capturing(void) = 0;
	virtual void stop_capturing(void) = 0;
	QTKitCapBuffer* GetBufferPointer(void) { return pRTBuffer; };
protected:
    QTKitCapBuffer* pRTBuffer;
};

#endif

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
	RTBufferInfoBase(void) { bFinalSample=false; pBuffer=NULL; pBufferLength=0; bIsVideo=true; timeStamp_uS=0; };
	virtual ~RTBufferInfoBase(void) { };
    int pBufferLength;  // Instructing the receiver how much of this data is stated to be valid.
	void* pBuffer;      // Single pointer to a contiguous block of data regardless of planar or non-
                    // In the case of AudioSamples, this is the AudioSampleList structure equiv.
	bool bFinalSample;
    bool bIsVideo;
    long long timeStamp_uS;
    // Not to be used by the consumer side -- this is for the QTKitCap to be able to release properly the buffers.
    void* pVideoFrame; // Used to be ... CVImageBufferRef pFrame;
    void* pAudioSamples; // Used to be ... QTSampleBuffer pAudioSamples;
};

#if defined(__APPLE__) & defined(__MACH__)
// single or multiple channel audio data based upon mNumberChannels.
struct NeuronAudioBuffer {
    uint32_t mNumberChannels;
    uint32_t mDataByteSize;
    void   *mData;
};
typedef struct NeuronAudioBuffer NeuronAudioBuffer;

// Audio buffer from Mac audio sample "pBuffer" pointer.
struct NeuronAudioBufferList {
    uint32_t      mNumberBuffers;
    NeuronAudioBuffer mBuffers[1];
};
typedef struct NeuronAudioBufferList NeuronAudioBufferList;

class QTKitBufferInfo : public RTBufferInfoBase {
public:
	QTKitBufferInfo(void) { pY=NULL; pCb=NULL; pCr=NULL; bIsVideo=true; rawLength=0; rawNumSamples=0; };
	virtual ~QTKitBufferInfo(void) { };
    
//    bool bIsVideo;
//    long long timeStamp_uS;
    
    // # bytes total when audio samples were sent RAW (contiguous block)
    // and number of samples so destination knows how to cut the buffers apart if necessary.
    uint32_t rawLength;
    int32_t  rawNumSamples;
    
    int captureStride;  // Used for sending to encoder in non-planar colorspace.
    
    // TODO - consider making three distinct planar pointers in addition to a non-planar pointer
    void* pY;
    void* pCb;
    void* pCr;


};
#endif

#define MAX_QUEUE_SIZE 100

//! \todo During ReleaseOutputSide or clear, we need to lock the mutex, clean up the semaphore, and re-enqueue all driver buffers.
class RTBuffer {
public:
	RTBuffer(void);
	virtual ~RTBuffer(void);
	bool FullBufferEnQ(RTBufferInfoBase* pBI);
	bool FullBufferDQ(RTBufferInfoBase** ppBI, void **ppb);
	void Shutdown(void);
    
	//! \brief To be implemented by the capture side in case buffers need to be released back to the driver.
	virtual bool EmptyBufferRelease(RTBufferInfoBase* pBI, void*pb) = 0;
    
	int Qsize(void) { return bufferQ.size(); };
    //TODO - Need to think about doing a pauseRunning() followed by a wait of some kind
    //       to wait for the queue to be drained. If not drained, how do we delete the
    //       samples? Call EmptyBufferRelease() manually on each sample remaining? Possibly.
	void clear(void) { bufferQ.clear(); mFrameCount=0; mRefusedCount=0; };
    
    void startRunning(void) { bIsRunning=true; };
    void pauseRunning(void) { bIsRunning=false; };
	
    int mFrameCount;
    int mRefusedCount;
    
	// \brief Used by stop_capture() (or the Dequeue side--not-preferred) for releasing its interest in the buffer.
	void ReleaseOutputSide(void) { bReleased = true; };

protected:
	deque<RTBufferInfoBase*> bufferQ;
    deque<void*> pBuff_copied;
	pthread_mutex_t         mutex;
	sem_t* p_sem_numbuffers;
	bool bReleased;
    bool bIsRunning;
};

class QTKitCapBuffer : public RTBuffer {
public:
	QTKitCapBuffer() { };
	~QTKitCapBuffer(void) { };
	bool EmptyBufferRelease(RTBufferInfoBase* pBI, void* pb);
};

class TempVidCapBase {
public:
    TempVidCapBase() { };
    ~TempVidCapBase() { };
    virtual void start_capturing(void) = 0;
	virtual void stop_capturing(void) = 0;
    virtual void release(void) = 0;
	QTKitCapBuffer* GetBufferPointer(void) { return pRTBuffer; };
protected:
    QTKitCapBuffer* pRTBuffer;
};

#endif

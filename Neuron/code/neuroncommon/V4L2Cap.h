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

#ifdef TEST
#define SKIP_DDS
#endif

#include <iostream>

#include "neuroncommon.h"

#include <string.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef struct v4l2def {
	string device;
	int width;
	int height;
	string format;
} vDefaults;

//! \todo TO CUT OUT
struct buffer {
        void *                  start;
        size_t                  length;
};

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

static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

class V4L2BufferInfo {
public:
	V4L2BufferInfo(void) { bFinalSample=false; CLEAR(buf); pBuffer=NULL; };
	virtual ~V4L2BufferInfo(void) { };
	void* pBuffer;
	struct v4l2_buffer buf;
	bool bFinalSample;
};

//! \todo During ReleaseOutputSide or clear, we need to lock the mutex, clean up the semaphore, and re-enqueue all driver buffers.
class RTBuffer {
public:
	RTBuffer(void);
	virtual ~RTBuffer(void);
	bool FullBufferEnQ(V4L2BufferInfo& BI);
	bool FullBufferDQ(V4L2BufferInfo& BI);
	void Shutdown(void);

	// \brief Used by stop_capture() (or the Dequeue side--not-preferred) for releasing its interest in the buffer.
	void ReleaseOutputSide(void) { bReleased = true; };
	//! \brief To be implemented by the capture side in case buffers need to be released back to the driver.
	virtual bool EmptyBufferRelease(V4L2BufferInfo& BI) = 0;

	int Qsize(void) { return bufferQ.size(); };
	void clear(void) { bufferQ.clear(); };
	
protected:
	deque<V4L2BufferInfo> bufferQ;
	pthread_mutex_t         mutex;
	sem_t sem_numbuffers;
	bool bReleased;
};

class V4L2CapBuffer : public RTBuffer {
public:
	V4L2CapBuffer(int in_fd) { fd = in_fd; };
	virtual ~V4L2CapBuffer(void) { };
	bool EmptyBufferRelease(V4L2BufferInfo& BI);
protected:
	int fd;	// replica from the capture driver once it's open.
	
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
class V4L2Cap : public ThreadSingle {
public:
	V4L2Cap(const char* indev=NULL, int w=0, int h=0, const char* fmt=NULL);
	virtual ~V4L2Cap(void);
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
	V4L2Cap::errtype showerr(errtype err);
	void init_mmap(void);
	int read_frame(void);
	int workerBee(void);

	V4L2CapBuffer* pRTBuffer;
	vDefaults def;
	static const int buffersRequested = 8;
	int fd;
	io_method	io;
	struct buffer *         buffers;
	unsigned int     n_buffers;

public:
	V4L2CapBuffer* GetBufferPointer(void) { return pRTBuffer; };
	void GetCurrentFormat(int width, int height, string& colorspace) { width = def.width; height = def.height; colorspace=def.format; };
};


#endif


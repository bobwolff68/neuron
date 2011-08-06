//!
//! \file V4L2Cap.cpp
//! 
//! \brief Linux version of video capture support. This utilizes the Video4Linux2 API and libraries.
//!	As a note, V4L2 captures in 3 distinct methods: fd read, memmapped, and user-pointer. Each has
//!	it's own pros and cons but memory mapped is preferred. Unfortunately any video capture driver
//! 	may support any one or more of these. So we must be able to suport each of them in as optimal
//!	a way possible since this is a major data-gathering and piping activity.
//! 
//! \author rwolff
//! \date Thu 30 Jun 2011 11:01:47 AM 
//! 

// When compiling from code/neuroncommon/
// testing-compilation: g++ -g3 V4L2Cap.cpp -I../include -I. -lpthread
#include "QTKitCap.h"

#include <iomanip>

using namespace std;

bool QTKitCapBuffer::EmptyBufferRelease(QTKitBufferInfo& BI)
{
    // This BI has already been taken off the internal Queue with DQ call
    CVBufferRelease(BI.pFrame);
	
	return true;
}

RTBuffer::RTBuffer(void) 
{ 
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_numbuffers, 0, 0); 
	bReleased = false;
    
    mFrameCount=0;
    mRefusedCount=0;
    
    bIsRunning=false;
};

RTBuffer::~RTBuffer(void) 
{ 
	if (bufferQ.size() && !bReleased)
	{
		cerr << "~RTBuffer() is waiting on " << bufferQ.size() << " samples to be cleared before shutdown." << flush;
		while (bufferQ.size() && !bReleased)
		{
			cerr << "." << flush;
			usleep(100000);
		}
	}
	
	pthread_mutex_destroy(&mutex);
	sem_destroy(&sem_numbuffers);
};

//!
//! RTBuffer::Shutdown
//! 
//! \brief Purpose is to send one more (null-sample) sample through the buffer so that
//!		any 'FullBufferDQ()' people are alerted that this is a final non-usable sample.
//! 
//! \note If the consumer of samples doesn't adhere to this, they will become hopelessly deadlocked in a sem_wait()
//!		while the object is being deleted. Not a good situation.
//! 
//! \param void
//! \return void
//! 
//! 
void RTBuffer::Shutdown(void)
{
    QTKitBufferInfo bi;
    
    bi.bFinalSample = true;
    
    FullBufferEnQ(bi);	
    
    pauseRunning();
    
    mFrameCount=0;
    mRefusedCount=0;
}

bool RTBuffer::FullBufferEnQ(QTKitBufferInfo& BI)
{
  int rc=0;
    
    //TODO WARNING: inbound BI must be copied to a new allocated BI
    // prior to enqueuing as these were stack objects inbound.
  
    if (!bIsRunning)
    {
        NSLog(@"RTBuffer is not running. Dropping enqueue.");
        mRefusedCount++;
        return false;
    }

    if (Qsize() >= MAX_QUEUE_SIZE)
    {
      NSLog(@"RTBuffer full: Enqueue refused.");
      mRefusedCount++;
      return false;
    }

    rc = pthread_mutex_lock(&mutex);
  if (rc)
  {
  string st;
  	cerr << "Lock failed with returncode = " << rc << endl;
  	cerr << "errno=" << errno << " at this moment." << endl;
  	cerr << "Equates to: " << errnostr(st) << endl;
  	assert(false && "Lock Failed");
  	return false;
  }
  
  // Enqueue the item sent in.
  bufferQ.push_back(BI);

  sem_post(&sem_numbuffers);	// Release the semaphore by incrementing by one.

    mFrameCount++;
    
    CVBufferRetain(BI.pFrame);

    rc = pthread_mutex_unlock(&mutex);
  if (rc)
  {
  	cerr << "UnLock failed with returncode = " << rc << endl;
  	assert(false && "Unlock Failed");
  	return false;
  }
  
  return true;
}

//!
//! RTBuffer::FullBufferDQ
//! 
//! \brief Remove one full buffer from the queue - ready for processing.
//! 
//! \note This is a mildly blocking function. Will not return until the 
//!		mutex is locked or fails-trying. Will return early if size() is zero.
//! 
//! \param V4L2BufferInfo& BI
//! \return bool
//! 
//! 

bool RTBuffer::FullBufferDQ(QTKitBufferInfo& BI)
{
  int rc=0;
  int val;
  bool ret = true;
  assert(sem_getvalue(&sem_numbuffers, &val)==0 && val==bufferQ.size());
  
  if (bReleased)
  {
  	BI.bFinalSample=true;
  	return true;
  }
  
    sem_wait(&sem_numbuffers);	// Waits in blocked mode until there is a frame ready.
  // Note that this occurs BEFORE the mutex is locked. Else the post and wait will
  // deadlock each other.

  rc = pthread_mutex_lock(&mutex);
  if (rc)
  {
  	cerr << "Lock failed with returncode = " << rc << endl;
  	assert(false && "Lock Failed");
  	return false;
  }

  if (bufferQ.size()==0)
  {
  	// Must have been cleared out from under us. Return false.
  	ret = false;
  }
  else
  {  
	  BI = bufferQ.front();  
      // TODO WARNING: on dequeue, must deallocate BI that pops from the fifo

	  // DeQueue the item at the front.
	  bufferQ.pop_front();
	  ret = true;
  }
  
  rc = pthread_mutex_unlock(&mutex);
  if (rc)
  {
  	cerr << "UnLock failed with returncode = " << rc << endl;
  	assert(false && "Unlock Failed");
  	return false;
  }
  
  return ret;
}



//!
//! V4L2Cap::V4L2Cap
//! 
//! \brief Instantiate a Video4Linux2 device
//! 
//! \note 
//! 
//! \param const char* indev
//! \param int w
//! \param int h
//! \param const char* fmt
//! 
QTKitCap::QTKitCap(QTCaptureSession* capsess, QTCaptureDecompressedVideoOutput* vidout, int w, int h, const char* fmt) 
{
    mCaptureSession = capsess;
    mDecompVideo = vidout;
	pRTBuffer = NULL;
	
	szErrors[OK] = "All ok.";
	szErrors[NO_DEVICE] = "No video devices found - expecting /dev/video* available.";
	szErrors[INVALID_DEVICE] = "Video device was invalid. Not a device.";
	szErrors[CANNOTOPEN_DEVICE] = "Cannot open device. Exists, but open() failed.";
	szErrors[NO_IMAGEFORMATS] = "Enumerating video image formats failed to produce results.";
	szErrors[FORMAT_REJECTED] = "Setting desired format failed.";
	
	szErrors[MAX_ERRTYPE] = "ACCESS ILLEGAL - MAX ENUM.";

//! Setup true defaults in the case of no over-ridden values by the instantiator.
	def.device = "";
	def.width  = 640;
	def.height = 360;
	def.format = "2vuy";

//! Now use override values if they exist.	
	if (w)
		def.width  = w;
		
	if (h)
		def.height = h;
		
	if (fmt)
		def.format = fmt;
	
	errtype ret;
	
	ret = InitDevice();
	if (ret != OK)
	{
		cerr << "Error initializing device: " << szErrors[ret] << endl;
		return;
	}
	
	pRTBuffer = new QTKitCapBuffer;
	assert(pRTBuffer);
	
};

QTKitCap::~QTKitCap(void) 
{
	DeInitDevice();
	
	delete pRTBuffer;
}

//!
//! V4L2Cap::InitDevice
//! 
//! \brief 
//! 
//! \note 
//! 
//! \todo Add support to automatically list all available /dev/video* devices
//! \return int - Success is '0' while errors are in the V4L2Cap.h file
//! 
//! 

inline QTKitCap::errtype QTKitCap::showerr(errtype err) {
	NSLog(@"QTKitCap::Error: %d(%s)",err, szErrors[err]);
	return err;
}

QTKitCap::errtype QTKitCap::InitDevice()
{
        
    // Irrelevant in QTKit version without a major change/shift in how it operates.
    // It would be nice to have a "changeFormat" with WxH, framerate, and maybe pixel format.

        return OK;
}

void QTKitCap::DeInitDevice(void)
{

	stop_capturing();

}

void QTKitCap::stop_capturing                  (void)
{
	[mCaptureSession stopRunning];

	// Now that we're done capturing...stop the workerBee thread.
	if (IsRunning())
		stopThread();

	pRTBuffer->Shutdown();
	// Give the shutdown a little time prior to the deletion.
    	
	pRTBuffer->clear();
	pRTBuffer->ReleaseOutputSide();
}

void QTKitCap::start_capturing(void)
{

	if (IsRunning())
	{
		cerr << "start_capturing: Thread is already running...hmm...skipping start..." << endl;
		return;
	}

    // Clear out the existing queue if we have a local one as a 'restart'.
    // TODO reset the local buffer list.
    pRTBuffer->startRunning();

    [mCaptureSession startRunning];
	
	// Now that we're capturing...run the workerBee thread.
	startThread();
}

int QTKitCap::workerBee(void)
{
// Sleep 250ms. Then every second, print out current resolution, #frames, and #drops.
    int modulo=4;
    int sleep_ms=250;
    int i=0;
    
    while(1) {

		// Time to kill the thread?
		if (IsStopRequested())
			break;

		if (i % modulo == 0)
            NSLog(@"STATUS: Running: %ds, Qdepth:%d, Frames:%d, Refused:%d",i*sleep_ms/1000,pRTBuffer->Qsize(),pRTBuffer->mFrameCount,pRTBuffer->mRefusedCount);
        
        usleep(sleep_ms*1000);
        i++;
    }
    
    NSLog(@"workerBee: Exiting thread. FYI.");
    return 0;
}


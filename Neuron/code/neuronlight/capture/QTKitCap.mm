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
#include "QTKitCap.h"
#import "RTBuffer.h"

#include <iomanip>

using namespace std;

bool QTKitCapBuffer::EmptyBufferRelease(RTBufferInfoBase& BI)
{
    // This BI has already been taken off the internal Queue with DQ call
    if (((QTKitBufferInfo&)BI).bIsVideo)
        CVBufferRelease((CVImageBufferRef)((QTKitBufferInfo&)BI).pVideoFrame);
    else
        [((QTSampleBuffer*)((QTKitBufferInfo&)BI).pAudioSamples) decrementSampleUseCount];
	
	return true;
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

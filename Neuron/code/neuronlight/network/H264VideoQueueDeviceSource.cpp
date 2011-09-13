// A template for a MediaSource encapsulating an audio/video input device
//

#include "H264VideoQueueDeviceSource.h"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <stdio.h>

H264VideoQueueDeviceSource*
H264VideoQueueDeviceSource::createNew(UsageEnvironment& env,
			/*DeviceParameters params*/SafeBufferDeque* _p_bsdq) {
  return new H264VideoQueueDeviceSource(env, _p_bsdq);
}

EventTriggerId H264VideoQueueDeviceSource::eventTriggerId = 0;

unsigned H264VideoQueueDeviceSource::referenceCount = 0;

H264VideoQueueDeviceSource::H264VideoQueueDeviceSource(UsageEnvironment& env,
			   /*DeviceParameters params*/SafeBufferDeque* _p_bsdq)
  : FramedSource(env), p_bsdq(_p_bsdq) {
    // Any global initialization of the device would be done here:
  ++referenceCount;

  // Any instance-specific initialization of the device would be done here:
  //%%% TO BE WRITTEN %%%

  // We arrange here for our "deliverFrame" member function to be called
  // whenever the next frame of data becomes available from the device.
  //
  // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
  //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
  // (See examples of this call in the "liveMedia" directory.)
  //
  // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
  // Create an 'event trigger' for this device (if it hasn't already been done):
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
  }
}

H264VideoQueueDeviceSource::~H264VideoQueueDeviceSource() {
  // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
  //%%% TO BE WRITTEN %%%

  --referenceCount;
  if (referenceCount == 0) {
    // Any global 'destruction' (i.e., resetting) of the device would be done here:
    //%%% TO BE WRITTEN %%%

    // Reclaim our 'event trigger'
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
}

void H264VideoQueueDeviceSource::nextTime(void *d)
{
    if(H264VideoQueueDeviceSource::referenceCount > 0)
    {
        H264VideoQueueDeviceSource* p_ds = (H264VideoQueueDeviceSource*) d;
        p_ds->doGetNextFrame();
    }
}

void H264VideoQueueDeviceSource::doGetNextFrame() {
  // This function is called (by our 'downstream' object) when it asks for new data.

  // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
  if (p_bsdq->IsDefunct()/*0 the source stops being readable */ /*%%% TO BE WRITTEN %%%*/) {
    handleClosure(this);
    return;
  }

  // If a new frame of data is immediately available to be delivered, then do this now:
  if (p_bsdq->qsize() > 0/*0 a new frame of data is immediately available to be delivered*/ /*%%% TO BE WRITTEN %%%*/) {
    deliverFrame();
  }
  else
  {
      //printf("Scheduling 15 ms delayed task...\n");
      envir().taskScheduler().scheduleDelayedTask(30000, (TaskFunc*)nextTime, this);
  }

  // No new data is immediately available to be delivered.  We don't do anything more here.
  // Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
}

void H264VideoQueueDeviceSource::deliverFrame0(void* clientData) {
  ((H264VideoQueueDeviceSource*)clientData)->deliverFrame();
}

void H264VideoQueueDeviceSource::deliverFrame() {
  // This function is called when new frame data is available from the device.
  // We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
  // 'in' parameters (these should *not* be modified by this function):
  //     fTo: The frame data is copied to this address.
  //         (Note that the variable "fTo" is *not* modified.  Instead,
  //          the frame data is copied to the address pointed to by "fTo".)
  //     fMaxSize: This is the maximum number of bytes that can be copied
  //         (If the actual frame is larger than this, then it should
  //          be truncated, and "fNumTruncatedBytes" set accordingly.)
  // 'out' parameters (these are modified by this function):
  //     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
  //     fNumTruncatedBytes: Should be set iff the delivered frame would have been
  //         bigger than "fMaxSize", in which case it's set to the number of bytes
  //         that have been omitted.
  //     fPresentationTime: Should be set to the frame's presentation time
  //         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
  //         by calling "gettimeofday()".
  //     fDurationInMicroseconds: Should be set to the frame's duration, if known.
  //         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
  //         to set this variable, because - in this case - data will never arrive 'early'.
  // Note the code below.

  unsigned char* newFrameDataStart = NULL;
  int newFrameSize = 0;
    
    if (!isCurrentlyAwaitingData())
    {
        envir().taskScheduler().scheduleDelayedTask(30000, (TaskFunc*)nextTime, this);
        return;
    }

    p_bsdq->RemoveItem(&newFrameDataStart, &newFrameSize);
    
  // Deliver the data here:
  if (newFrameSize > fMaxSize) {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = newFrameSize - fMaxSize;
  } else {
    fFrameSize = newFrameSize;
  }
  
  gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
  memmove(fTo, newFrameDataStart, fFrameSize);
  delete (unsigned char*)newFrameDataStart;
    
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
/*void signalNewFrameData() {
  TaskScheduler* ourScheduler = NULL; //%%% TO BE WRITTEN %%%
  H264VideoQueueDeviceSource* ourDevice  = NULL; //%%% TO BE WRITTEN %%%

  if (ourScheduler != NULL) { // sanity check
    ourScheduler->triggerEvent(H264VideoQueueDeviceSource::eventTriggerId, ourDevice);
  }
}*/

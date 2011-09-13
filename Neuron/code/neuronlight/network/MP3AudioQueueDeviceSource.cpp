// A template for a MediaSource encapsulating an audio/video input device
//

#include "MP3AudioQueueDeviceSource.h"
#include "MP3StreamState.hh"

#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <stdio.h>

MP3AudioQueueDeviceSource*
MP3AudioQueueDeviceSource::createNew(UsageEnvironment& env,
                                      /*DeviceParameters params*/SafeBufferDeque* _p_bsdq) {
    MP3AudioQueueDeviceSource* pSrc = NULL;
    
    do {
        pSrc = new MP3AudioQueueDeviceSource(env, _p_bsdq);
        if (pSrc==NULL) break;
        
        if (!pSrc->initializeStream()) break;
        
        return pSrc;
    }
    while(0);
    
    // Failure if we get here.
    if (pSrc)
        Medium::close(pSrc);
    return NULL;
}

EventTriggerId MP3AudioQueueDeviceSource::eventTriggerId = 0;

unsigned MP3AudioQueueDeviceSource::referenceCount = 0;

MP3AudioQueueDeviceSource::MP3AudioQueueDeviceSource(UsageEnvironment& env,
                                                       /*DeviceParameters params*/
                                                     SafeBufferDeque* _p_bsdq)
: FramedSource(env), p_bsdq(_p_bsdq), fStreamState(new MP3StreamState(env)) {
    /*if (referenceCount == 0) {
     assert(params.getRTBuffer());
     pRT = params.getRTBuffer();
     // Any global initialization of the device would be done here:
     //%%% TO BE WRITTEN %%%
     }*/
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

MP3AudioQueueDeviceSource::~MP3AudioQueueDeviceSource() {
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
    
    delete fStreamState;
}

char const* MP3AudioQueueDeviceSource::MIMEtype() const {
    return "audio/MPEG";
}

Boolean MP3FileSource::initializeStream() {
    // Make sure the file has an appropriate header near the start:
    if (fStreamState->findNextHeader(fFirstFramePresentationTime) == 0) {
        envir().setResultMsg("not an MPEG audio file");
        return False;
    }
    
    fStreamState->checkForXingHeader(); // in case this is a VBR file
    
    fHaveJustInitialized = True;
    
    // Hack: It's possible that our environment's 'result message' has been
    // reset within this function, so set it again to our name now:
    envir().setResultMsg(name());
    return True;
}

void MP3AudioQueueDeviceSource::nextTime(void *d)
{
    MP3AudioQueueDeviceSource* p_ds = (MP3AudioQueueDeviceSource*) d;
    p_ds->doGetNextFrame();
}

void MP3AudioQueueDeviceSource::doGetNextFrame() {
    
    assert(false); // Need to resolve usage of doGetNextFrame**1**() for MP3.
    
    
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

Boolean MP3FileSource::doGetNextFrame1() {
    if (fLimitNumBytesToStream && fNumBytesToStream == 0) return False; // we've already streamed as much as we were asked for
    
    if (!fHaveJustInitialized) {
        if (fStreamState->findNextHeader(fPresentationTime) == 0) return False;
    } else {
        fPresentationTime = fFirstFramePresentationTime;
        fHaveJustInitialized = False;
    }
    
    if (!fStreamState->readFrame(fTo, fMaxSize, fFrameSize, fDurationInMicroseconds)) {
        char tmp[200];
        sprintf(tmp,
                "Insufficient buffer size %d for reading MPEG audio frame (needed %d)\n",
                fMaxSize, fFrameSize);
        envir().setResultMsg(tmp);
        fFrameSize = fMaxSize;
        return False;
    }
    if (fNumBytesToStream > fFrameSize) fNumBytesToStream -= fFrameSize; else fNumBytesToStream = 0;
    
    return True;
}

void MP3AudioQueueDeviceSource::deliverFrame0(void* clientData) {
    ((MP3AudioQueueDeviceSource*)clientData)->deliverFrame();
}

void MP3AudioQueueDeviceSource::deliverFrame() {
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
    
    void* newFrameDataStart = NULL; //%%% TO BE WRITTEN %%%
    int newFrameSize = 0; //%%% TO BE WRITTEN %%%
    
    if (!isCurrentlyAwaitingData())
    {
        envir().taskScheduler().scheduleDelayedTask(30000, (TaskFunc*)nextTime, this);
        return;
    }
    
    p_bsdq->RemoveItem(&newFrameDataStart, &newFrameSize);
    
    //printf("fMaxSize = %d, newFrameSize = %d\n",fMaxSize,newFrameSize);
    
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
    
    // After delivering the data, inform the reader that it is now available:
    FramedSource::afterGetting(this);
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
/*void signalNewMP3AudioFrameData() {
 TaskScheduler* ourScheduler = NULL; //%%% TO BE WRITTEN %%%
 MP3AudioQueueDeviceSource* ourDevice  = NULL; //%%% TO BE WRITTEN %%%
 
 if (ourScheduler != NULL) { // sanity check
 ourScheduler->triggerEvent(MP3AudioQueueDeviceSource::eventTriggerId, ourDevice);
 }
 }*/

// A template for a MediaSource encapsulating an audio/video input device
//

#include "ADTSAudioQueueDeviceSource.h"

#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <stdio.h>

////////// ADTSAudioFileSource //////////

static unsigned const samplingFrequencyTable[16] = {
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350, 0, 0, 0
};

ADTSAudioQueueDeviceSource*
ADTSAudioQueueDeviceSource::createNew(UsageEnvironment& env,
                                      /*DeviceParameters params*/SafeBufferDeque* _p_bsdq) {

    // TODO - Replace hardwired 44100, 2-channel ADTS with parameterized version from stream or call itself.
    return new ADTSAudioQueueDeviceSource(env, _p_bsdq, 1, //profile,
                                          4, //sampling_frequency_index, 
                                          2); //channel_configuration);
}

EventTriggerId ADTSAudioQueueDeviceSource::eventTriggerId = 0;
unsigned ADTSAudioQueueDeviceSource::referenceCount = 0;

ADTSAudioQueueDeviceSource::ADTSAudioQueueDeviceSource(UsageEnvironment& env,
                                                       SafeBufferDeque* _p_bsdq,
                                                       u_int8_t profile,
                                                       u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration)
: FramedSource(env), p_bsdq(_p_bsdq) {
     // Any global initialization of the device would be done here:
     //%%% TO BE WRITTEN %%%
    ++referenceCount;
    
    // Any instance-specific initialization of the device would be done here:
    
    // Need to calculate the config string based on the settings passed in...
    fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
    fNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
    fuSecsPerFrame
    = (1024/*samples-per-frame*/*1000000) / fSamplingFrequency/*samples-per-second*/;
    
    // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
    unsigned char audioSpecificConfig[2];
    u_int8_t const audioObjectType = profile + 1;
    audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
    audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
    sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
    
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

ADTSAudioQueueDeviceSource::~ADTSAudioQueueDeviceSource() {
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

void ADTSAudioQueueDeviceSource::nextTime(void *d)
{
    ADTSAudioQueueDeviceSource* p_ds = (ADTSAudioQueueDeviceSource*) d;
    
    if (referenceCount > 0)
        p_ds->doGetNextFrame();
}

void ADTSAudioQueueDeviceSource::doGetNextFrame() {
    
    //assert(false); // Need to resolve usage of doGetNextFrame**1**() for MP3.
    
    
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

void ADTSAudioQueueDeviceSource::deliverFrame0(void* clientData) {
    ((ADTSAudioQueueDeviceSource*)clientData)->deliverFrame();
}

void ADTSAudioQueueDeviceSource::deliverFrame() {
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

#if 1
    // Before delivering the frame, let's do some calculations to ensure this is everything we
    // expect it to be. ie full frame of data with an appropriate length and header-aligned (discrete)
    unsigned char headers[7];
    memcpy(headers, newFrameDataStart, sizeof(headers));
    
    // Extract important fields from the headers:
    //Boolean protection_absent = headers[1]&0x01;
    u_int16_t frame_length
    = ((headers[3]&0x03)<<11) | (headers[4]<<3) | ((headers[5]&0xE0)>>5);

    u_int16_t syncword = (headers[0]<<4) | (headers[1]>>4);
//    fprintf(stderr, "Read frame: syncword 0x%x, protection_absent %d, frame_length %d\n", syncword, protection_absent, frame_length);
    if (syncword != 0xFFF) fprintf(stderr, "WARNING: Bad syncword!\n");

    if (frame_length != newFrameSize)
        printf("ERROR: MISMATCH in Audio - Inbound deque bufsize=%d, calculated framesize=%d.\n", newFrameSize, frame_length);
#endif
    
    // Deliver the data here:
    if (newFrameSize > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = newFrameSize - fMaxSize;
    } else {
        fFrameSize = newFrameSize;
    }
    
    memmove(fTo, newFrameDataStart, fFrameSize);
    delete (unsigned char*)newFrameDataStart;
    
#if 1
    gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
#else
    // Set the 'presentation time':
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        // This is the first frame, so use the current time:
        gettimeofday(&fPresentationTime, NULL);
    } else {
        // Increment by the play time of the previous frame:
        unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
        fPresentationTime.tv_sec += uSeconds/1000000;
        fPresentationTime.tv_usec = uSeconds%1000000;
    }
    
//    fDurationInMicroseconds = fuSecsPerFrame;
    
#endif

#if 0
    // Switch to another task, and inform the reader that he has data:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                         (TaskFunc*)FramedSource::afterGetting, this);
#else    
    // After delivering the data, inform the reader that it is now available:
    FramedSource::afterGetting(this);
#endif
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
/*void signalNewMP3AudioFrameData() {
 TaskScheduler* ourScheduler = NULL; //%%% TO BE WRITTEN %%%
 ADTSAudioQueueDeviceSource* ourDevice  = NULL; //%%% TO BE WRITTEN %%%
 
 if (ourScheduler != NULL) { // sanity check
 ourScheduler->triggerEvent(ADTSAudioQueueDeviceSource::eventTriggerId, ourDevice);
 }
 }*/

// A template for a MediaSource encapsulating an audio/video input device
// 

#ifndef _ADTSAUDIOQUEUE_DEVICE_SOURCE_HH
#define _ADTSAUDIOQUEUE_DEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <deque>
#include "SafeBufferDeque.h"

class ADTSAudioQueueDeviceSource: public FramedSource {
public:
    // TODO - Eventually, we will want to pass in the codec configuration params here.
  static ADTSAudioQueueDeviceSource* createNew(UsageEnvironment& env,
//				 DeviceParameters params);
                                              SafeBufferDeque* _p_bsdq);

public:
  static EventTriggerId eventTriggerId;

private:
    ADTSAudioQueueDeviceSource(UsageEnvironment& env, /*DeviceParameters params); */
                            SafeBufferDeque* _p_bsdq,
                               u_int8_t profile,
                               u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration);
  // called only by createNew(), or by subclass constructors
  virtual ~ADTSAudioQueueDeviceSource();

private:
  // redefined virtual functions:
    virtual void doGetNextFrame();

private:
    static void deliverFrame0(void* clientData);
    static void nextTime(void* d);
    void deliverFrame();

private:
  static unsigned referenceCount; // used to count how many instances of this class currently exist
    SafeBufferDeque* p_bsdq;  
    unsigned fSamplingFrequency;
    unsigned fNumChannels;
    unsigned fuSecsPerFrame;
    char fConfigStr[5];
};

#endif

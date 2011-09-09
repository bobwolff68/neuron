// A template for a MediaSource encapsulating an audio/video input device
// 

#ifndef _MP3AUDIOQUEUE_DEVICE_SOURCE_HH
#define _MP3AUDIOQUEUE_DEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <deque>
#include "SafeBufferDeque.h"

// The following class can be used to define specific encoder parameters
//class DeviceParameters {
//public:
//    DeviceParameters() { };
//    ~DeviceParameters(void) { };
//protected:
//};

class MP3AudioQueueDeviceSource: public FramedSource {
public:
  static MP3AudioQueueDeviceSource* createNew(UsageEnvironment& env,
//				 DeviceParameters params);
                                              SafeBufferDeque* _p_bsdq);

public:
  static EventTriggerId eventTriggerId;

protected:
    MP3AudioQueueDeviceSource(UsageEnvironment& env, /*DeviceParameters params); */
                            SafeBufferDeque* _p_bsdq);
  // called only by createNew(), or by subclass constructors
  virtual ~MP3AudioQueueDeviceSource();

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
//  DeviceParameters fParams;
};

#endif

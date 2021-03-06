// A template for a MediaSource encapsulating an audio/video input device
// 

#ifndef _H264_VIDEO_QUEUE_DEVICE_SOURCE_HH
#define _H264_VIDEO_QUEUE_DEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <deque>
#include "SafeBufferDeque.h"

class H264VideoQueueDeviceSource: public FramedSource {
public:
  static H264VideoQueueDeviceSource* createNew(UsageEnvironment& env,SafeBufferDeque* _p_bsdq);
    static void SignalData(void*);

public:
  static EventTriggerId eventTriggerId;

protected:
  H264VideoQueueDeviceSource(UsageEnvironment& env,SafeBufferDeque* _p_bsdq);
  // called only by createNew(), or by subclass constructors
  virtual ~H264VideoQueueDeviceSource();

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
    bool bHaventClearedQueue;
};

#endif

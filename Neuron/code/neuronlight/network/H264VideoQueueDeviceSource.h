// A template for a MediaSource encapsulating an audio/video input device
// 

#ifndef _RTBUFFER_DEVICE_SOURCE_HH
#define _RTBUFFER_DEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include <deque>

// The following class can be used to define specific encoder parameters
class DeviceParameters {
public:
    DeviceParameters() { };
    ~DeviceParameters(void) { };
protected:
};

class H264VideoQueueDeviceSource: public FramedSource {
public:
  static H264VideoQueueDeviceSource* createNew(UsageEnvironment& env,
				 DeviceParameters params);

public:
  static EventTriggerId eventTriggerId;

protected:
  H264VideoQueueDeviceSource(UsageEnvironment& env, DeviceParameters params);
  // called only by createNew(), or by subclass constructors
  virtual ~H264VideoQueueDeviceSource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  static void deliverFrame0(void* clientData);
  void deliverFrame();

private:
  static unsigned referenceCount; // used to count how many instances of this class currently exist
  DeviceParameters fParams;
};

#endif

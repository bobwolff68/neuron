// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 Elementary Stream video buffer/device utilizing RTBuffer classes.

#ifndef _H264_VIDEO_DEVICE_SERVER_MEDIA_SUBSESSION_H
#define _H264_VIDEO_DEVICE_SERVER_MEDIA_SUBSESSION_H

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "H264VideoQueueDeviceSource.h"
#include "SafeBufferDeque.h"

class H264VideoDeviceServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
  static H264VideoDeviceServerMediaSubsession*
                createNew(UsageEnvironment& env, Boolean reuseFirstSource, SafeBufferDeque* _pbsdq);

  // Used to implement "getAuxSDPLine()":
  void checkForAuxSDPLine1();
  void afterPlayingDummy1();

private:
  H264VideoDeviceServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, SafeBufferDeque* _pbsdq);
      // called only by createNew();
  virtual ~H264VideoDeviceServerMediaSubsession();

  void setDoneFlag() { fDoneFlag = ~0; }

private: // redefined virtual functions
  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource* inputSource);

private:
    H264VideoQueueDeviceSource* pSrc;   // Used for initializing the framer regardless of a re-used source or not.
    
  char* fAuxSDPLine;
  char fDoneFlag; // used when setting up "fAuxSDPLine"
  RTPSink* fDummyRTPSink; // ditto
    SafeBufferDeque* p_bsdq;
};

#endif


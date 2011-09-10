// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from an MP3 audio file.
// (Actually, any MPEG-1 or MPEG-2 audio file should work.)
// C++ header

#ifndef _ADTS_AUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_H
#define _ADTS_AUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_H

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "MP3AudioQueueDeviceSource.h"
#include "SafeBufferDeque.h"

class ADTSAudioDeviceServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
  static ADTSAudioDeviceServerMediaSubsession*
    createNew(UsageEnvironment& env, 
              Boolean reuseFirstSource,
              SafeBufferDeque* _p_bsdq);

private:
  ADTSAudioDeviceServerMediaSubsession(UsageEnvironment& env,
                                       Boolean reuseFirstSource,
                                       SafeBufferDeque* _pbsdq);
    // called only by createNew();
  virtual ~ADTSAudioDeviceServerMediaSubsession();

private: // redefined virtual functions
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);

private:
    SafeBufferDeque* p_bsdq;
};

#endif

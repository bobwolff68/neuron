// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from an MP3 audio file.
// (Actually, any MPEG-1 or MPEG-2 audio file should work.)
// C++ header

#ifndef _MP3_AUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_H
#define _MP3_AUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_H

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif
#ifndef _MP3_ADU_INTERLEAVING_HH
#include "MP3ADUinterleaving.hh"
#endif

#include "MP3AudioQueueDeviceSource.h"
#include "SafeBufferDeque.h"

class MP3AudioDeviceServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
  static MP3AudioDeviceServerMediaSubsession*
    createNew(UsageEnvironment& env, Boolean reuseFirstSource,
              Boolean generateADUs, Interleaving* interleaving, SafeBufferDeque* _pbsdq);
    // Note: "interleaving" is used only if "generateADUs" is True,
    // (and a value of NULL means 'no interleaving')

private:
  MP3AudioDeviceServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource,
                                      Boolean generateADUs,
                                      Interleaving* interleaving, SafeBufferDeque* _pbsdq);
      // called only by createNew();
  virtual ~MP3AudioDeviceServerMediaSubsession();

private: // redefined virtual functions
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);
    virtual void testScaleFactor(float& scale);
    virtual float duration() const;

private:
    Boolean fGenerateADUs;
    Interleaving* fInterleaving;
    float fFileDuration;
    SafeBufferDeque* p_bsdq;
};

#endif
    

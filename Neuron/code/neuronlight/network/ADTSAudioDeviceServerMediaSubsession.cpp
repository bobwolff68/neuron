// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video buffer provided via RTBuffer.

#include "ADTSAudioDeviceServerMediaSubsession.h"
#include "ADTSAudioQueueDeviceSource.h"

#include "MPEG4GenericRTPSink.hh"

#include <iostream>

//static ADTSAudioDeviceServerMediaSubsession *gpReuseSrc = NULL;

ADTSAudioDeviceServerMediaSubsession*
ADTSAudioDeviceServerMediaSubsession::createNew(UsageEnvironment& env,
                                                Boolean reuseFirstSource,
                                                SafeBufferDeque* _p_bsdq) {
    return new ADTSAudioDeviceServerMediaSubsession(env, reuseFirstSource, _p_bsdq);
}

ADTSAudioDeviceServerMediaSubsession
::ADTSAudioDeviceServerMediaSubsession(UsageEnvironment& env,
                                       Boolean reuseFirstSource,
                                       SafeBufferDeque* _p_bsdq)
: OnDemandServerMediaSubsession(env, reuseFirstSource),
  p_bsdq(_p_bsdq) {
}

ADTSAudioDeviceServerMediaSubsession
::~ADTSAudioDeviceServerMediaSubsession() {
}

FramedSource* ADTSAudioDeviceServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 128; // kbps, estimate

  
    // Now - we get called just as there is a new (first) access to this stream.
    // So, we want to CLEAR the deque buffer so that we remove any stale captured items
    // from the system.
    
    std::cerr << "Reducing AUDIO Queue..." << std::endl;
    
    // Before getting started, clear out the queue until we only have a small amount of time remaining.
    p_bsdq->clearUntilOnlyMSAvailable(DESIRED_STARTING_CAPTUREDELAY);

    std::cerr << "ADTSAudioDeviceServerMediaSubsession::createNewStreamSource() entered." << std::endl;

    return ADTSAudioQueueDeviceSource::createNew(envir(), p_bsdq);
}

RTPSink* ADTSAudioDeviceServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* inputSource) {
    
    ADTSAudioQueueDeviceSource* adtsSource = (ADTSAudioQueueDeviceSource*)inputSource;
    return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
                                          rtpPayloadTypeIfDynamic,
                                          adtsSource->samplingFrequency(),
                                          "audio", "AAC-hbr", adtsSource->configStr(),
                                          adtsSource->numChannels());
}

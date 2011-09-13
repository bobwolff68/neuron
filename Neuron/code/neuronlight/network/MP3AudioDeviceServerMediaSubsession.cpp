// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video buffer provided via RTBuffer.

#include "MP3AudioDeviceServerMediaSubsession.h"

#include "MPEG1or2AudioRTPSink.hh"
#include "MP3ADURTPSink.hh"
#include "MP3ADU.hh"

#include <iostream>

static MP3AudioDeviceServerMediaSubsession *gpReuseSrc = NULL;

MP3AudioDeviceServerMediaSubsession*
MP3AudioDeviceServerMediaSubsession::createNew(UsageEnvironment& env,
                                            Boolean reuseFirstSource,
                                            Boolean generateADUs, Interleaving* interleaving,
                                            SafeBufferDeque* _p_bsdq) {
    return new MP3AudioDeviceServerMediaSubsession(env, reuseFirstSource,
                                                     generateADUs, interleaving, _p_bsdq);
}

MP3AudioDeviceServerMediaSubsession
::MP3AudioDeviceServerMediaSubsession(UsageEnvironment& env,
                                    Boolean reuseFirstSource,
                                    Boolean generateADUs,
                                    Interleaving* interleaving,
                                    SafeBufferDeque* _p_bsdq)
: OnDemandServerMediaSubsession(env, reuseFirstSource),
fGenerateADUs(generateADUs), fInterleaving(interleaving), fFileDuration(0.0), p_bsdq(_p_bsdq) {
}

MP3AudioDeviceServerMediaSubsession
::~MP3AudioDeviceServerMediaSubsession() {
    delete fInterleaving;
}

FramedSource* MP3AudioDeviceServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 128; // kbps, estimate

  // Create the video source:
//  DeviceParameters devparm;
  
    // Now - we get called just as there is a new (first) access to this stream.
    // So, we want to CLEAR the deque buffer so that we remove any additional latency
    // from the system.
    // Later we may wish to only clear this to a low-water-mark but we'll be aggressive for now.
    //p_bsdq->clearAll();
    
    std::cerr << "MP3AudioDeviceServerMediaSubsession::createNewStreamSource() entered." << std::endl;
    
    MP3AudioQueueDeviceSource* pSrc;   // Used for initializing the framer regardless of a re-used source or not.
    
    // Not re-using, so create a new device source each time.
    pSrc = MP3AudioQueueDeviceSource::createNew(envir(), p_bsdq);
    if (pSrc == NULL) return NULL;
  
    FramedSource* streamSource;
    do {
        streamSource = pSrc;    // Setup for success (keeping above separate for
                                // later consolidation when reuseFirstSource is involved.
        fFileDuration = 0.0;    // This is a live source by definition.
        
        if (fGenerateADUs) {
            // Add a filter that converts the source MP3s to ADUs:
            streamSource = ADUFromMP3Source::createNew(envir(), streamSource);
            if (streamSource == NULL) break;
            
            if (fInterleaving != NULL) {
                // Add another filter that interleaves the ADUs before packetizing:
                streamSource = MP3ADUinterleaver::createNew(envir(), *fInterleaving,
                                                            streamSource);
                if (streamSource == NULL) break;
            }
        } else if (fFileDuration > 0.0) {
            // Because this is a seekable file, insert a pair of filters: one that
            // converts the input MP3 stream to ADUs; another that converts these
            // ADUs back to MP3.  This allows us to seek within the input stream without
            // tripping over the MP3 'bit reservoir':
            streamSource = ADUFromMP3Source::createNew(envir(), streamSource);
            if (streamSource == NULL) break;
            
            streamSource = MP3FromADUSource::createNew(envir(), streamSource);
            if (streamSource == NULL) break;
        }
    } while (0);
    
    return streamSource;
}

RTPSink* MP3AudioDeviceServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
    if (fGenerateADUs) {
        return MP3ADURTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic);
    } else {
        return MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
    }
}

void MP3AudioDeviceServerMediaSubsession::testScaleFactor(float& scale) {
    scale = 1;  // Non-seekable as it is live.
}

float MP3AudioDeviceServerMediaSubsession::duration() const {
    return fFileDuration;
}


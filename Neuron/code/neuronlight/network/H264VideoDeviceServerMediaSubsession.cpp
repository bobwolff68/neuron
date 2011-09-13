// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video buffer provided via RTBuffer.

#include "H264VideoDeviceServerMediaSubsession.h"

#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"
//#include "H264VideoStreamFramer.hh"

#include <iostream>

using namespace std;

//static H264VideoQueueDeviceSource *gpReuseSrc = NULL;

H264VideoDeviceServerMediaSubsession*
H264VideoDeviceServerMediaSubsession::createNew(UsageEnvironment& env,
					       			       Boolean reuseFirstSource, SafeBufferDeque* _p_bsdq) {
  return new H264VideoDeviceServerMediaSubsession(env, reuseFirstSource, _p_bsdq);
}

H264VideoDeviceServerMediaSubsession::H264VideoDeviceServerMediaSubsession(UsageEnvironment& env,
								       Boolean reuseFirstSource, SafeBufferDeque* _p_bsdq)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL), p_bsdq(_p_bsdq), pSrc(NULL) {
}

H264VideoDeviceServerMediaSubsession::~H264VideoDeviceServerMediaSubsession() {
  delete[] fAuxSDPLine;
}

//TODO - Not sure what we should be doing after playing and afterPlayingDummy1 for that matter.
static void afterPlayingDummy(void* clientData) {
  H264VideoDeviceServerMediaSubsession* subsess = (H264VideoDeviceServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void H264VideoDeviceServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  H264VideoDeviceServerMediaSubsession* subsess = (H264VideoDeviceServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void H264VideoDeviceServerMediaSubsession::checkForAuxSDPLine1() {
  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && fDummyRTPSink->auxSDPLine() != NULL) {
    fAuxSDPLine = strDup(fDummyRTPSink->auxSDPLine());
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else {
    // try again after a brief delay:
    int uSecsToDelay = 30000; // 30 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* H264VideoDeviceServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource* H264VideoDeviceServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
    
    estBitrate = 800; // kbps, estimate

  // Create the video source:
  //DeviceParameters devparm;
    
    cerr << "H264 Video createNewStreamSource() entered." << endl;
    
    
/*  if (reuseFirstSource)
  {
      if (!gpReuseSrc)
      {
          gpReuseSrc = H264VideoQueueDeviceSource::createNew(envir(), devparm);
          if (pReuseSrc == NULL) return NULL;
      }
      
      pSrc = gpReuseSrc;
  }
  else */
  {
      // Not re-using, so create a new device source each time.
#ifdef TRY_REUSE_GAME
      if (gpReuseSrc==NULL)
      {
          pSrc = H264VideoQueueDeviceSource::createNew(envir(), p_bsdq);
          gpReuseSrc = pSrc;
      }
      else
      {
          pSrc = gpReuseSrc;
          cerr << "H264 Video createNewStreamSource() Re-Using existing pSrc." << endl;
      }
#else
#endif
      pSrc = H264VideoQueueDeviceSource::createNew(envir(), p_bsdq);
      
      if (pSrc == NULL) return NULL;
  }
  
    // Create a framer for the Video Elementary Stream:
    return H264VideoStreamDiscreteFramer::createNew(envir(), pSrc);
    
    /*if(fDoneFlag)
        p_bsdq->clearAll();*/
}

RTPSink* H264VideoDeviceServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
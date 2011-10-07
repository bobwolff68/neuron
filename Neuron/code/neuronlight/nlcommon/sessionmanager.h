#include "littlehttpd.h"

#include <map>
#include <list>

#ifndef STANDALONE_TESTER
// For testing only - ability to clear a/v queues in Live555 source and monitor their lengths
extern    bool bQuit;

extern    int bitRate;
extern    bool bChangeBitrate;
extern    int frameRate;
extern    bool bChangeFramerate;
#endif

class SessionManager;
typedef  int (SessionManager::*SessionManagerFn)(void);

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class SessionManager : public LittleHttpd {
public:
    enum states { stopped, paused, running, error, max_states };

	SessionManager(map<string, string> vals, int port);
	virtual ~SessionManager() { };
    
    map<string, string> mystuff;
	bool ParseRequest(void);
	bool ExecuteAction(void);
    
    bool ExecuteStartCapture(void);
    bool ExecuteStopCapture(void);
    bool ExecuteGetCaptureState(void);
    
    bool ExecuteSetBitrate(void);
    bool ExecuteGetBitrate(void);
    bool ExecuteSetFramerate(void);
    bool ExecuteGetFramerate(void);
    bool ExecuteSetResolution(void);
    bool ExecuteGetResolution(void);
    
    bool ExecuteGetRtspUrl(void);
    bool ExecuteQuit(void);
    bool ExecuteShowGui(void);
    
    bool ExecuteSetMicVolume(void);
    bool ExecuteGetMicVolume(void);
    bool ExecuteSetMicToggleMute(void);
    
    bool ExecuteSetSpeakerVolume(void);
    bool ExecuteGetSpeakerVolume(void);
    bool ExecuteSetSpeakerToggleMute(void);
    
#ifndef NDEBUG
    bool ExecuteSendTestScript(void);
#endif
    
protected:
    map<string, SessionManagerFn> commandMap;
    string states[max_states];
    
    stringstream strstream;
    
    void (*pCallback)(void* pData, SessionManager* psm);
    typedef map<void (*)(void* , SessionManager* ), void* > callbackMap;
    callbackMap callbacks;
    
public:
    void addCallback(void* parentinstance, void (*callback)(void* pData, SessionManager* psm))
    {
        assert(parentinstance);
        assert(callback);
        
        pCallback = callback;
        callbacks[callback] = parentinstance;
  
        doCallbacks();
//        pCallback((void*)parentinstance, this);
    }
    
    void removeCallback(void (*callback)(void* pData, SessionManager* psm))
    {
        callbackMap::iterator it;
        
        it = callbacks.find(callback);
        assert(it != callbacks.end());
        
        callbacks.erase(it);
    }
    
    void doCallbacks(void)
    {
        callbackMap::iterator it;
        
        for (it=callbacks.begin(); it != callbacks.end(); it++)
            it->first(it->second, this);
    }
    
    void setActualCaptureState(enum states st) { capState = st; };
    enum states capState;
    bool bIsQuitting;
    int curBitrate;
    int curFramerate;
    int curWidth, curHeight;
    int curMicVol, curSpeakerVol;
    bool bMicMuted, bSpeakerMuted;
    
};


#include "littlehttpd.h"

class Tester;

typedef  int (Tester::*TesterFn)(void);

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class Tester : public LittleHttpd {
public:
	Tester(map<string, string> vals, int port);
	virtual ~Tester() { };

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
protected:
    
    map<string, TesterFn> commandMap;
    enum states { stopped, paused, running, error, max_states };
    states capState;
    string states[max_states];
    
    int curBitrate;
    int curFramerate;
    
    stringstream strstream;
};

Tester::Tester(map<string, string> vals, int port) : LittleHttpd(vals, port)
{
    states[stopped] = "stopped";
    states[paused] = "paused";
    states[running] = "running";
    states[error] = "error";
    
    capState = stopped;
    
    curBitrate = 600;
    curFramerate = 30;
    
    commandMap["START_CAPTURE"] = (TesterFn)&Tester::ExecuteStartCapture;
    commandMap["STOP_CAPTURE"] = (TesterFn)&Tester::ExecuteStopCapture;
    commandMap["GET_CAPTURE_STATE"] = (TesterFn)&Tester::ExecuteGetCaptureState;
    
    commandMap["SET_BITRATE"] = (TesterFn)&Tester:: ExecuteSetBitrate;
    commandMap["GET_BITRATE"] = (TesterFn)&Tester:: ExecuteGetBitrate;
    commandMap["SET_FRAMERATE"] = (TesterFn)&Tester:: ExecuteSetFramerate;
    commandMap["GET_FRAMERATE"] = (TesterFn)&Tester:: ExecuteGetFramerate;
    commandMap["SET_RESOLUTION"] = (TesterFn)&Tester:: ExecuteSetResolution;
    commandMap["GET_RESOLUTION"] = (TesterFn)&Tester:: ExecuteGetResolution;
    
    commandMap["GET_RTSP_URL"] = (TesterFn)&Tester:: ExecuteGetRtspUrl;
    commandMap["QUIT"] = (TesterFn)&Tester:: ExecuteQuit;
    commandMap["SHOW_GUI"] = (TesterFn)&Tester:: ExecuteShowGui;

    commandMap["SET_MIC_VOLUME"] = (TesterFn)&Tester:: ExecuteSetMicVolume;
    commandMap["GET_MIC_VOLUME"] = (TesterFn)&Tester:: ExecuteGetMicVolume;
    commandMap["SET_MIC_TOGGLE_MUTE"] = (TesterFn)&Tester:: ExecuteSetMicToggleMute;
    commandMap["SET_SPEAKER_VOLUME"] = (TesterFn)&Tester:: ExecuteSetSpeakerVolume;
    commandMap["GET_SPEAKER_VOLUME"] = (TesterFn)&Tester:: ExecuteGetSpeakerVolume;
}

///
/// \brief ParseRequest is an opportunity to parse if desired by hand. But this is unlikely as
///        the original request has been boiled down to inboundBaseURL and a map<> of name/value pairs.
///
bool Tester::ParseRequest(void)
{
    string finalcommand;

    assert(inboundBaseURL[0] == '/');
    if (inboundBaseURL[0] == '/')
        finalcommand = inboundBaseURL.substr(1, string::npos);  /// Strip leading '/'
    
    // Strip any trailing spaces
    while (finalcommand[finalcommand.length()-1] == ' ')
        finalcommand = finalcommand.substr(0, finalcommand.length() - 1);
    
    // Now all uppercase.
    std::transform(finalcommand.begin(), finalcommand.end(), finalcommand.begin(), ::toupper);

    // Now check to see if the command exists in the list. If not, return an error. No command.
    if (commandMap.find(finalcommand)==commandMap.end())
    {
        bodyToReturn = "{ errMsg: \"Bad Command: Command unknown.\" }";
        return false;
    }
    
    // Inbound request can be found in LittleHttpd:: fullInboundURL and the pre-parsed
    // items of:
    // - inboundBaseURL (stripped of GET and stripped of trailing '?*'
    // - inboundPairs as a map of name/value pairs.

    // If parsing fails in the derived class, simply return false and a failure return will be
    // sent to the client.
    
    // Iterate through the inboundPairs and print them out
    cout << endl << "Inbound Command is: " << finalcommand << endl;
    
    map<string,string>::iterator iter;
    
    for (iter=inboundPairs.begin() ; iter != inboundPairs.end() ; iter++)
    {
        if (iter->second == "")
            cout << "Argument: " << iter->first << "=<NO_VALUE_GIVEN>" << endl;
        else
            cout << "Argument: " << iter->first << "=" << iter->second << endl;
    }

    // Now make the call to the appropriate parser/execution method.
    CALL_MEMBER_FN(*this, commandMap[finalcommand])();
    
    return true;
}

///
/// \brief ExecuteAction is an opportunity to make decisions based upon the raw URL, processed URL,
///        and/or the name/value pairs.
///
///
/// \note We are choosing to do nothing here as each Execute*() function will setup it's own bodyToReturn.
///
bool Tester::ExecuteAction(void)
{
    stringstream tosend;
    
    // The below snips show how a bodyToReturn can be built up of name/value pairs using
    // a stringstream and then assigning the final .str() to bodyToReturn.
    
/*    tosend << "name1" << "=" << "value1" << endl;
    tosend << "name2" << "=" << "value2" << endl;
    tosend << "name3" << "=" << "value3" << endl;
    
    // The value of 'bodyString' is what will be sent back to the client upon a 'true' success to ExecuteAction().
    bodyToReturn = tosend.str();
  */
    
    return true;
}

bool Tester::ExecuteStartCapture(void)
{
    cerr << "Execute START_CAPTURE" << endl;
    // Return true if execution went ok.
    
    capState = running;
    
    bodyToReturn = "";
    
    return true;
}

bool Tester::ExecuteStopCapture(void)
{
    cerr << "Execute STOP_CAPTURE" << endl;
    // Return true if execution went ok.
    
    capState = stopped;
    
    bodyToReturn = "";
    
    return true;
}

bool Tester::ExecuteGetCaptureState(void)
{
    cerr << "Execute GET_CAPTURE_STATE" << endl;
    // Return true if execution went ok.
    
    // { state: [“running” | “stopped” | “paused” | “error”] }
    strstream.str("");
    strstream << "{ state: \"" << states[capState] << "\" }";
    bodyToReturn = strstream.str();
    
//    bodyToReturn = "{ state: \"";
//    bodyToReturn += states[capState];
//    bodyToReturn += "\" }";
    
    return true;
}


bool Tester::ExecuteSetBitrate(void)
{
    int temprate;
    
    if (!RequiredArgPresent("bitrate"))
        return false;

    if (!getRequiredArgAsInt("bitrate", 100, 50000, temprate))
        return false;
    
    // Else all is good.
    curBitrate = temprate;
    
    return true;
}

bool Tester::ExecuteGetBitrate(void)
{
    strstream.str("");
    strstream << "{ bitrate: " << curBitrate << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool Tester::ExecuteSetFramerate(void)
{
    int temprate;
    
    if (!RequiredArgPresent("fps"))
        return false;
    
    if (!getRequiredArgAsInt("fps", 1, 60, temprate))
        return false;
    
    // Else all is good.
    curFramerate = temprate;
    
    return true;
}

bool Tester::ExecuteGetFramerate(void)
{
    strstream.str("");
    strstream << "{ fps: " << curFramerate << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool Tester::ExecuteSetResolution(void)
{
    return true;
}

bool Tester::ExecuteGetResolution(void)
{
    return true;
}


bool Tester::ExecuteGetRtspUrl(void)
{
    return true;
}

bool Tester::ExecuteQuit(void)
{
    return true;
}

bool Tester::ExecuteShowGui(void)
{
    return true;
}


bool Tester::ExecuteSetMicVolume(void)
{
    return true;
}

bool Tester::ExecuteGetMicVolume(void)
{
    return true;
}

bool Tester::ExecuteSetMicToggleMute(void)
{
    return true;
}

bool Tester::ExecuteSetSpeakerVolume(void)
{
    return true;
}

bool Tester::ExecuteGetSpeakerVolume(void)
{
    return true;
}


int main(int argc, char**argv)
{
    map<string, string> pairs;
  Tester tst(pairs, 8081);

  while(tst.isServerRunning())
	usleep(250 * 1000);

  return 0;
}

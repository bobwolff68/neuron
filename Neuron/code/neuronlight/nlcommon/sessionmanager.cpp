#include "sessionmanager.h"

SessionManager::SessionManager(map<string, string> vals, int port) : LittleHttpd(vals, port)
{
    states[stopped] = "stopped";
    states[paused] = "paused";
    states[running] = "running";
    states[error] = "error";
    
    capState = stopped;
    
    curBitrate = 600;
    curFramerate = 30;
    curWidth = 640;
    curHeight = 360;
    
    curMicVol = 75;
    bMicMuted = false;
    
    curSpeakerVol = 50;
    bSpeakerMuted = false;
    
    bIsQuitting = false;

    commandMap["START_CAPTURE"] = (SessionManagerFn)&SessionManager::ExecuteStartCapture;
    commandMap["STOP_CAPTURE"] = (SessionManagerFn)&SessionManager::ExecuteStopCapture;
    commandMap["GET_CAPTURE_STATE"] = (SessionManagerFn)&SessionManager::ExecuteGetCaptureState;
    
    commandMap["SET_BITRATE"] = (SessionManagerFn)&SessionManager:: ExecuteSetBitrate;
    commandMap["GET_BITRATE"] = (SessionManagerFn)&SessionManager:: ExecuteGetBitrate;
    commandMap["SET_FRAMERATE"] = (SessionManagerFn)&SessionManager:: ExecuteSetFramerate;
    commandMap["GET_FRAMERATE"] = (SessionManagerFn)&SessionManager:: ExecuteGetFramerate;
    commandMap["SET_RESOLUTION"] = (SessionManagerFn)&SessionManager:: ExecuteSetResolution;
    commandMap["GET_RESOLUTION"] = (SessionManagerFn)&SessionManager:: ExecuteGetResolution;
    
    commandMap["GET_RTSP_URL"] = (SessionManagerFn)&SessionManager:: ExecuteGetRtspUrl;
    commandMap["QUIT"] = (SessionManagerFn)&SessionManager:: ExecuteQuit;
    commandMap["SHOW_GUI"] = (SessionManagerFn)&SessionManager:: ExecuteShowGui;

    commandMap["SET_MIC_VOLUME"] = (SessionManagerFn)&SessionManager:: ExecuteSetMicVolume;
    commandMap["GET_MIC_VOLUME"] = (SessionManagerFn)&SessionManager:: ExecuteGetMicVolume;
    commandMap["SET_MIC_TOGGLE_MUTE"] = (SessionManagerFn)&SessionManager:: ExecuteSetMicToggleMute;
    
    commandMap["SET_SPEAKER_VOLUME"] = (SessionManagerFn)&SessionManager:: ExecuteSetSpeakerVolume;
    commandMap["GET_SPEAKER_VOLUME"] = (SessionManagerFn)&SessionManager:: ExecuteGetSpeakerVolume;
    commandMap["SET_SPEAKER_TOGGLE_MUTE"] = (SessionManagerFn)&SessionManager:: ExecuteSetSpeakerToggleMute;
#ifndef NDEBUG
    commandMap["TEST"] = (SessionManagerFn)&SessionManager:: ExecuteSendTestScript;
#endif
}

///
/// \brief ParseRequest is an opportunity to parse if desired by hand. But this is unlikely as
///        the original request has been boiled down to inboundBaseURL and a map<> of name/value pairs.
///
bool SessionManager::ParseRequest(void)
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
#if 0
    cout << endl << "Inbound Command is: " << finalcommand << endl;
    
    map<string,string>::iterator iter;
    
    for (iter=inboundPairs.begin() ; iter != inboundPairs.end() ; iter++)
    {
        if (iter->second == "")
            cout << "Argument: " << iter->first << "=<NO_VALUE_GIVEN>" << endl;
        else
            cout << "Argument: " << iter->first << "=" << iter->second << endl;
    }
#endif
    
    // Now make the call to the appropriate parser/execution method.
    if (CALL_MEMBER_FN(*this, commandMap[finalcommand])())
    {
        doCallbacks();
        return true;
    }
    else
        return false;
}

///
/// \brief ExecuteAction is an opportunity to make decisions based upon the raw URL, processed URL,
///        and/or the name/value pairs.
///
///
/// \note We are choosing to do nothing here as each Execute*() function will setup it's own bodyToReturn.
///
bool SessionManager::ExecuteAction(void)
{
    stringstream tosend;
    
    // The below snips show how a bodyToReturn can be built up of name/value pairs using
    // a stringstream and then assigning the final .str() to bodyToReturn.
    
/*
    tosend << "name1" << "=" << "value1" << endl;
    tosend << "name2" << "=" << "value2" << endl;
    tosend << "name3" << "=" << "value3" << endl;
    
    // The value of 'bodyString' is what will be sent back to the client upon a 'true' success to ExecuteAction().
    bodyToReturn = tosend.str();
*/
    
    return true;
}

bool SessionManager::ExecuteStartCapture(void)
{
    cerr << "Execute START_CAPTURE" << endl;
    // Return true if execution went ok.
    
    capState = running;
    
    bodyToReturn = "";
    
    return true;
}

bool SessionManager::ExecuteStopCapture(void)
{
    cerr << "Execute STOP_CAPTURE" << endl;
    // Return true if execution went ok.
    
    capState = stopped;
    
    bodyToReturn = "";
    
    return true;
}

bool SessionManager::ExecuteGetCaptureState(void)
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


bool SessionManager::ExecuteSetBitrate(void)
{
    int temprate;
    
    if (!getRequiredArgAsInt("bitrate", 100, 50000, temprate))
        return false;
    
    // Else all is good.
    curBitrate = temprate;
    
    return true;
}

bool SessionManager::ExecuteGetBitrate(void)
{
    strstream.str("");
    strstream << "{ bitrate: " << curBitrate << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool SessionManager::ExecuteSetFramerate(void)
{
    int temprate;
    
    if (!getRequiredArgAsInt("fps", 1, 60, temprate))
        return false;
    
    // Else all is good.
    curFramerate = temprate;
    
    return true;
}

bool SessionManager::ExecuteGetFramerate(void)
{
    strstream.str("");
    strstream << "{ fps: " << curFramerate << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool SessionManager::ExecuteSetResolution(void)
{
    int tempwidth, tempheight;
    
    if (serverURL != "")
    {
        bodyToReturn = "Failed: Resolution can only be set prior to initial (first time) capturing.";
        return false;
    }

    // Setting resolution is only legal when capturing is stopped.
//    if (capState != stopped)
//    {
//        bodyToReturn = "Failed: Resolution cannot be set while capturing.";
//        return false;
//    }
    
    if (!getRequiredArgAsInt("width", 2, 4096, tempwidth))
        return false;
    
    if (!getRequiredArgAsInt("height", 2, 2048, tempheight))
        return false;
    
    // Else all is good.
    curWidth = tempwidth;
    curHeight = tempheight;
    
    return true;
}

bool SessionManager::ExecuteGetResolution(void)
{
    strstream.str("");
    strstream << "{ width: " << curWidth << ", height: " << curHeight << " }";
    bodyToReturn = strstream.str();
    return true;
}


bool SessionManager::ExecuteGetRtspUrl(void)
{
    strstream.str("");
    strstream << "{ rtsp_url: \"" << serverURL << "\" }";
    bodyToReturn = strstream.str();
    return true;
}

bool SessionManager::ExecuteQuit(void)
{
    // Flag parent upon return that we need to shutdown the server.
    bNeedsToShutdown = true;
    
    bIsQuitting = true;
    
    return true;
}

bool SessionManager::ExecuteShowGui(void)
{
    //TODO: Show the GUI
    return true;
}


bool SessionManager::ExecuteSetMicVolume(void)
{
    int tempvol;
    
    if (!getRequiredArgAsInt("vol", 0, 100, tempvol))
        return false;
    
    // Else all is good.
    curMicVol = tempvol;
    return true;
}

bool SessionManager::ExecuteGetMicVolume(void)
{
    strstream.str("");
    strstream << "{ vol: " << curMicVol << ", muted: " << (bMicMuted ? "true" : "false") << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool SessionManager::ExecuteSetMicToggleMute(void)
{
    bMicMuted = !bMicMuted;
    return true;
}

bool SessionManager::ExecuteSetSpeakerVolume(void)
{
    int tempvol;
    
    bodyToReturn = "Currently Speaker control is not implemented.";
    return false;
    
    if (!getRequiredArgAsInt("vol", 0, 100, tempvol))
        return false;
    
    // Else all is good.
    curSpeakerVol = tempvol;
    return true;
}

bool SessionManager::ExecuteGetSpeakerVolume(void)
{
    bodyToReturn = "Currently Speaker control is not implemented.";
    return false;
    
    strstream.str("");
    strstream << "{ vol: " << curSpeakerVol << ", muted: " << (bSpeakerMuted ? "true" : "false") << " }";
    bodyToReturn = strstream.str();
    return true;
}

bool SessionManager::ExecuteSetSpeakerToggleMute(void)
{
    bodyToReturn = "Currently Speaker control is not implemented.";
    return false;
    
    bSpeakerMuted = !bSpeakerMuted;
    return true;
}

#ifndef NDEBUG
bool SessionManager::ExecuteSendTestScript(void)
{
    string homedir;
    string fname;
    FILE *fp;
    
    homedir = getenv("HOME");
    fname =  homedir + "/TestSessionManager.html";
    fp = fopen(fname.c_str(), "r");
    
    
    if (!fp) {
        
        // Try again in local working directory.
        fp = fopen("./TestSessionManager.html", "r");
        
        if (!fp)
        {
            bodyToReturn = "'TestSessionManager.html' - file not found in ~ and not found in CWD (.)";
            return false;
        }
    }
    
    char ln[101];
    int bytesread;

    bodyToReturn = "";
    
    while (!feof(fp))
    {
        bytesread = fread(ln, 1, 100, fp);
        ln[bytesread] = 0;  // null forced.
        bodyToReturn += ln;
    }
    
    fclose(fp);
    return true;
}
#endif

#ifdef STANDALONE_TESTER
int main(int argc, char**argv)
{
    map<string, string> pairs;
  SessionManager tst(pairs, 8081);

  while(tst.isServerRunning())
	usleep(250 * 1000);

  return 0;
}
#endif

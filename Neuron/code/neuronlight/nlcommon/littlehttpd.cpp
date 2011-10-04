/*
 * LittleHttpd.cpp
 *
 *  Created on: Aug 23, 2011
 *      Author: rwolff
 */

#include "littlehttpd.h"

#include <string.h>

#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define MAXCLIENTCON 10

//#define DEBUG_LITTLEHTTPD

LittleHttpd::LittleHttpd(map<string, string> rvals, int initport)
{
	reqParameters.clear();

	bIsServerUp = false;
	serverError = 0;

	port = initport;
	serversock = 0;

	respvalues = rvals;

#if 0
	// Setup parent process/thread to have an 'ignore' handler so its getlin() doesnt get interrupted.
	struct sigaction sact;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
	sact.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &sact, NULL);
#endif

	startThread();
    
    while (!bInitComplete)
        usleep(20 * 1000);
}

LittleHttpd::~LittleHttpd()
{
	cout << "LittleHttpd: Shutting down...please wait..." << flush;

	// Fake-out thread stop just for this odd blocking accept() problem (to avoid non-blocking code at this time)
	isStopRequested = true;

	// Now formulate a 'wget' to handle the last (current) blocking accept()
	string syscmd("wget -q http://localhost:");
	stringstream finalget;
	finalget << port;
	syscmd += finalget.str();
	system(syscmd.c_str());

	stopThread();

	cout << "Complete." << endl;

	ShutdownServer();			// Should be done but just for completeness sake.

	// TODO unlisten(), unbind(), close() the socket(s)
}

void LittleHttpd::ShutdownServer(void)
{
	if (serversock)
		close(serversock);

	serversock = 0;
	bIsServerUp = false;
}

int LittleHttpd::workerBee(void)
{
//	struct sigaction sact;

	Init();
    
    // Server didn't come up. Likely due to bind error on old port not given up by kernel yet.
    if (!bIsServerUp)
        return -1;

#if 0
	// Pre-setup of sig handler due to alarm() being used for breaking out of blocking i/o
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
	sact.sa_handler = sighandler;
	sigaction(SIGALRM, &sact, NULL);
#endif

	while (serversock != -1)
	{
		struct sockaddr_in remAddress;
		socklen_t remAddress_len;
		int accept_sock;

		// Kill our thread at parent's request.
		if (isStopRequested)
		{
			// Close all socket stuff
			ShutdownServer();
			return 0;
		}

#if 0
		// Setup timer for 5 seconds to interrupt the blocking accept() call.
		alarm(5);
#endif

		// TODO This item is synchronous inside the thread. So quitting the thread won't happen
		//      unless one more connection comes in AFTER the quit is signalled.
		//      Change to async.
//		cout << "Prepping to wait (block) for an incoming connection right now." << endl;
		accept_sock = accept(serversock, (struct sockaddr*) &remAddress, &remAddress_len);
//		cout << "   accept() completed.  returned = " << accept_sock << ", errno = " << errno << endl;

		if (accept_sock >= 0)
		{
//			cout << "   Incoming connection was recevied" << endl;
			HConnection(accept_sock);
			close(accept_sock);
		}
		else
		{
			if (errno == EINTR)
			{
				cout << "Alarm went off. Continuing to see if we need to quit or try again." << endl;
				continue;
			}
			else
			{
				  perror("   errno string");
				  return EINTR;
			}
		}

	}

	return 0;
}

///
/// \brief init() shall setup the http server socket, bind to the port,
///        and then listen on that socket with a max limit of simultaneous
///         connections
///
void LittleHttpd::Init(void)
{
	struct sockaddr_in serverAddress;

	serverAddress.sin_port = htons(port);
	serverAddress.sin_family = AF_INET;

	serversock = socket(AF_INET, SOCK_STREAM, 0);

//	inet_aton("127.0.0.1", &mySockAddr.sin_addr);
//	inet_aton("0.0.0.0", &mySockAddr.sin_addr);

	// Bind to any address
	serverAddress.sin_addr.s_addr = htonl( INADDR_ANY );

	if (serversock == -1)
	{
		serverError = 1;
		cout << "Error: Unable to create server socket." << endl;
	}
	else if (bind(serversock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		serverError = 2;
		cout << "Error: Unable to bind on " << port << ". errno=" << errno << endl;
		perror("Error: errno description:");
	}
	else if (listen(serversock, MAXCLIENTCON) == -1)
	{
		serverError = 3;
		cout << "Error: Unable to listen on " << port << ". errno=" << errno << endl;
		perror("Error: errno description:");
	}
	else
	{
		bIsServerUp = true;
		cout << endl << "Success: server is ready on port " << port << endl;
	}
    
    bInitComplete = true;
}

string urlDecode(string &SRC) {
    string ret;
    char ch;
    unsigned int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}

bool LittleHttpd::HConnection(int csock)
{
	struct sockaddr_storage addr;
	socklen_t addrLen;
	ssize_t bytesRead;
	static char request[200];

    reqParameters.clear();

	addrLen = sizeof(addr);
	getpeername(csock, (struct sockaddr*)&addr, &addrLen);

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *cAddress = (struct sockaddr_in *)&addr;
//		port = ntohs(cAddress->sin_port);
		inet_ntop(AF_INET, &cAddress->sin_addr, clientIpAddress, sizeof(clientIpAddress));
	}
	else
		return false;

	bytesRead = read(csock, request, 99);
	if (bytesRead < 0)
	{
		cout << "Error: Read on connection failed. Bytes read reported as " << bytesRead << endl;
		return false;
	}
	else if (bytesRead == 0)
	{
		// This is an EOF condition.
		cout << "Connection ended." << endl;;
		return false;
	}

#ifdef DEBUG_LITTLEHTTPD
	cout << "Request received:'" << request << "'" << endl;;
#endif
    
    fullInboundURL = request;
    fullInboundURL = urlDecode(fullInboundURL);

    // Reset all responses prior to parsing.
    bodyToReturn = "";
    
    // Grab out the URL for derived classes who want to do a simple 'parse'.
    AutoParse();
    
    if (inboundBaseURL=="")
        return true;    // Jump out. No need to parse at the higher level.
    
	if (!ParseRequest())
	{
        SendBadRequestResponse(csock, bodyToReturn);
		return false;
	}

    // Now that the request is parsed, it should be held in the derived class' data structure.
    // Execute on it.
    
    if (!ExecuteAction())
	{
        SendBadRequestResponse(csock, bodyToReturn);
		return false;
	}
    else
        SendOKResponse(csock, bodyToReturn);
    
	return true;
}

void LittleHttpd::AutoParse(void)
{
    stringstream instr;
    unsigned int pos;
    
    inboundPairs.clear();
    
    pos = fullInboundURL.find("?");
    
    // No '?' in the URL -- so no parameters.
    if (pos==string::npos)
    {
        unsigned int poscmd;
        // Need to make sure to get at least the main command / base url.
        inboundBaseURL = fullInboundURL.substr(4, pos);
        poscmd = inboundBaseURL.find("/favicon");
        
        if (poscmd==0)
            inboundBaseURL = "";
        else
        {
            poscmd =inboundBaseURL.find("HTTP/");
            inboundBaseURL = inboundBaseURL.substr(0, poscmd);
        }
        return;
    }
    
    // Otherwise, we have at least a trailing ? so chop off the base and let's find the paired parameters.
    // Must skip the beginning "GET " command (hence the '4')
    inboundBaseURL = fullInboundURL.substr(4, pos-4);

    string chopped_req;
    
    // Prep for pairs...
    chopped_req = fullInboundURL.substr(pos+1, string::npos);

    // Now lop off the ending from "HTTP/"
    pos = chopped_req.find("HTTP/");
    if (pos!=string::npos)
    {
        // Take front end of string until HTTP/
        chopped_req = chopped_req.substr(0, pos);
    }
    
//    cout << "LittleHttpd:: After lopping-of-HTTP/ we have:" << chopped_req << endl;
    
#ifdef DEBUG_LITTLEHTTPD
    cout << "inbound base URL is: " << inboundBaseURL << endl;
    cout << "Leftover for pairs is: " << chopped_req << endl;
#endif
    
    // Now all lowercase for args.
    std::transform(chopped_req.begin(), chopped_req.end(), chopped_req.begin(), ::tolower);

    // Now get the pairs.
    instr.str(chopped_req);
    
    while(instr.good())
    {
        string name;
        string value;
        char buff[100];
        
        instr.getline(buff, 99, '=');
        
        if (instr.eof())
            break;
        
        while (strlen(buff) && buff[strlen(buff)-1]==' ')
            buff[strlen(buff)-1]=0;
        
        name = buff;
        
        // Eat pre-run whitespace after '='
        while (instr.peek()==' ')
            instr.get();
        
        // If next char is '"', we must eat all '"' if more than one and then read until '"'
        // else just read till next space.
        if (instr.peek()=='\"')
        {
            instr.get();
            
            // Now first '"' is read...now read till '"'
            instr.getline(buff, 99, '\"');
            
            // Eat all trailing ' ' spaces.
            while (buff[strlen(buff)-1]==' ')
                buff[strlen(buff)-1]=0;
            
            value = buff;
            
            while (instr.peek()=='&' || instr.peek()==' ')
                instr.get();
        }
        else
        {
            instr.getline(buff, 99, '&');
            value = buff;
        }
        
#ifdef DEBUG_LITTLEHTTPD
        cout << "Incoming PAIR: " << name << "=" << value << endl;
#endif
        
        inboundPairs[name]=value;

        // Already ate the '&' if there was one.
        while(instr.peek()==' ')
            instr.get();
    }
}

void LittleHttpd::SendBadRequestResponse(int csock, string &body)
{
    stringstream header;
    string headerString;
    
	// Now form the header. This is intertwined for a reason.
	// The "Content-Length: **" item must know the size of the body.
	// So it needs formed first and sent last.
    
	header << "HTTP/1.0 400 Bad Request\r\n";
	header << "Server: NeuronLight Server\r\n";
	header << "Content-Type: text/html; charset=UTF-8\r\n";
	header << "Content-Length: " << body.size() << "\r\n";
	header << "\r\n";
    
	headerString = header.str();
    
    //	cout << "LittleHttpd: Ready to send Header:" << headerString << endl;
    //	cout << "LittleHttpd: Ready to send Body:" << bodyString << endl;
    
	// Send header.
	send(csock, headerString.c_str(), headerString.size(), 0);
	// Send body.
    
    if (body.size())
        send(csock, body.c_str(), body.size(), 0);
    
    return;    
}

void LittleHttpd::SendOKResponse(int csock, string &body)
{
    stringstream header;
    string headerString;
    
	// Now form the header. This is intertwined for a reason.
	// The "Content-Length: **" item must know the size of the body.
	// So it needs formed first and sent last.
    
	header << "HTTP/1.0 200 OK\r\n";
	header << "Server: NeuronLight Server\r\n";
	header << "Content-Type: text/html; charset=UTF-8\r\n";
	header << "Content-Length: " << body.size() << "\r\n";
	header << "\r\n";
    
	headerString = header.str();
    
    //	cout << "LittleHttpd: Ready to send Header:" << headerString << endl;
    //	cout << "LittleHttpd: Ready to send Body:" << bodyString << endl;
    
	// Send header.
	send(csock, headerString.c_str(), headerString.size(), 0);
	// Send body.
	send(csock, body.c_str(), body.size(), 0);
    
    return;    
}

bool LittleHttpd::RequiredArgPresent(const char* reqarg)
{
    // Argument must be present.
    if (inboundPairs.find(reqarg)==inboundPairs.end()) {
        bodyToReturn = "Bad parameters. Expected '";
        bodyToReturn += reqarg;
        bodyToReturn += "'";
        return false;
    }
    else
        return true;
}

bool LittleHttpd::getRequiredArgAsInt(const char* reqarg, int low, int high, int& outint)
{
    outint = -1;
    
    if (!RequiredArgPresent(reqarg))
        return false;
    
    // Otherwise we're ready to grab the parameter and set it.
    int tempval = StrToInt(inboundPairs[reqarg]);
    
    if (tempval < low || tempval > high)
    {
        stringstream strm;
        
        strm << "Bad parameter " << reqarg << "=" << inboundPairs[reqarg] << ". Expected " << reqarg << "=[" << low << "-" << high << "]";
        
        bodyToReturn += strm.str();
        return false;
    }

    outint = tempval;
    return true;
}

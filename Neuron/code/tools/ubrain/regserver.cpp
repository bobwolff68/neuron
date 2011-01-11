/*
 * regserver.cpp
 *
 *  Created on: Dec 3, 2010
 *      Author: rwolff
 */

#include "regserver.h"

#include "string.h"

#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define MAXCLIENTCON 10

#define BAD_REQUEST "HTTP/1.0 400 Bad Request\r\n" \
					"Server: Neuron Server\r\n" \
					"Content-Type: text/html; charset=UTF-8\r\n" \
					"Content-Length: 0\r\n" \
					"\r\n"

RegServer::RegServer(uBrainManager* initBrain, map<string, string> rvals, int initport)
{
	assert(initBrain);

	theBrain = initBrain;
	tempID.clear();
	reqParameters.clear();

	bIsEndpoint = false;
	bIsServerUp = false;
	serverError = 0;
	globalID = (128 << 24) | 1;		// Start @ 2^31 and increment. This makes all ep-gid's HIGH and all non-ep-gid's start at zero.

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
}

RegServer::~RegServer()
{
	cout << "RegServer: Shutting down...please wait..." << flush;

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

void RegServer::ShutdownServer(void)
{
	if (serversock)
		close(serversock);

	serversock = 0;
	bIsServerUp = false;
}

int RegServer::workerBee(void)
{
//	struct sigaction sact;

	Init();

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
void RegServer::Init(void)
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
		cout << endl << "Success: http daemon server is up and ready to accept connections." << endl;
	}
}

bool RegServer::HConnection(int csock)
{
	struct sockaddr_storage addr;
	socklen_t addrLen;
	char clientIpAddress[INET6_ADDRSTRLEN];
	int bytesRead;
	char request[100];
	int temp_gid;

	stringstream header;
	string headerString;

	stringstream body;
	string bodyString;

	bIsEndpoint = false;		// Until proven true...
	tempID.clear();
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
		cout << "End of File/Connection." << endl;;
		return false;
	}

//	cout << "Request received:'" << request << "'" << endl;;

	if (!ParseRequest(request))
	{
		headerString = BAD_REQUEST;
		send(csock, headerString.c_str(), headerString.size(), 0);
		return false;
	}

	// Now we should have a proper flag
	tempID = reqParameters["tempid"];

	int sf_id=0;

	// Prep the body after grabbing final needed items.
	if (bIsEndpoint && theBrain->GetUniqueSFidAndMarkSFAsCreated(sf_id))
	{
		stringstream sfid_sstr;

		sfid_sstr << sf_id;
		if (!sfid_sstr)
		{
			headerString = BAD_REQUEST;
			send(csock, headerString.c_str(), headerString.size(), 0);
			return false;
		}

		respvalues["ep_sf_id"] = sfid_sstr.str();
	}

	respvalues["client_pub_ip"]=clientIpAddress;

	temp_gid = globalID++;				// Must increment immediately to avoid a conflict.
	respvalues["client_scp_id"] = ToString<int>(temp_gid);
    temp_gid = globalID++;              // Must increment immediately to avoid a conflict.
    respvalues["client_acp_id"] = ToString<int>(temp_gid);

	// Prep the 'static' and recently determined items.
	AddToStream(body, "client_pub_ip");
	AddToStream(body, "stun_ip");
    AddToStream(body, "ubrain_scp_desc");
	AddToStream(body, "ubrain_acp_desc");
    AddToStream(body, "client_scp_id");
    AddToStream(body, "client_acp_id");

    if (bIsEndpoint)
        AddToStream(body, "ep_sf_id");

	bodyString = body.str();

	cout << "INFO: RegServer: Full body sending back to client:" << endl << bodyString << endl;

	// Now form the header. This is intertwined for a reason.
	// The "Content-Length: **" item must know the size of the body.
	// So it needs formed first and sent last.

	header << "HTTP/1.0 200 OK\r\n";
	header << "Server: Neuron Server\r\n";
	header << "Content-Type: text/html; charset=UTF-8\r\n";
	header << "Content-Length: " << bodyString.size() << "\r\n";
	header << "\r\n";

	headerString = header.str();

//	cout << "RegServer: Ready to send Header:" << headerString << endl;
//	cout << "RegServer: Ready to send Body:" << bodyString << endl;

	// Send header.
	send(csock, headerString.c_str(), headerString.size(), 0);
	// Send body.
	send(csock, bodyString.c_str(), bodyString.size(), 0);

	// Now it's complete....let the uBrainManager:: know this fact.
	// GlobalID-1 is used since it was post-incremented above for safety.
	// And the actual 'globalID' is not used for safety in case it has been changed already.
	// TODO

	respvalues["ep_friendly_name"] = reqParameters["ep_friendly_name"];

	theBrain->RegistrationComplete(respvalues, bIsEndpoint);

	return true;
}

void RegServer::AddToStream(stringstream& outstream, const char* respname)
{
	if (respvalues[respname] != "")
		outstream << respname << "=" << respvalues[respname] << endl;
}

///
/// \brief ParseRequest must parse (in rudamentary non-defensive form) an incoming
///        HTTP GET request URI. In essence we're looking for "neuron-" to start
///        and then we'll split -sf or -ep and the inbound name/value pairs.
///
///        Note that the request will have appended to it " HTTP/1.x" so lop this off first.
///
///        This URI is something on the order of...
///             ^*neuron-{sf|ep}?[sp*]name[=value] [ [sp*]&[sp*]name[=value] .... ]
///        The simplified version for our purposes is...
///             ^*neuron-sf
///             ^*neuron-ep?name=["]value["]
///             Guarantee that '?' exists and 'name' is non-quoted and '=' exists
///
bool RegServer::ParseRequest(const char* req)
{
	stringstream reqstream;
	string chopped_req(req);
	int pos;

	// Strip off the front until we are at "neuron-" plus one character.
	pos = chopped_req.find("neuron-");
	if (pos==string::npos)
	{
//		cout << "Error: URI Input fail. No 'neuron-' - instead='" << req << "'" << endl;
		return false;
	}

	// Advance pointer beyond neuron-
	chopped_req = chopped_req.substr(pos+strlen("neuron-"), string::npos);

	cout << "RegServer:: Pre-lop-of-HTTP/ we have:" << chopped_req << endl;

	// Now lop off the ending from "HTTP/"
	pos = chopped_req.find("HTTP/");
	if (pos!=string::npos)
	{
		// Take front end of string until HTTP/
		chopped_req = chopped_req.substr(0, pos);
	}

	cout << "RegServer:: After lopping-of-HTTP/ we have:" << chopped_req << endl;

	if (chopped_req.substr(0,2)=="sf")
	{
		// We're done here.
		bIsEndpoint = false;

		return true;
	}
	else if (chopped_req.substr(0,2)=="ep")
	{
		assert(chopped_req[2]=='?');

		bIsEndpoint = true;

		chopped_req = chopped_req.substr(3, string::npos);

		// Now we are at the name/value pair.
		reqstream.str(chopped_req);

		while (reqstream.good())
		{
			string name;
			string value;
			char buff[100];

			reqstream.getline(buff, 99, '=');

			// If we have a failure here, it may just mean we're in a non-sense attrib list or at the end of a list.
			// Dont fail...just break so that the length of the name/value pair list will determine success.
			if (reqstream.eof())
				break;

			// Eat all trailing ' ' spaces.
			while (strlen(buff) && buff[strlen(buff)-1]==' ')
				buff[strlen(buff)-1]=0;

			name = buff;	// Copies buff string into name.

			// Moving on to the next part (the value - either quoted or not)
			// Eat pre-run-whitespace
			while (reqstream.peek()==' ')
				reqstream.get();

			// If next char is '"', we must eat all '"' if more than one and then read until '"'
			// else just read till next space.
			if (reqstream.peek()=='\"')
			{
				reqstream.get();

				// Now first '"' is read...now read till '"'
				reqstream.getline(buff, 99, '\"');

				// Eat all trailing ' ' spaces.
				while (buff[strlen(buff)-1]==' ')
					buff[strlen(buff)-1]=0;

				value = buff;
			}
			else
				reqstream >> value;

			cout << endl << "RegServer::Parse() - PAIR: " << name << "=" << value << endl;

			// Now what to do with the pair...
			reqParameters[name] = value;

			// And prepare to go around again by eating until '&' and then eating whitespace.
			while (reqstream.good() && reqstream.peek()!='&')
				reqstream.get();
			while (reqstream.good() && reqstream.peek()==' ')
				reqstream.get();
		}

		return true;
	}
	else
	{
		cout << "Error: URI Input fail. No '-ep' or '-sf' - instead='" << req << "'" << endl;
		return false;
	}

	assert(false);
	return false;
}

/*----------------------------------------------------------------------
 Portable function to set a socket into nonblocking mode.
 Calling this on a socket causes all future read() and write() calls on
 that socket to do only as much as they can immediately, and return
 without waiting.
 If no data can be read or written, they return -1 and set errno
 to EAGAIN (or EWOULDBLOCK).
 Thanks to Bjorn Reese for this code.
----------------------------------------------------------------------*/
int RegServer::setNonblocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

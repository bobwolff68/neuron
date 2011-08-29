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

#define BAD_REQUEST "HTTP/1.0 400 Bad Request\r\n" \
					"Server: Neuron Server\r\n" \
					"Content-Type: text/html; charset=UTF-8\r\n" \
					"Content-Length: 0\r\n" \
					"\r\n"

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
}

bool LittleHttpd::HConnection(int csock)
{
	struct sockaddr_storage addr;
	socklen_t addrLen;
	int bytesRead;
	static char request[200];
	int temp_acp_gid, temp_scp_gid;

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

//	cout << "Request received:'" << request << "'" << endl;;

	if (!ParseRequest(request))
	{
        SendBadRequestResponse(csock);
		return false;
	}

    // Now that the request is parsed, it should be held in the derived class' data structure.
    // Execute on it.
    
    if (!ExecuteAction())
	{
        SendBadRequestResponse(csock);
		return false;
	}
    else
        SendOKResponse(csock, bodyString);
    
	return true;
}

void LittleHttpd::AddToStream(stringstream& outstream, const char* respname)
{
	if (respvalues[respname] != "")
		outstream << respname << "=" << respvalues[respname] << endl;
}

void LittleHttpd::SendBadRequestResponse(int csock)
{
    string headerString;
    
    headerString = BAD_REQUEST;
    send(csock, headerString.c_str(), headerString.size(), 0);
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
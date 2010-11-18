/*
 * parsecmd.cpp
 *
 *  Originally Created on: Aug 4, 2010
 *  Significant changes for project 'ubrain' when created Nov 9th, 2010
 *      Author: rwolff
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "neuroncommon.h"
#include <strings.h>
#include <string.h>
#include "parsecmd.h"

//static class options {
//public:
	static string stunserver = "207.145.121.125";	// XVD T1-line direct connect server in 2010
	static string startupscript;
	static string logoutfile;
//
//};
//
//options options::_gOpts;

unsigned long router_ip;
unsigned long domain;
unsigned long bitrate;
char topicname[100];
char partname[100];
char peerList[10][100];
char stunLivePeriodStr[20];
char stunRetranIntvlStr[20];
char stunNumRetransStr[20];
int chunks;
int wanID;
bool bUseUDP;
bool bUseDefaultPeers;
bool bEnableMonitor;
bool bUseFlowCtrl;
long sizeSampleWindowForStats;

bool parsecmd(char**argv, int argc)
{
	router_ip = 0;
	domain = 0;
	bitrate = 0;
	topicname[0] = 0;
	partname[0] = 0;
	stunLivePeriodStr[0] = 0;
	stunRetranIntvlStr[0] = 0;
	stunNumRetransStr[0] = 0;
	bUseUDP = false;
	bEnableMonitor = false;
	bUseDefaultPeers = true;
	bUseFlowCtrl = false;
	chunks = 4;
	sizeSampleWindowForStats = 100;

	AnyOption *opt = new AnyOption();

	opt->addUsage("");
	opt->addUsage("Usage: ");
	opt->addUsage("");
	opt->addUsage(" -h  --help                    Prints this help ");
	opt->addUsage(
			" --stun <ip>[:<port>]      Address and port of stun server for udpwan.");
	opt->addUsage(
			" --script <scriptname>     Macro script file to run upon startup.");




	opt->addUsage(
			" -d  --domain <dom_number>     Set Domain# (default 0, monitor domain 100)");
	opt->addUsage(
			" -b  --bitrate <bitrate>       (For publisher) bitrate target in bits-per-second");
	opt->addUsage("                               (default is 0)");
	opt->addUsage(
			" -r  --router <router_ip>      If routing is required - add this to peers");
	opt->addUsage("                               (default is none)");
	opt->addUsage(
			" -i  --IP <UDP|TCP>            Force use of TCP or UDP (default is UDP)");
	opt->addUsage(
			" -t  --topic <name>            Topic to use - on reception, '*' can be used.");
	opt->addUsage("                               (default *)");
	opt->addUsage(
			" -p  --partition <name>        Partition to use - on reception, '*' can be used.");
	opt->addUsage("                               (default *)");
	opt->addUsage(
			" -l  --peerlist <loc>,<loc>,... List of peers to be discovered apart from shmem");
	opt->addUsage("                                and udplan.");
	opt->addUsage(
			" -w  --wanid <id>              WAN ID in case of UDP transport.");
	opt->addUsage(
			" -v  --slp <period>            STUN Liveliness Period (milliseconds).");
	opt->addUsage(
			" -m  --sri <period>            STUN Retransmission Interval (milliseconds).");
	opt->addUsage(
			" -n  --snr <number>            STUN No. of Retransmissions (milliseconds).");
	opt->addUsage(" -o  --monitor                 Enable monitoring.");
	opt->addUsage(
			" -f  --flowctrl                Enable preconfigured flow controller (Publisher only).");
	opt->addUsage(
			" -c  --chunksize               Set Chunk Size (Publisher only)(Default value: 4). ");
	opt->addUsage(
			" -e  --sampwin                 Sample Window Size (Subscriber only)(Default value: 100). ");
	opt->addUsage("");

	opt->setFlag("help", 'h'); /* a flag (takes no argument), supporting long and short form */
	opt->setFlag("monitor", 'o');
	opt->setFlag("flowctrl", 'f');
	opt->setOption("domain", 'd'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("bitrate", 'b'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("router", 'r'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("IP", 'i'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("topic", 't'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("partition", 'p'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("peerlist", 'l');
	opt->setOption("stun", 's');
	opt->setOption("wanid", 'w');
	opt->setOption("slp", 'v');
	opt->setOption("sri", 'm');
	opt->setOption("snr", 'n');
	opt->setOption("chunksize", 'c');
	opt->setOption("sampwin", 'e');

	opt->processCommandArgs(argc, argv);

	/*if( ! opt->hasOptions()) { 
	 opt->printUsage();
	 delete opt;
	 return false;
	 }*/

	/* 6. GET THE VALUES */
	if (opt->getFlag("help") || opt->getFlag('h'))
	{
		opt->printUsage();
		exit(1);
	}

	if (opt->getValue("stun") != NULL)
	{
		// For now only default STUN port used
		if (bUseUDP)
		{
			stunserver = opt->getValue("stun");
			cout << "STUN server ip address: " << stunserver << endl;
		}
	}

	if (opt->getValue("script") != NULL)
	{
		startupscript = opt->getValue("script");
		cout << "Initial script is: " << startupscript << endl;
	}





	if (opt->getValue('i') != NULL || opt->getValue("IP") != NULL)
	{
		bUseUDP = !strncasecmp(opt->getValue('i'), "UDP", 3);
		bUseDefaultPeers = false;
		if (bUseUDP)
			cout << "Using UDP for transport." << endl;
		else
			cout << "Using TCP for transport." << endl;
	}

	if (opt->getValue('t') != NULL || opt->getValue("topic") != NULL)
	{
		strncpy(topicname, opt->getValue('t'), 99);
		cout << "topicname = " << topicname << endl;
	}

	if (opt->getValue('p') != NULL || opt->getValue("partition") != NULL)
	{
		strncpy(partname, opt->getValue('p'), 99);
		cout << "partitionname = " << partname << endl;
	}

	if (opt->getValue('d') != NULL || opt->getValue("domain") != NULL)
	{
		domain = atol(opt->getValue('d'));
		cout << "domain = " << domain << endl;
	}

	if (opt->getValue('b') != NULL || opt->getValue("bitrate") != NULL)
	{
		bitrate = atol(opt->getValue('b'));
		cout << "bitrate = " << bitrate << " (" << bitrate / 1024 << "Kbps)"
				<< endl;
	}

	if (opt->getValue('r') != NULL || opt->getValue("router") != NULL)
	{
		router_ip = inet_addr(opt->getValue('r'));
		cout << "router = " << opt->getValue('r') << endl;
	}

	// For now, assume only one peer entered
	if (opt->getValue('l') != NULL || opt->getValue("peerlist") != NULL)
	{
		if (bUseUDP)
		{
			strcpy(peerList[0], "0@wan://::");
			strcat(peerList[0], opt->getValue('l'));
			//strcat(peerList[0],":127.0.0.1");
		}
		else
		{
			strcpy(peerList[0], "tcpv4_lan://");
			strcat(peerList[0], opt->getValue('l'));
		}
		cout << "PeerList: " << peerList[0] << endl;
	}

	if (opt->getValue('w') != NULL || opt->getValue("wanid") != NULL)
	{
		// For now only default STUN port used
		if (bUseUDP)
		{
			sscanf(opt->getValue('w'), "%d", &wanID);
			cout << "WAN ID: " << wanID << endl;
		}
	}

	if (opt->getValue('v') != NULL || opt->getValue("slp") != NULL)
	{
		if (bUseUDP)
		{
			strcpy(stunLivePeriodStr, opt->getValue('v'));
			cout << "STUN Liveliness Period: " << stunLivePeriodStr
					<< " (milliseconds)" << endl;
		}
	}

	if (opt->getValue('m') != NULL || opt->getValue("sri") != NULL)
	{
		if (bUseUDP)
		{
			strcpy(stunRetranIntvlStr, opt->getValue('m'));
			cout << "STUN Retransmission Interval: " << stunRetranIntvlStr
					<< " (milliseconds)" << endl;
		}
	}

	if (opt->getValue('n') != NULL || opt->getValue("snr") != NULL)
	{
		if (bUseUDP)
		{
			strcpy(stunNumRetransStr, opt->getValue('n'));
			cout << "STUN No. of Retransmissions: " << stunNumRetransStr
					<< endl;
		}
	}

	if (opt->getFlag('o') || opt->getFlag("monitor"))
	{
		bEnableMonitor = true;
		cout << "Monitoring Enabled." << endl;
	}

	if (opt->getFlag('f') || opt->getFlag("flowctrl"))
	{
		bUseFlowCtrl = true;
		cout << "Default Flow Controller Enabled." << endl;
	}

	if (opt->getValue('c') != NULL || opt->getValue("chunksize") != NULL)
	{
		sscanf(opt->getValue('c'), "%d", &chunks);
		cout << "Chunk Size: " << chunks << endl;
	}

	if (opt->getValue('e') != NULL || opt->getValue("sampwin") != NULL)
	{
		sscanf(opt->getValue('e'), "%ld", &sizeSampleWindowForStats);
		cout << "Sample Window: " << sizeSampleWindowForStats << endl;
	}

	delete opt;
	return true;
}

ParseCmd::ParseCmd()
{
	// TODO Auto-generated constructor stub

}

ParseCmd::~ParseCmd()
{
	// TODO Auto-generated destructor stub
}

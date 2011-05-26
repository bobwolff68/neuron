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
	string stunserver = "207.145.121.125";	// XVD T1-line direct connect server in 2010
	string startupscript = "";
	string logoutfile = "";
	string ubrain_ip = "50.18.56.81";
	string luascripts[16];
	int luascriptsInUse = 0;
//
//};
//
//options options::_gOpts;

unsigned long domain;
char peerList[10][100];
char stunLivePeriodStr[20];
char stunRetranIntvlStr[20];
char stunNumRetransStr[20];
int wanID;
bool bUseUDP;
bool bUseDefaultPeers;
bool bEnableMonitor;
bool bUseLANOnly;
//bool bUseFlowCtrl;
//long sizeSampleWindowForStats;

bool parsecmd(int argc, char**argv)
{
    bUseLANOnly = false;
	domain = 0;
	stunLivePeriodStr[0] = 0;
	stunRetranIntvlStr[0] = 0;
	stunNumRetransStr[0] = 0;
	bUseUDP = true;
	bEnableMonitor = false;
	bUseDefaultPeers = true;
//	bUseFlowCtrl = false;
//	sizeSampleWindowForStats = 100;

	AnyOption *opt = new AnyOption();

	opt->addUsage("");
	opt->addUsage("Usage: ");
	opt->addUsage("");
	opt->addUsage(" -h  --help                    Prints this help ");
    opt->addUsage(
            " --ubrain <public_ip>      Sets the publicly available IP for http: requests/registration");
	opt->addUsage(
			" --stun <ip>[:<port>]      Address and port of stun server for udpwan.");
	opt->addUsage(
			" --script <scriptname>     Macro script file to run upon startup.");
	opt->addUsage(
			" --luascripts <scriptname.lua>[,scriptname2.lua...]     Lua script(s) to run upon startup.");
	opt->addUsage(
			" --luascript <scriptname.lua>     Single Lua script to run upon startup.");
    opt->addUsage(
            " --lanonly                 Operate without STUN, WAN, etc. LAN only. (default is WAN)");




    opt->addUsage(
            " -d  --domain <dom_number>     Set Domain# (default 0, monitor domain 100)");
	opt->addUsage(
			" -i  --IP <UDP|TCP>            Force use of TCP or UDP (default is UDP)");
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
//	opt->addUsage(
//			" -f  --flowctrl                Enable preconfigured flow controller (Publisher only).");
//	opt->addUsage(
//			" -e  --sampwin                 Sample Window Size (Subscriber only)(Default value: 100). ");
	opt->addUsage("");

	opt->setFlag("help", 'h'); /* a flag (takes no argument), supporting long and short form */
    opt->setFlag("lanonly");
    opt->setOption("ubrain");
    opt->setOption("stun");
    opt->setOption("script");
    opt->setOption("luascripts");
    opt->setOption("luascript");

    opt->setFlag("monitor", 'o');
//	opt->setFlag("flowctrl", 'f');
	opt->setOption("domain", 'd'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("IP", 'i'); /* an option (takes an argument), supporting long and short form */
	opt->setOption("peerlist", 'l');

	opt->setOption("wanid", 'w');
	opt->setOption("slp", 'v');
	opt->setOption("sri", 'm');
	opt->setOption("snr", 'n');
//	opt->setOption("sampwin", 'e');

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

    if (opt->getValue("lanonly"))
    {
        cout << "Forcing the use of **LAN** only. No WAN connections." << endl;
        stunserver = "";
	bUseLANOnly = true;
    }

    if (opt->getValue("ubrain") != NULL)
        ubrain_ip = opt->getValue("ubrain");

    cout << "Public registration server/ubrain ip address: " << ubrain_ip << endl;

    if (opt->getValue("stun") != NULL)
    {
        // For now only default STUN port used
        if (bUseUDP)
            stunserver = opt->getValue("stun");
        else
        {
            cout << "Error: --stun is not valid in TCP mode. Use UDP." << endl;
            exit(2);
        }
    }

    if (stunserver!="")
        cout << "STUN server ip address: " << stunserver << endl;

    if (opt->getValue("script") != NULL)
    {
        startupscript = opt->getValue("script");
        cout << "Initial script is: " << startupscript << endl;
    }

    if (opt->getValue("luascripts") != NULL)
    {
	char scr[100];
	stringstream allscripts(opt->getValue("luascripts"));
	
	while (allscripts)
	{
		allscripts.getline(scr, 99, ',');
		
		if (*scr == 0) break;	// Last entry read already.
		
//		cout << "--luascripts Iterating: Current script==" << scr << endl;
		luascripts[luascriptsInUse++] = scr;
	}
    }
    
    if (opt->getValue("luascript") != NULL)
    {
        if (luascriptsInUse)
	{
	  cout << "Single lua script OVERRIDES prior --luascripts option. Ignoring --luascripts" << endl;
	  luascriptsInUse=0;
	}
	
        luascripts[luascriptsInUse++] = opt->getValue("luascript");
//        cout << "Single lua script listed is: " << luascripts[luascriptsInUse-1] << endl;
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

	if (opt->getValue('d') != NULL || opt->getValue("domain") != NULL)
	{
		domain = atol(opt->getValue('d'));
		cout << "domain = " << domain << endl;
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

#if 0
	if (opt->getFlag('f') || opt->getFlag("flowctrl"))
	{
		bUseFlowCtrl = true;
		cout << "Default Flow Controller Enabled." << endl;
	}

	if (opt->getValue('e') != NULL || opt->getValue("sampwin") != NULL)
	{
		sscanf(opt->getValue('e'), "%ld", &sizeSampleWindowForStats);
		cout << "Sample Window: " << sizeSampleWindowForStats << endl;
	}
#endif

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

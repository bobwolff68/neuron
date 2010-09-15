/*
 * parsecmd.cpp
 *
 *  Created on: Aug 4, 2010
 *      Author: rwolff
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "anyoption/anyoption.h"
#include <strings.h>
#include <string.h>

unsigned long router_ip;
unsigned long domain;
unsigned long bitrate;
char topicname[100];
char partname[100];
char stunLocator[100];
char peerList[10][100];
int  wanID;
bool bUseUDP;

bool parsecmd(char**argv, int argc)
{
  router_ip = 0;
  domain = 0;
  bitrate = 0;
  topicname[0] = 0;
  partname[0] = 0;
  bUseUDP = true;

  AnyOption *opt = new AnyOption();

  opt->addUsage( "" );
  opt->addUsage( "Usage: " );
  opt->addUsage( "" );
  opt->addUsage( " -h  --help                    Prints this help " );
  opt->addUsage( " -d  --domain <dom_number>     Set Domain# (default 0)" );
  opt->addUsage( " -b  --bitrate <bitrate>       (For publisher) bitrate target in bits-per-second");
  opt->addUsage( "                               (default is 0)" );
  opt->addUsage( " -r  --router <router_ip>      If routing is required - add this to peers");
  opt->addUsage( "                               (default is none)" );
  opt->addUsage( " -i  --IP <UDP|TCP>            Force use of TCP or UDP (default is UDP)" );
  opt->addUsage( " -t  --topic <name>            Topic to use - on reception, '*' can be used.");
  opt->addUsage( "                               (default *)");
  opt->addUsage( " -p  --partition <name>        Partition to use - on reception, '*' can be used.");
  opt->addUsage( "                               (default *)");
  opt->addUsage( " -l  --peerlist <loc>,<loc>,... List of peers to be discovered apart from shmem" );
  opt->addUsage( "                                and udplan." );
  opt->addUsage( " -s  --stun <ip>[:<port>]      Address and port of stun server for udpwan." );
  opt->addUsage( " -w  --wanid <id>              WAN ID in case of UDP transport." );
  opt->addUsage( "" );

  opt->setFlag(  "help", 'h' );   /* a flag (takes no argument), supporting long and short form */
  opt->setOption(  "domain",    'd' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "bitrate",   'b' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "router",    'r' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "IP",        'i' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "topic",     't' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "partition", 'p' ); /* an option (takes an argument), supporting long and short form */
  opt->setOption(  "peerlist",  'l' );
  opt->setOption(  "stun",      's' );
  opt->setOption(  "wanid",     'w' );

  opt->processCommandArgs( argc, argv );

  if( ! opt->hasOptions()) { /* print usage if no options */
          opt->printUsage();
          delete opt;
          return false;
  }

  /* 6. GET THE VALUES */
  if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) )
  {
          opt->printUsage();
          exit(1);
  }

  if( opt->getValue( 'i' ) != NULL  || opt->getValue( "IP" ) != NULL  )
  {
          bUseUDP = !strncasecmp(opt->getValue('i'), "UDP", 3);
          if (bUseUDP)
            cout << "Using UDP for transport." << endl ;
          else
            cout << "Using TCP for transport." << endl ;
  }

  if( opt->getValue( 't' ) != NULL  || opt->getValue( "topic" ) != NULL  )
  {
          strncpy(topicname, opt->getValue('t'), 99);
          cout << "topicname = " << topicname << endl ;
  }

  if( opt->getValue( 'p' ) != NULL  || opt->getValue( "partition" ) != NULL  )
  {
          strncpy(partname, opt->getValue('p'), 99);
          cout << "partitionname = " << partname << endl ;
  }

  if( opt->getValue( 'd' ) != NULL  || opt->getValue( "domain" ) != NULL  )
  {
          domain = atol(opt->getValue('d'));
          cout << "domain = " << domain << endl ;
  }

  if( opt->getValue( 'b' ) != NULL || opt->getValue( "bitrate" ) != NULL )
  {
          bitrate = atol(opt->getValue('b'));
          cout << "bitrate = " << bitrate << " (" << bitrate/1024 << "Kbps)" << endl ;
  }

  if( opt->getValue( 'r' ) != NULL || opt->getValue( "router" ) != NULL )
  {
          router_ip = inet_addr(opt->getValue('r'));
          cout << "router = " << opt->getValue('r') << endl ;
  }

  // For now, assume only one peer entered
  if( opt->getValue( 'l' ) != NULL || opt->getValue( "peerlist" ) != NULL )
  {
          if(bUseUDP)	
          {
          		strcpy(peerList[0],"wan://::");
  		  		strcat(peerList[0],opt->getValue('l'));
  		  		//strcat(peerList[0],":127.0.0.1");
  		  }
  		  else
  		  {
          		strcpy(peerList[0],"tcpv4_lan://");
  		  		strcat(peerList[0],opt->getValue('l'));
  		  }
          cout << "PeerList: " << peerList[0] << endl ;
  }

  if( opt->getValue( 's' ) != NULL || opt->getValue( "stun" ) != NULL )
  {
  		  // For now only default STUN port used
          if(bUseUDP)	
          {
          		strcpy(stunLocator,opt->getValue('s'));
          		cout << "STUN server ip address: " << stunLocator << endl ;
          }
  }
  
  if( opt->getValue( 'w' ) != NULL || opt->getValue( "wanid" ) != NULL )
  {
  		  // For now only default STUN port used
          if(bUseUDP)	
          {
          		sscanf(opt->getValue('w'),"%d",&wanID);
          		cout << "WAN ID: " << wanID << endl ;
          }
  }
  
  delete opt;
  return true;
}


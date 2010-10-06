#include <stdio.h>
#include "ndds/ndds_cpp.h"
#include "monitor/monitor_common.h"
#include "cfc.h"
#include "cfcSupport.h"
#include "cfc_transport_config.h"

#define CHECK_RETCODE(retCode,retCodeVal,errMsg)	if(retCode!=retCodeVal)\
													{\
														printf(errMsg);\
														exit(0);\
													}
#define ERROUT(str)	fprintf(stderr,str);

int participant_qos_with_monitoring_enabled(DDS_DomainParticipantQos& partqos)
{
// Could allow a default param for 'verbosity' = 1 to allow external change to higher values.
//
// Based upon RTI support call, simply load the default qos profile instead of
// specifying a particular one.
// Then either have a simple USER_QOS_PROFILES.xml which has the necessary items
// *OR* then use the programmatic access below for using monitoring by adding properties.
//
// Using programmatic version here.

	int retcode;

    printf("Using C++ explicit adding of properties for Monitoring enablement.\n");
#ifndef NDEBUG
    retcode = DDSPropertyQosPolicyHelper::add_property(partqos.property, 
                "rti.monitor.config.verbosity", "1", DDS_BOOLEAN_FALSE);
    if (retcode != DDS_RETCODE_OK)
    {
        ERROUT("add property for VERBOSITY - Debug only");
        return -1;
    }
    
    const char *libnam = "rtimonitoringd";
#else
    const char *libnam = "rtimonitoring";
#endif
    
    // Configure monitoring info to be published by a separate domain participant
    // on a different domain.
    retcode  = DDSPropertyQosPolicyHelper::add_property(partqos.property, 
                "rti.monitor.config.new_participant_domain_id", MONITOR_DP_DOMAIN_ID_STR, DDS_BOOLEAN_FALSE);
    if (retcode != DDS_RETCODE_OK)
    {
        ERROUT("add property for different domain for monitor data");
        return -2;
    }
         
    
    retcode = DDSPropertyQosPolicyHelper::add_property(partqos.property, 
                "rti.monitor.library", libnam, DDS_BOOLEAN_FALSE);
    if (retcode != DDS_RETCODE_OK)
    {
        ERROUT("add property for library load");
        return -3;
    }

    char valueBuf[17];
    sprintf(valueBuf, "%p", RTIDefaultMonitor_create);

    retcode = DDSPropertyQosPolicyHelper::add_property(partqos.property, "rti.monitor.create_function_ptr",
                valueBuf, DDS_BOOLEAN_FALSE);
    if (retcode != DDS_RETCODE_OK)
    {
        ERROUT("add property for monitor entrypoint function");
        return -4;
    }

	return 0;
}

void DDS_DomainParticipantQos_setup_udpv4_message_size_max(DDS_DomainParticipantQos &dpQos,const char *msg_size_max_str)
{
	int					msg_size_max;
	DDS_ReturnCode_t	retCode;
	
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.UDPv4.builtin.parent.message_size_max",
													   msg_size_max_str,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.UDPv4.builtin.parent.message_size_max)\n");
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.UDPv4.builtin.send_socket_buffer_size",
													   msg_size_max_str,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.UDPv4.builtin.send_socket_buffer_size)\n");
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.UDPv4.builtin.recv_socket_buffer_size",
													   msg_size_max_str,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.UDPv4.builtin.recv_socket_buffer_size)\n");	
	
	//Configure for shmem
	/*retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.shmem.parent.message_size_max",
													   msg_size_max_str,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.shmem.parent.message_size_max)\n");
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.shmem.receive_buffer_size",
													   msg_size_max_str,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.shmem.receive_buffer_size)\n");*/
		
	
	sscanf(msg_size_max_str,"%d",&msg_size_max);
	dpQos.receiver_pool.buffer_size = msg_size_max;
	printf("Max Message Size: %d\n",dpQos.receiver_pool.buffer_size);

	return;
}

DDS_DomainParticipantQos DPQos_with_UDPWAN(const char *stunLocator,int wanID,const char *stunLivePeriodStr,
										   const char *stunRetranIntvlStr, const char *stunNumRetransStr,
										   bool bEnableMonitor)
{
	DDS_DomainParticipantQos	dpQos;
	DDS_ReturnCode_t			retCode;
	char						wanIDStr[20];
	int							msg_size_max;

	// Get default domain participant QOS	
	retCode = DDSTheParticipantFactory->get_default_participant_qos(dpQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: get_default_participant_qos()\n");

	// Disable built-in transports
	if(!bEnableMonitor)
		dpQos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;
	// Add property to load plugin
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.load_plugins",
													   "dds.transport.wan_plugin.wan", 
    												   DDS_BOOLEAN_FALSE);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.load_plugins)\n");
	// Add property to load transport plugin library
#ifndef NDEBUG
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.wan_plugin.wan.verbosity",
													   "5",DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.verbosity)\n");
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.wan_plugin.wan.library",
													   "libnddstransportwand.so",DDS_BOOLEAN_FALSE);
#else
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.wan_plugin.wan.library",
													   "libnddstransportwan.so",DDS_BOOLEAN_FALSE);
#endif
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.library)\n");
	// Add property to specify create function
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													"dds.transport.wan_plugin.wan.create_function",
													"NDDS_Transport_WAN_create",DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.create_function)\n");
	// Add property to specify the address of the STUN server
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.wan_plugin.wan.server",
													   stunLocator,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.server)\n");
	// Add property to set the WAN ID for the application
	sprintf(wanIDStr,"%d",wanID);
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.wan_plugin.wan.transport_instance_id",
													   wanIDStr,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.transport_instance_id)\n");
	// Add property to set the stun liveliness period
	if(stunLivePeriodStr[0]!=0)
	{
		retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
														   "dds.transport.wan_plugin.wan.stun_liveliness_period",
														   stunLivePeriodStr,DDS_BOOLEAN_FALSE);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.stun_liveliness_period)\n");	
	}
	// Add property to set the stun retransmission interval
	if(stunRetranIntvlStr[0]!=0)
	{
		retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
														   "dds.transport.wan_plugin.wan.stun_retransmission_interval",
														   stunRetranIntvlStr,DDS_BOOLEAN_FALSE);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.stun_retransmission_interval)\n");	
	}
	// Add property to set the stun number of retransmissions
	if(stunNumRetransStr[0]!=0)
	{
		retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
														   "dds.transport.wan_plugin.wan.stun_number_of_retransmissions",
														   stunNumRetransStr,DDS_BOOLEAN_FALSE);
		CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.wan_plugin.wan.stun_number_of_retransmissions)\n");	
	}
	// Enable Monitoring if specified
	if(bEnableMonitor)	
	{
		if(participant_qos_with_monitoring_enabled(dpQos)<0)
			exit(0);
	}
	
	// Modify message_size_max to 65535 bytes
	//DDS_DomainParticipantQos_setup_udpv4_message_size_max(dpQos,MSG_SIZE_MAX_BYTES_STR);
	
	return dpQos;
}

DDS_DomainParticipantQos DPQos_with_TCPLAN(int bindPort,bool bEnableMonitor)
{
	DDS_DomainParticipantQos	dpQos;
	DDS_ReturnCode_t			retCode;
	char						bindPortStr[10];

	// Get default domain participant QOS	
	retCode = DDSTheParticipantFactory->get_default_participant_qos(dpQos);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: get_default_participant_qos()\n");

	// Disable built-in transports
	if(!bEnableMonitor)
		dpQos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;
	// Add property to load plugin
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,"dds.transport.load_plugins",
													   "dds.transport.TCPv4.tcp1", 
    												   DDS_BOOLEAN_FALSE);
    CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.load_plugins)\n");
	// Add property to load transport plugin library
#ifndef NDEBUG
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.TCPv4.tcp1.library",
													   "libnddstransporttcpd.so",DDS_BOOLEAN_FALSE);
#else
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.TCPv4.tcp1.library",
													   "libnddstransporttcp.so",DDS_BOOLEAN_FALSE);
#endif
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.TCPv4.tcp1.library)\n");
	// Add property to specify create function
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													"dds.transport.TCPv4.tcp1.create_function",
													"NDDS_Transport_TCPv4_create",
													DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.TCPv4.tcp1.create_function)\n");
	// Add property to specify the tcp parent class id
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.TCPv4.tcp1.parent.classid",
													   "NDDS_TRANSPORT_CLASSID_TCPV4_LAN",
													   DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.TCPv4.tcp1.parent.classid)\n");
	// Add property to set the bind port
	sprintf(bindPortStr,"%d",bindPort);
	retCode = DDSPropertyQosPolicyHelper::add_property(dpQos.property,
													   "dds.transport.TCPv4.tcp1.server_bind_port",
													   bindPortStr,DDS_BOOLEAN_FALSE);
	CHECK_RETCODE(retCode,DDS_RETCODE_OK,"Error: add_property(dds.transport.TCPv4.tcp1.server_bind_port)\n");

	if(bEnableMonitor)	
	{
		if(participant_qos_with_monitoring_enabled(dpQos)<0)
			exit(0);
	}

	return dpQos;
}


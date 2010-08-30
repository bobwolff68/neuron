/* HelloWorld_publisher.cxx

   A publication of data of type HelloWorld

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C++ -example <arch> HelloWorld.idl

   Example publication of type HelloWorld automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription on the same domain used for RTI Data Distribution
       with the command
       objs/<arch>/HelloWorld_subscriber <domain_id> <sample_count>
                
   (3) Start the publication on the same domain used for RTI Data Distribution
       with the command
       objs/<arch>/HelloWorld_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.

                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       On Unix: 
       
       objs/<arch>/HelloWorld_publisher <domain_id> o
       objs/<arch>/HelloWorld_subscriber <domain_id> 
                            
       On Windows:
       
       objs\<arch>\HelloWorld_publisher <domain_id>  
       objs\<arch>\HelloWorld_subscriber <domain_id>    

       
modification history
------------ -------       
*/

#include <stdio.h>
#include <stdlib.h>
#include "HelloWorld.h"
#include "HelloWorldSupport.h"
#include "ndds/ndds_cpp.h"

/* Delete all entities */
static int publisher_shutdown(
    DDSDomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Data Distribution Service provides finalize_instance() method on
       domain participant factory for people who want to release memory used
       by the participant factory. Uncomment the following block of code for
       clean destruction of the singleton. */
/*
    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }
*/

    return status;
}

extern "C" int publisher_main(int domainId, int sample_count)
{
    DDSDomainParticipant *participant = NULL;
    DDSPublisher *publisher1 = NULL;
    DDSPublisher *publisher2 = NULL;
    DDSPublisher *publisher3 = NULL;
    DDSTopic *topic = NULL;
    DDSDataWriter *writer1 = NULL;
    DDSDataWriter *writer2 = NULL;
    DDSDataWriter *writer3 = NULL;
    HelloWorldDataWriter * HelloWorld_writer1 = NULL;
    HelloWorldDataWriter * HelloWorld_writer2 = NULL;
    HelloWorldDataWriter * HelloWorld_writer3 = NULL;
    HelloWorld *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;  
    DDS_Duration_t send_period = {4,0};

    /* To customize participant QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    participant = DDSTheParticipantFactory->create_participant(
        domainId, DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize publisher QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
	DDS_PublisherQos publisher1Qos;
	participant->get_default_publisher_qos(publisher1Qos);
    publisher1Qos.partition.name.ensure_length(1,1);
    publisher1Qos.partition.name[0] = DDS_String_dup("name/L0");
    publisher1 = participant->create_publisher(
        publisher1Qos, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (publisher1 == NULL) {
        printf("create_publisher1 error\n");
        publisher_shutdown(participant);
        return -1;
    }


	DDS_PublisherQos publisher2Qos;
	participant->get_default_publisher_qos(publisher2Qos);
    publisher2Qos.partition.name.ensure_length(1,1);
    publisher2Qos.partition.name[0] = DDS_String_dup("name/L1");
    publisher2 = participant->create_publisher(
        publisher2Qos, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (publisher2 == NULL) {
        printf("create_publisher2 error\n");
        publisher_shutdown(participant);
        return -1;
    }

	DDS_PublisherQos publisher3Qos;
	participant->get_default_publisher_qos(publisher3Qos);
    publisher3Qos.partition.name.ensure_length(1,1);
    publisher3Qos.partition.name[0] = DDS_String_dup("name/L2");
    publisher3 = participant->create_publisher(
        publisher3Qos, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (publisher3 == NULL) {
        printf("create_publisher3 error\n");
        publisher_shutdown(participant);
        return -1;
    }


    /* Register type before creating topic */
    type_name = HelloWorldTypeSupport::get_type_name();
    retcode = HelloWorldTypeSupport::register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    topic = participant->create_topic(
        "Example HelloWorld",
        type_name, DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize data writer QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    writer1 = publisher1->create_datawriter(
        topic, DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (writer1 == NULL) {
        printf("create_datawriter1 error\n");
        publisher_shutdown(participant);
        return -1;
    }

    writer2 = publisher2->create_datawriter(
        topic, DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (writer2 == NULL) {
        printf("create_datawriter2 error\n");
        publisher_shutdown(participant);
        return -1;
    }

    writer3 = publisher3->create_datawriter(
        topic, DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (writer1 == NULL) {
        printf("create_datawriter3 error\n");
        publisher_shutdown(participant);
        return -1;
    }




    HelloWorld_writer1 = HelloWorldDataWriter::narrow(writer1);
    if (HelloWorld_writer1 == NULL) {
        printf("DataWriter1 narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }


    HelloWorld_writer2 = HelloWorldDataWriter::narrow(writer2);
    if (HelloWorld_writer2 == NULL) {
        printf("DataWriter2 narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }


    HelloWorld_writer3 = HelloWorldDataWriter::narrow(writer3);
    if (HelloWorld_writer3 == NULL) {
        printf("DataWriter3 narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }




    /* Create data sample for writing */
    instance = HelloWorldTypeSupport::create_data();
    if (instance == NULL) {
        printf("HelloWorldTypeSupport::create_data error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* For a data type that has a key, if the same instance is going to be
       written multiple times, initialize the key here
       and register the keyed instance prior to writing */
/*
    instance_handle = HelloWorld_writer->register_instance(*instance);
*/

    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {

        printf("Writing HelloWorld, count %d\n", count);

        /* Modify the data to be sent here */
		sprintf(instance->prefix, "Hello writer #1 Sample %d\n", count);

        retcode = HelloWorld_writer1->write(*instance, instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            printf("write1 error %d\n", retcode);
        }

		sprintf(instance->prefix, "Hello writer #2 Sample %d\n", count);

        retcode = HelloWorld_writer2->write(*instance, instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            printf("write2 error %d\n", retcode);
        }
		sprintf(instance->prefix, "Hello writer #3 Sample %d\n", count);

        retcode = HelloWorld_writer3->write(*instance, instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            printf("write3 error %d\n", retcode);
        }

        NDDSUtility::sleep(send_period);
    }

/*
    retcode = HelloWorld_writer->unregister_instance(
        *instance, instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        printf("unregister instance error %d\n", retcode);
    }
*/

    /* Delete data sample */
    retcode = HelloWorldTypeSupport::delete_data(instance);
    if (retcode != DDS_RETCODE_OK) {
        printf("HelloWorldTypeSupport::delete_data error %d\n", retcode);
    }

    /* Delete all entities */
    return publisher_shutdown(participant);
}


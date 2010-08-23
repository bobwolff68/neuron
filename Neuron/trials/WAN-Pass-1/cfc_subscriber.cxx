/* cfc_subscriber.cxx

   A subscription example

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C++ -example <arch> cfc.idl

   Example subscription of type cfc automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example publication.

   (2) Start the subscription on the same domain used for RTI Data Distribution
       Service with the command
       objs/<arch>/cfc_subscriber <domain_id> <sample_count>

   (3) Start the publication on the same domain used for RTI Data Distribution
       with the command
       objs/<arch>/cfc_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
              
                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       On Unix: 
       
       objs/<arch>/cfc_publisher <domain_id> 
       objs/<arch>/cfc_subscriber <domain_id> 
                            
       On Windows:
       
       objs\<arch>\cfc_publisher <domain_id>  
       objs\<arch>\cfc_subscriber <domain_id>   
              
       
modification history
------------ -------       
* Add clock to show the time we get each sample.
*/

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_cpp.h"
#include "cfc.h"
#include "cfcSupport.h"

//// Changes for Custom_Flowcontroller
// For timekeeping
#include <time.h>
clock_t init;

#define CLK_TCK CLOCKS_PER_SEC

extern unsigned long router_ip;
extern unsigned long domain;
extern unsigned long bitrate;
extern char topicname[100];
extern char partname[100];
extern bool bUseUDP;
extern bool parsecmd(char**argv, int argc);

class cfcListener : public DDSDataReaderListener {
  public:
    virtual void on_requested_deadline_missed(
        DDSDataReader* /*reader*/,
        const DDS_RequestedDeadlineMissedStatus& /*status*/) {}
    
    virtual void on_requested_incompatible_qos(
        DDSDataReader* /*reader*/,
        const DDS_RequestedIncompatibleQosStatus& /*status*/) {}
    
    virtual void on_sample_rejected(
        DDSDataReader* /*reader*/,
        const DDS_SampleRejectedStatus& /*status*/) {}

    virtual void on_liveliness_changed(
        DDSDataReader* /*reader*/,
        const DDS_LivelinessChangedStatus& /*status*/) {}

    virtual void on_sample_lost(
        DDSDataReader* /*reader*/,
        const DDS_SampleLostStatus& /*status*/) {}

    virtual void on_subscription_matched(
        DDSDataReader* /*reader*/,
        const DDS_SubscriptionMatchedStatus& /*status*/) {}

    virtual void on_data_available(DDSDataReader* reader);
};

void cfcListener::on_data_available(DDSDataReader* reader)
{
    cfcDataReader *cfc_reader = NULL;
    cfcSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;

    cfc_reader = cfcDataReader::narrow(reader);
    if (cfc_reader == NULL) {
        printf("DataReader narrow error\n");
        return;
    }

    retcode = cfc_reader->take(
        data_seq, info_seq, DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);

    if (retcode == DDS_RETCODE_NO_DATA) {
        return;
    } else if (retcode != DDS_RETCODE_OK) {
        printf("take error %d\n", retcode);
        return;
    }

    for (i = 0; i < data_seq.length(); ++i) {
        if (info_seq[i].valid_data) {
            //// Start changes for Custom_Flowcontroller

            // print the time we get each sample.
            double elapsed_ticks = clock() - init;
            double elapsed_secs = elapsed_ticks/CLK_TCK;
            
            printf("@ t=%.2fs, got x = %d\n",
                   elapsed_secs, data_seq[i].x);

            //// End changes for Custom_Flowcontroller
        }
    }

    retcode = cfc_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        printf("return loan error %d\n", retcode);
    }
}

/* Delete all entities */
static int subscriber_shutdown(
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

    /* RTI Data Distribution Service provides finalize_instance() method for
       people who want to release memory used by the participant factory
       singleton. Uncomment the following block of code for clean destruction of
       the participant factory singleton. */
/*
    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }
*/
    return status;
}

extern "C" int subscriber_main(int domainId, int sample_count)
{
    DDSDomainParticipant *participant = NULL;
    DDSSubscriber *subscriber = NULL;
    DDSTopic *topic = NULL;
    cfcListener *reader_listener = NULL; 
    DDSDataReader *reader = NULL;
    DDS_ReturnCode_t retcode;
    const char *type_name = NULL;
    int count = 0;
    struct DDS_Duration_t receive_period = {1, 0};
    int status = 0;

    //// Start changes for Custom_Flowcontroller
    // for timekeeping
    init = clock();

    /* Get default participant QoS to customize */
    DDS_DomainParticipantQos participant_qos;
    retcode = DDSTheParticipantFactory->get_default_participant_qos(participant_qos);
    if (retcode != DDS_RETCODE_OK) {
        printf("get_default_participant_qos error\n");
        return -1;
    }

    // By default, discovery will communicate via shared memory for platforms
    // that support it.  Because we disable shared memory on the publishing
    // side, we do so here to ensure the reader and writer discover each other.
    participant_qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;

    /* To create participant with default QoS, use DDS_PARTICIPANT_QOS_DEFAULT
       instead of participant_qos */
    participant = DDSTheParticipantFactory->create_participant(
        domainId, participant_qos, 
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    //// End changes for Custom_Flowcontroller

    /* To customize subscriber QoS, use
       participant->get_default_subscriber_qos() */
    subscriber = participant->create_subscriber(
        DDS_SUBSCRIBER_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        printf("create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Register type before creating topic */
    type_name = cfcTypeSupport::get_type_name();
    retcode = cfcTypeSupport::register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use
       participant->get_default_topic_qos() */
    topic = participant->create_topic(
        "Example cfc",
        type_name, DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Create data reader listener */
    reader_listener = new cfcListener();
    if (reader_listener ==  NULL) {
        printf("listener instantiation error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* To customize data reader QoS, use
       subscriber->get_default_datareader_qos() */
    reader = subscriber->create_datareader(
        topic, DDS_DATAREADER_QOS_DEFAULT, reader_listener,
        DDS_STATUS_MASK_ALL);
    if (reader == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        delete reader_listener;
        return -1;
    }

    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {

//        printf("cfc subscriber sleeping for %d sec...\n",
//               receive_period.sec);

        NDDSUtility::sleep(receive_period);
    }

    /* Delete all entities */
    status = subscriber_shutdown(participant);
    delete reader_listener;

    return status;
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t** argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */ 
    
    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }
    
    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
                                  
    return subscriber_main(domainId, sample_count);
}

#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[])
{
    int sample_count = 0; /* infinite loop */

    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
                                  
    if (parsecmd(argv, argc))
      return subscriber_main(domain, sample_count);
    else
    {
        printf("Error parsing command line parameters.\n");
        return -1;
    }

    // Check for errant command situations.
    if (bitrate)
    {
      printf("Bitrate cannot be used in subscriber. Please do NOT use -b <bitrate> option.\n");
      return -1;
    }
}
#endif


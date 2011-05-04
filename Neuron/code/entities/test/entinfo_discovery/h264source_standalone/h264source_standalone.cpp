// H264 File Source Standalone Binary (not part of any session leader/factory, for testing only)
// Manjesh Malavalli
// XVDTH, USA

#include "testcommon.h"
#include "H264FileSrc.h"

using namespace std;

int main(int argc, char *argv[])
{
    int sourceId;
    DDS_ReturnCode_t retCode;

    if(argc==2)
    {
        DDSDomainParticipant *pMediaDP = DDSTheParticipantFactory->create_participant_with_profile(TEST_DOMAIN_ID,
                                         "NEURON","MEDIALAN",NULL,DDS_STATUS_MASK_NONE);

        if(pMediaDP!=NULL)
        {
            //Register type with domain participant
            const char *typeName = com::xvd::neuron::media::DataUnitTypeSupport::get_type_name();
            retCode = com::xvd::neuron::media::DataUnitTypeSupport::register_type(pMediaDP,typeName);
            if(retCode!=DDS_RETCODE_OK)
            {
                cerr << "Cannot register type" << endl;
                exit(0);
            }

            //Create topic for specified topic name
            DDSTopic *pTopic = pMediaDP->create_topic(TEST_VIDEO_TOPIC_NAME,typeName,DDS_TOPIC_QOS_DEFAULT,
                                                      NULL,DDS_STATUS_MASK_NONE);
            if(pTopic==NULL)
            {
                cerr << "Cannot create topic " << TEST_VIDEO_TOPIC_NAME << endl;
                exit(0);
            }

            //Register entity-info type with domain participant
            typeName = com::xvd::neuron::media::EntityInfoTypeSupport::get_type_name();
            retCode = com::xvd::neuron::media::EntityInfoTypeSupport::register_type(pMediaDP,typeName);
            if(retCode!=DDS_RETCODE_OK)
            {
                cerr << "Cannot register entity-info type" << endl;
                exit(0);
            }

            //Create topic for specified topic name
            DDSTopic *pEntInfoTopic = pMediaDP->create_topic(TEST_ENTINFO_TOPIC_NAME,typeName,DDS_TOPIC_QOS_DEFAULT,
                                                      NULL,DDS_STATUS_MASK_NONE);
            if(pEntInfoTopic==NULL)
            {
                cerr << "Cannot create topic " << TEST_ENTINFO_TOPIC_NAME << endl;
                exit(0);
            }

            sscanf(argv[1],"%d",&sourceId);
            H264FileSrc *pSrc = new H264FileSrc(sourceId,0,TEST_SESSION_ID,TEST_H264_FILE,pMediaDP,pTopic);
            pSrc->startThread();

            for(int i=0; i<1000; i++)
                usleep(50000);

            pSrc->stopThread();
            delete pSrc;
            retCode = pMediaDP->delete_topic(pTopic);
            if(retCode!=DDS_RETCODE_OK)
            {
                cerr << "Cannot delete topic " << TEST_VIDEO_TOPIC_NAME << endl;
                exit(0);
            }

            retCode = pMediaDP->delete_topic(pEntInfoTopic);
            if(retCode!=DDS_RETCODE_OK)
            {
                cerr << "Cannot delete topic " << TEST_ENTINFO_TOPIC_NAME << endl;
                exit(0);
            }
            
            DDSTheParticipantFactory->delete_participant(pMediaDP);
        }
        else
        {
            cerr << "Domain Participant creation failed..." << endl;
        }
    }
    else
    {
        cerr << "Usage: ./h264source_standalone <sourceId>" << endl;
    }

    return 0;
}


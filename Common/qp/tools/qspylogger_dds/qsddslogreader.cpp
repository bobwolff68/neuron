#include <iostream>
#include <unistd.h>
#include <errno.h>
#include "qsddslogreader.h"

using namespace std;

QSDDSLogReader* QSDDSLogReader::pTheInstance = NULL;

QSDDSLogReaderListener::QSDDSLogReaderListener(int32_t qspyPipeSinkP):
qspyPipeSink(qspyPipeSinkP)
{
}

QSDDSLogReaderListener::~QSDDSLogReaderListener()
{
}

void QSDDSLogReaderListener::on_data_available(DDSDataReader* pReader)
{
    com::xvd::neuron::qslogger::LogItemDataReader* pLogItemReader = NULL;
    com::xvd::neuron::qslogger::LogItemSeq seqLogItems;
    DDS_SampleInfoSeq seqSampleInfos;
    DDS_ReturnCode_t retcode;

    pLogItemReader = com::xvd::neuron::qslogger::LogItemDataReader::narrow(pReader);
    if(pLogItemReader==NULL)
    {
        cerr << "QSDDSLogReaderListener::on_data_available(): narrow() error..." << endl;
        return;
    }

    retcode = pLogItemReader->take(seqLogItems,seqSampleInfos,DDS_LENGTH_UNLIMITED,
                                   DDS_ANY_SAMPLE_STATE,DDS_ANY_VIEW_STATE,DDS_ANY_INSTANCE_STATE);
    if(retcode==DDS_RETCODE_NO_DATA)
        return;
    else if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogReaderListener::on_data_available(): Sample read error..." << endl;
        return;
    }
    else
    {
        for(int i=0; i<seqLogItems.length(); i++)
            if(seqSampleInfos[i].valid_data)
            {
                unsigned char* pSampleBuf = reinterpret_cast<unsigned char*>(seqLogItems[i].payload.
                                                                            get_contiguous_buffer());
                if(write(qspyPipeSink,pSampleBuf,seqLogItems[i].payload.length())<0)
                {
                    perror("QSDDSLogReaderListener::on_data_available():");
                    break;
                }
            }
            else
            {
                if(seqSampleInfos[i].instance_state==DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE ||
                   seqSampleInfos[i].instance_state==DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE)
                {
                    cout << endl << "Lost QS target, assuming that it is done..." << endl;
                    close(qspyPipeSink);
                }
            }
    }

    pLogItemReader->return_loan(seqLogItems,seqSampleInfos);
    return;
}

QSDDSLogReader::QSDDSLogReader(uint32_t domainIdP,const string& uniqueTopicNameP):
domainId(domainIdP),uniqueTopicName(uniqueTopicNameP)
{
    if(pipe(qspyPipe)<0)
        perror("QSDDSLogReader::QSDDSLogReader():");
}

QSDDSLogReader::~QSDDSLogReader()
{
    if(close(qspyPipe[0])<0)
        perror("QSDDSLogReader::~QSDDSLogReader(close(qspyPipe[0])):");
}

void QSDDSLogReader::CreateTheInstance(uint32_t domainIdP,const char* uniqueTopicNameP)
{
    string uniqueTopicName = uniqueTopicNameP;
    if(pTheInstance==(QSDDSLogReader*)0)
        pTheInstance = new QSDDSLogReader(domainIdP,uniqueTopicName);

    return;
}

QSDDSLogReader* QSDDSLogReader::TheInstance(void)
{
    return pTheInstance;
}

void QSDDSLogReader::DestroyTheInstance(void)
{
    if(pTheInstance)
    {
        delete pTheInstance;
        pTheInstance = (QSDDSLogReader*)0;
    }
}

uint8_t QSDDSLogReader::DDSStartup(void)
{
    DDS_ReturnCode_t retcode;

    //Create domain participant
    pParticipant = DDSDomainParticipantFactory::get_instance()->
                        create_participant(domainId,DDS_PARTICIPANT_QOS_DEFAULT,
                                           NULL,DDS_STATUS_MASK_NONE);
    if(!pParticipant)
    {
        cerr << "QSLogReader::DDSStartup(): Cannot create domain participant..." << endl;
        return (uint8_t)0;
    }

    //Register type
    retcode = com::xvd::neuron::qslogger::LogItemTypeSupport::register_type(pParticipant,
                          com::xvd::neuron::qslogger::LogItemTypeSupport::get_type_name());
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogReader::DDSStartup(): Cannot register LogItem type..." << endl;
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Add topic
    pTopic = pParticipant->create_topic(uniqueTopicName.c_str(),
                                        com::xvd::neuron::qslogger::LogItemTypeSupport::get_type_name(),
                                        DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    if(!pTopic)
    {
        cerr << "QSDDSLogReader::DDSStartup(): Cannot create topic '" << uniqueTopicName << "'..." << endl;
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Create subscriber
    pSubscriber = pParticipant->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    if(!pSubscriber)
    {
        cerr << "QSDDSLogReader::DDSStartup(): Cannot create subscriber..." << endl;
        pParticipant->delete_topic(pTopic);
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Set datareader qos to TRANSIENT_LOCAL_DURABILITY,RELIABLE_RELIABILITY
    DDS_DataReaderQos rQos;
    retcode = pSubscriber->get_default_datareader_qos(rQos);
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogReader::DDSStartup(): Cannot get default datareader qos..." << endl;
        pParticipant->delete_contained_entities();
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    rQos.history.depth = 20;
    rQos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    rQos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;

    //Create data reader
    QSDDSLogReaderListener* pListener = new QSDDSLogReaderListener(qspyPipe[1]);
    pReader = pSubscriber->create_datareader(pTopic,rQos,pListener,DDS_STATUS_MASK_ALL);
    if(!pReader)
    {
        cerr << "QSDDSLogReader::DDSStartup(): Cannot create data reader..." << endl;
        pParticipant->delete_contained_entities();
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    return (uint8_t)1;
}

void QSDDSLogReader::DDSTeardown(void)
{
    DDS_ReturnCode_t retcode;

    retcode = pParticipant->delete_contained_entities();
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogReader::DDSTeardown(): Cannot delete domain participant entities..." << endl;
        return;
    }

    retcode = DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogReader::DDSTeardown(): Cannot delete domain participant..." << endl;
        return;
    }
}

int32_t QSDDSLogReader::QspyPipeSource(void)
{
    return qspyPipe[0];
}

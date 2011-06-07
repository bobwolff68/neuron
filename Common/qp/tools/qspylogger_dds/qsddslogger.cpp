//!
//! \file qsddslogger.cpp
//!
//! \brief Function definitions of the QSDDSLogger class.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#include <iostream>
#include "qsddslogger.h"

using namespace std;

QSDDSLogger* QSDDSLogger::pTheInstance = NULL;

void QSDDSLogger::CreateTheInstance(uint32_t domainIdP,const string &uniqueTopicNameP)
{
    if(pTheInstance==(QSDDSLogger*)0)
        pTheInstance = new QSDDSLogger(domainIdP,uniqueTopicNameP);

    return;
}

QSDDSLogger* QSDDSLogger::TheInstance(void)
{
    return pTheInstance;
}

void QSDDSLogger::DestroyTheInstance(void)
{
    if(pTheInstance!=(QSDDSLogger*)0)
    {
        delete pTheInstance;
        pTheInstance = (QSDDSLogger*)0;
    }
}

uint8_t QSDDSLogger::DDSStartup(void)
{
    DDS_ReturnCode_t retcode;

    //Create domain participant
    pParticipant = DDSDomainParticipantFactory::get_instance()->
                        create_participant(domainId,DDS_PARTICIPANT_QOS_DEFAULT,
                                           NULL,DDS_STATUS_MASK_NONE);
    if(!pParticipant)
    {
        cerr << "QSDDSLogger::DDSStartup(): Cannot create domain participant..." << endl;
        return (uint8_t)0;
    }

    //Register type
    retcode = com::xvd::neuron::qslogger::LogItemTypeSupport::register_type(pParticipant,
                          com::xvd::neuron::qslogger::LogItemTypeSupport::get_type_name());
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogger::DDSStartup(): Cannot register LogItem type..." << endl;
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Add topic
    pTopic = pParticipant->create_topic(uniqueTopicName.c_str(),
                                        com::xvd::neuron::qslogger::LogItemTypeSupport::get_type_name(),
                                        DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    if(!pTopic)
    {
        cerr << "QSDDSLogger::DDSStartup(): Cannot create topic '" << uniqueTopicName << "'..." << endl;
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Create publisher
    pPublisher = pParticipant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    if(!pPublisher)
    {
        cerr << "QSDDSLogger::DDSStartup(): Cannot create publisher..." << endl;
        pParticipant->delete_topic(pTopic);
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    //Set datawriter qos to RELIABLE_RELIABILITY and TRANSIENT_LOCAL_DURABILITY
    DDS_DataWriterQos wQos;
    retcode = pPublisher->get_default_datawriter_qos(wQos);
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogger()::DDSStartup(): Cannot get default datawriter qos..." << endl;
        pParticipant->delete_contained_entities();
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    wQos.history.depth = 20;
    wQos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    wQos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;

    //Create datawriter
    pWriter = pPublisher->create_datawriter(pTopic,wQos,NULL,DDS_STATUS_MASK_NONE);
    if(!pWriter)
    {
        cerr << "QSDDSLogger::DDSStartup(): Cannot create data writer..." << endl;
        pParticipant->delete_contained_entities();
        DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
        return (uint8_t)0;
    }

    pLogItemInstance = com::xvd::neuron::qslogger::LogItemTypeSupport::create_data();
    pLogItemInstance->payload.maximum(0);
    return (uint8_t)1;
}

void QSDDSLogger::DDSTeardown(void)
{
    DDS_ReturnCode_t retcode;

    com::xvd::neuron::qslogger::LogItemTypeSupport::delete_data(pLogItemInstance);
    retcode = pParticipant->delete_contained_entities();
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogger::DDSTeardown(): Cannot delete domain participant entities..." << endl;
        return;
    }

    retcode = DDSDomainParticipantFactory::get_instance()->delete_participant(pParticipant);
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogger::DDSTeardown(): Cannot delete domain participant..." << endl;
        return;
    }
}

bool QSDDSLogger::Log(uint8_t* pLogItemBuf,const uint32_t bytes)
{
    DDS_ReturnCode_t retcode;

    if(done) return false;
    if(pLogItemBuf==(uint8_t*)0) return false;
    if(bytes>QS_BUF_LEN) return false;

    com::xvd::neuron::qslogger::LogItemDataWriter* pLogItemWriter =
            com::xvd::neuron::qslogger::LogItemDataWriter::narrow(pWriter);

    if(!pLogItemWriter)
    {
        cerr << "QSDDSLogger::Log(): pWriter not of type com::xvd::neuron::qslogger::LogItemDataWriter*..." << endl;
        return false;
    }

    if(!pLogItemInstance->payload.loan_contiguous(reinterpret_cast<DDS_Octet*>(pLogItemBuf),bytes,bytes+1))
    {
        cerr << "QSDDSLogger::Log(): log item buffer cannot be loaned to octet sequence..." << endl;
        return false;
    }

    retcode = pLogItemWriter->write(*pLogItemInstance,DDS_HANDLE_NIL);
    if(retcode!=DDS_RETCODE_OK)
    {
        cerr << "QSDDSLogger::Log(): Cannot write log item instance..." << endl;
        return false;
    }

    pLogItemInstance->payload.unloan();
    return true;
}

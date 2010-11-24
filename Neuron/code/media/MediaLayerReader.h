#ifndef MEDIALAYERREADER_H_
#define MEDIALAYERREADER_H_

#include <stdlib.h>
#include "media.h"
#include "mediaSupport.h"
#include "MediaEvent.h"

class DataSampleSet
{
    public:

    com::xvd::neuron::media::DataUnitDataReader *pReader;
    com::xvd::neuron::media::DataUnitSeq        *pSeqData;
    DDS_SampleInfoSeq                           *pSeqInfo;

    DataSampleSet(com::xvd::neuron::media::DataUnitDataReader   *pReaderP,
                  com::xvd::neuron::media::DataUnitSeq          *pSeqDataP,
                  DDS_SampleInfoSeq                             *pSeqInfoP)
    {
        pReader = pReaderP;
        pSeqData = pSeqDataP;
        pSeqInfo = pSeqInfoP;
    }

    ~DataSampleSet()
    {
        pReader->return_loan(*pSeqData,*pSeqInfo);
        delete pSeqData;
        delete pSeqInfo;
    }
};

class MediaLayerReader
{
    class DataUnitDataListener : public DDSDataReaderListener
    {
        private:

            EventHandler   *pOwnerEventHandler;

        public:

            DataUnitDataListener(EventHandler *pOwnerEventHandlerP):
            DDSDataReaderListener()
            {
                pOwnerEventHandler = pOwnerEventHandlerP;
            }

            ~DataUnitDataListener()
            {
            }

            void on_data_available(DDSDataReader *pGenericReader)
            {
                DDS_ReturnCode_t                                retCode;
                DDS_SampleInfoSeq                              *pSeqInfo = NULL;
                com::xvd::neuron::media::DataUnitSeq           *pSeqData = NULL;
                com::xvd::neuron::media::DataUnitDataReader    *pReader = NULL;

                pSeqInfo = new DDS_SampleInfoSeq(0);
                pSeqData = new com::xvd::neuron::media::DataUnitSeq(0);

                //Obtain the specific type reader from generic reader
                pReader = com::xvd::neuron::media::DataUnitDataReader::narrow(pGenericReader);

                //Read data and push to event queue
                retCode = pReader->take(*pSeqData,*pSeqInfo,DDS_LENGTH_UNLIMITED,
                                        DDS_ANY_SAMPLE_STATE,DDS_ANY_VIEW_STATE,
                                        DDS_ANY_INSTANCE_STATE);
                if(retCode!=DDS_RETCODE_NO_DATA)
                {
                    if(retCode!=DDS_RETCODE_OK)
                    {
                        std::cout << "Cannot read data from data reader queue" << std::endl;
                        return;
                    }
                    else
                    {
                        MediaInputEvent<DataSampleSet*>  *pEvent = NULL;
                        DataSampleSet                    *pSampleSet = NULL;

                        pSampleSet = new DataSampleSet(pReader,pSeqData,pSeqInfo);
                        pEvent = new MediaInputEvent<DataSampleSet*>(pSampleSet);
                        pOwnerEventHandler->SignalEvent(pEvent);

                        //Memory loaned for seqInfo and seqData is to be returned after output object
                        //process the sequences
                    }
                }

                return;
            }
    };

    private:

        EventHandler                        *pOwnerEventHandler;
        DDSDomainParticipant                *pOwnerDP;
        DDSTopic                            *pTopic;
        DDSSubscriber                       *pSub;
        DDSDataReader                       *pGenericReader;
        DataUnitDataListener                *pListener;

    public:

        MediaLayerReader(EventHandler         *pOwnerEventHandlerP,
                         DDSDomainParticipant *pOwnerDPP,
                         DDSTopic             *pTopicP,
                         const char           *layerPartitionRegExp)
        {
            DDS_ReturnCode_t                                retCode;
            DDS_SubscriberQos                               subQos;
            com::xvd::neuron::media::DataUnitDataReader    *pReader;

            pOwnerEventHandler = pOwnerEventHandlerP;

            //Use parent entity's domain participant to create subscriber
            //Set the subscriber's partition to layerPartitionRegExp
            pOwnerDP = pOwnerDPP;
            pTopic = pTopicP;
            retCode = pOwnerDP->get_default_subscriber_qos(subQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot get default subscriber qos" << std::endl;
                exit(0);
            }
            subQos.partition.name.ensure_length(1,1);
            subQos.partition.name[0] = DDS_String_dup(layerPartitionRegExp);
            retCode = pOwnerDP->set_default_subscriber_qos(subQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot set default subscriber qos" << std::endl;
                exit(0);
            }
            pSub = pOwnerDP->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,
                                                NULL,DDS_STATUS_MASK_NONE);
            if(pSub==NULL)
            {
                std::cout << "Cannot create subscriber" << std::endl;
                exit(0);
            }

            //Use parent entity's topic to create data reader
            pGenericReader = pSub->create_datareader(pTopic,DDS_DATAREADER_QOS_USE_TOPIC_QOS,
												     NULL,DDS_STATUS_MASK_NONE);
            if(pGenericReader==NULL)
            {
                std::cout << "Cannot create data reader" << std::endl;
                exit(0);
            }

            //Set data reader listener
            pListener = new DataUnitDataListener(pOwnerEventHandler);
            pReader = com::xvd::neuron::media::DataUnitDataReader::narrow(pGenericReader);
            pReader->set_listener(pListener,DDS_STATUS_MASK_ALL);
        }

        ~MediaLayerReader()
        {
            DDS_ReturnCode_t    retCode;

            delete pListener;
            retCode = pSub->delete_contained_entities();
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete subscriber entities" << std::endl;
                exit(0);
            }

            retCode = pOwnerDP->delete_subscriber(pSub);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete subscriber" << std::endl;
                exit(0);
            }
        }

        bool SetLayerPartitionRegExp(const char *regExp)
        {
            DDS_SubscriberQos   subQos;
            DDS_ReturnCode_t    retCode;

            retCode = pSub->get_qos(subQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot get subscriber qos" << std::endl;
                return false;
            }

            subQos.partition.name.ensure_length(1,1);
            subQos.partition.name[0] = DDS_String_dup(regExp);

            retCode = pSub->set_qos(subQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot set subscriber qos" << std::endl;
                return false;
            }

            return true;
        }
};

#endif // MEDIALAYERREADER_H_

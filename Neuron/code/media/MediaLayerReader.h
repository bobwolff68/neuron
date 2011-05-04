//!
//! \file MediaLayerReader.h
//!
//! \brief Object to read samples of multiple media layers from DDS.
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef MEDIALAYERREADER_H_
#define MEDIALAYERREADER_H_

#include <stdlib.h>
#include "media.h"
#include "mediaSupport.h"
#include "MediaEvent.h"

class DataSampleSet
{
    public:

    int nSamplesToBeWritten;
    
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
        
        nSamplesToBeWritten = pSeqData->length();
        for(int i=0; i<pSeqData->length(); i++)
            if(!(*pSeqInfo)[i].valid_data)
                nSamplesToBeWritten--;
    }

    ~DataSampleSet()
    {
        pReader->return_loan(*pSeqData,*pSeqInfo);
        delete pSeqData;
        delete pSeqInfo;
    }
};

//!
//! \class MediaLayerReader
//!
//! \brief Reads samples from one or more media layers for processing by the owner session entity.
//!
//! Details: This is basically a subscriber-datareader pair that subscribes to data from
//! one or more partitions, each representing a media layer. Partitions are used
//! to differentiate among different media layers.
//!
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

        //!
        //! \var pOwnerEventHandler
        //!
        //! \brief Pointer to the owner session entity's event handler.
        //!
        EventHandler *pOwnerEventHandler;

        //!
        //! \var pOwnerDP
        //!
        //! \brief Pointer to the domain participant to which the owner session entity belongs.
        //!
        //! \a NOTE: Memory is not freed in destructor, because it is owned by the session
        //!    leader to which the owner session entity belongs.
        //!
        DDSDomainParticipant *pOwnerDP;

        //!
        //! \var pTopic
        //!
        //! \brief Pointer to the DDS topic whose samples are to be read by the object.
        //!
        DDSTopic *pTopic;

        //!
        //! \var pSub
        //!
        //! \brief Pointer to the subscriber which is configured to subscribe to one or more layer partitions.
        //!
        DDSSubscriber *pSub;

        //!
        //! \var pGenericReader
        //!
        //! \brief Pointer to the generic data reader created to read media data.
        //!
        DDSDataReader *pGenericReader;

        //!
        //! \var pListener
        //!
        //! \brief Pointer to a listener that attaches to the data reader to listen for media data.
        //!
        DataUnitDataListener *pListener;

    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] pOwnerEventHandlerP - Pointer to the owner session entity's event handler.
        //! \param [in] pOwnerDPP - Pointer to the domain participant to which the owner session entity belongs.
        //! \param [in] pTopicP - Pointer to the DDS topic whose samples are to be read by the object.
        //! \param [in] layerPartitionRegExp - Indicates the media layer(s) to which the object subscribes.
        //!
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
            pGenericReader = pSub->create_datareader(pTopic,DDS_DATAREADER_QOS_DEFAULT,
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

        //!
        //! \brief Destructor
        //!
        ~MediaLayerReader()
        {
            DDS_ReturnCode_t    retCode;

            retCode = pSub->delete_contained_entities();
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete subscriber entities" << std::endl;
                exit(0);
            }

            delete pListener;
            retCode = pOwnerDP->delete_subscriber(pSub);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete subscriber" << std::endl;
                exit(0);
            }
        }

        //!
        //! \brief Function to change the media layer(s) to which the object subscribes.
        //!
        //! \param [in] regExp - Indicates the new media layer(s) to which the object subscribes.
        //!
        //! \return true if successful, false if not.
        //!
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

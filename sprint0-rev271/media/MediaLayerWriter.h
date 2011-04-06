//!
//! \file MediaLayerWriter.h
//!
//! \brief Object to transmit a media layer over DDS
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef MEDIALAYERWRITER_H_
#define MEDIALAYERWRITER_H_

#include <stdlib.h>
#include "media.h"
#include "mediaSupport.h"

//!
//! \class MediaLayerWriter
//!
//! \brief Writes a media layer over DDS
//!
//! Details: This is basically a publisher-datawriter pair that
//! outputs data over DDS on a single partition. Partitions are used
//! to differentiate among different media layers.
//!
class MediaLayerWriter
{
    private:

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
        //! \brief Pointer to the DDS topic whose samples are to be written by the object.
        //!
        DDSTopic *pTopic;

        //!
        //! \var pPub
        //!
        //! \brief Pointer to the publisher which is configured to publish in a particular partition.
        //!
        DDSPublisher *pPub;

        //!
        //! \var pGenericWriter
        //!
        //! \brief Pointer to the generic data writer created to write media data.
        //!
        DDSDataWriter *pGenericWriter;

        //!
        //! \var pDataUnitSample
        //!
        //! \brief For internal use only.
        //!
        com::xvd::neuron::media::DataUnit *pDataUnitSample;

    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] pOwnerDPP - Pointer to the domain participant to which the owner session entity belongs.
        //! \param [in] pTopicP - Pointer to the DDS topic whose samples are to be written by the object.
        //! \param [in] layerPartitionName - Partition on which a particular media layer is published.
        //!
        MediaLayerWriter(DDSDomainParticipant *pOwnerDPP,
                         DDSTopic             *pTopicP,
                         const char           *layerPartitionName)
        {
            DDS_ReturnCode_t    retCode;
            DDS_PublisherQos    pubQos;

            //Use parent entity's domain participant to create publisher
            //Set the publisher's partition to layerPartitionName
            pOwnerDP = pOwnerDPP;
            pTopic = pTopicP;
            retCode = pOwnerDP->get_default_publisher_qos(pubQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot get default publisher QOS" << std::endl;
                exit(0);
            }
            pubQos.partition.name.ensure_length(1,1);
            pubQos.partition.name[0] = DDS_String_dup(layerPartitionName);
            retCode = pOwnerDP->set_default_publisher_qos(pubQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot set default publisher QOS" << std::endl;
                exit(0);
            }
            pPub = pOwnerDP->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,
                                               NULL,DDS_STATUS_MASK_NONE);
            if(pPub==NULL)
            {
                std::cout << "Cannot create publisher" << std::endl;
                exit(0);
            }

            //Use parent entity's topic to create data writer
            pGenericWriter = pPub->create_datawriter(pTopic,DDS_DATAWRITER_QOS_DEFAULT,
												     NULL,DDS_STATUS_MASK_NONE);
            if(pGenericWriter==NULL)
            {
                std::cout << "Cannot create data writer" << std::endl;
                exit(0);
            }
            //Ignore local data writer at domian participant
            pOwnerDP->ignore_publication(pGenericWriter->get_instance_handle());

            pDataUnitSample = com::xvd::neuron::media::DataUnitTypeSupport::create_data();
            pDataUnitSample->payload.maximum(0);
        }

        //!
        //! \brief Destructor.
        //!
        ~MediaLayerWriter()
        {
            DDS_ReturnCode_t    retCode;

            com::xvd::neuron::media::DataUnitTypeSupport::delete_data(pDataUnitSample);
            retCode = pPub->delete_contained_entities();
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete publisher entities" << std::endl;
                exit(0);
            }

            retCode = pOwnerDP->delete_publisher(pPub);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete publisher" << std::endl;
                exit(0);
            }
        }

        //!
        //! \brief Encapsulates media layer data in a DataUnit sample and writes it over DDS.
        //!
        //! \param [in] seqNum - Sequence number of the sample to be written.
        //! \param [in] payloadBuf - Pointer to the payload to be written.
        //! \param [in] payloadSize - Size of the payload.
        //!
        //! \return true if write successful, false if not.
        //!
        bool Write(long seqNum,unsigned char *payloadBuf,int payloadSize)
        {
            if(payloadBuf!=NULL)
            {
                pDataUnitSample->seqNum = seqNum;

                // Loan memory associated with the payload to be written to the Data Unit sample
                // If the load and write are successful, return the memory loan
                if(pDataUnitSample->payload.loan_contiguous(reinterpret_cast<DDS_Octet*>(payloadBuf),payloadSize,payloadSize+1))
                {
                    com::xvd::neuron::media::DataUnitDataWriter    *pWriter = NULL;
                    DDS_ReturnCode_t                                retCode;

                    pWriter = com::xvd::neuron::media::DataUnitDataWriter::narrow(pGenericWriter);
                    retCode = pWriter->write(*pDataUnitSample,DDS_HANDLE_NIL);
                    if(retCode!=DDS_RETCODE_OK)
                    {
                        std::cout << "Cannot write data" << std::endl;
                        return false;
                    }

                    pDataUnitSample->payload.unloan();
                }
                else
                {
                    std::cout << "Cannot loan payload memory to octet sequence" << std::endl;
                    return false;
                }
            }
            else
            {
                std::cout << "Payload buffer is null" << std::endl;
                return false;
            }

            return true;
        }

        //!
        //! \brief Writes a DataUnit sample over DDS.
        //!
        //! \param [in] datUnitSample - A sample of DataUnit type that is to be written.
        //!
        //! \return true of write successful, false if not.
        //!
        bool Write(com::xvd::neuron::media::DataUnit &dataUnitSample)
        {
            com::xvd::neuron::media::DataUnitDataWriter    *pWriter = NULL;
            DDS_ReturnCode_t                                retCode;

            pWriter = com::xvd::neuron::media::DataUnitDataWriter::narrow(pGenericWriter);
            retCode = pWriter->write(dataUnitSample,DDS_HANDLE_NIL);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot write data" << std::endl;
                return false;
            }

            return true;
        }

};

#endif // MEDIALAYERWRITER_H_

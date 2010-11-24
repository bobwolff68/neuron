#ifndef MEDIALAYERWRITER_H_
#define MEDIALAYERWRITER_H_

#include <stdlib.h>
#include "media.h"
#include "mediaSupport.h"

class MediaLayerWriter
{
    private:

        DDSDomainParticipant                *pOwnerDP;
        DDSTopic                            *pTopic;
        DDSPublisher                        *pPub;
        DDSDataWriter                       *pGenericWriter;
        com::xvd::neuron::media::DataUnit   *pDataUnitSample;

    public:

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
            pGenericWriter = pPub->create_datawriter(pTopic,DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
												     NULL,DDS_STATUS_MASK_NONE);
            if(pGenericWriter==NULL)
            {
                std::cout << "Cannot create data writer" << std::endl;
                exit(0);
            }

            pDataUnitSample = com::xvd::neuron::media::DataUnitTypeSupport::create_data();
            pDataUnitSample->payload.maximum(0);
        }

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

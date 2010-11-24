#ifndef DDSOUTPUTOBJECT_H_
#define DDSOUTPUTOBJECT_H_

#include "MediaOutputObject.h"
#include "MediaWriter.h"

class DDSOutputObject : public MediaOutputObject
{
    private:

        MediaWriter    *pMediaWriter;

    public:

        DDSOutputObject(int                     ownerIdP,
                        DDSDomainParticipant   *pOwnerDPP,
                        const char             *topicName):
        MediaOutputObject(ownerIdP)
        {
            pMediaWriter = new MediaWriter(pOwnerDPP,topicName);
        }

        ~DDSOutputObject()
        {
            delete pMediaWriter;
        }

        bool AddLayerWriter(const char *layerPartitionName)
        {
            return pMediaWriter->AddLayerWriter(layerPartitionName);
        }

        void Write(void)
        {
        }

        bool Write(const char *layerPartitionName,long seqNum,
                   unsigned char *payloadBuf,int payloadSize)
        {
            return pMediaWriter->Write(layerPartitionName,seqNum,
                                       payloadBuf,payloadSize);
        }

        bool Write(const char *layerPartitionName,
                   com::xvd::neuron::media::DataUnit &dataUnitSample)
        {
            return pMediaWriter->Write(layerPartitionName,dataUnitSample);
        }
};

#endif // DDSOUTPUTOBJECT_H_

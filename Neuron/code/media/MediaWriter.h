#ifndef MEDIAWRITER_H_
#define MEDIAWRITER_H_

#include <map>
#include <string>
#include "MediaLayerWriter.h"

class MediaWriter
{
    private:

        std::map<std::string,MediaLayerWriter*> LayerWriters;
        DDSTopic                               *pTopic;
        DDSDomainParticipant                   *pOwnerDP;

    public:

        MediaWriter(DDSDomainParticipant    *pOwnerDPP,
                    DDSTopic                *pTopicP)
        {
            pOwnerDP = pOwnerDPP;
            pTopic = pTopicP;
        }

        ~MediaWriter()
        {
            std::map<std::string,MediaLayerWriter*>::iterator    it;
            DDS_ReturnCode_t                           retCode;

            //Delete all the layer writers
            for(it=LayerWriters.begin(); it!=LayerWriters.end(); it++)
                delete it->second;
            LayerWriters.clear();
        }

        bool AddLayerWriter(const char *layerPartitionName)
        {
            std::string layerPartition(layerPartitionName);
            if(LayerWriters.find(layerPartition)==LayerWriters.end())
            {
                LayerWriters[layerPartition] = new MediaLayerWriter(pOwnerDP,pTopic,
                                                                    layerPartitionName);
            }
            else
            {
                std::cout << "Cannot add layer writer (already present)" << std::endl;
                return false;
            }

            return true;
        }

        bool Write(const char *layerPartitionName,long seqNum,
                   unsigned char *payloadBuf,int payloadSize)
        {
            std::string layerPartition(layerPartitionName);

            if(LayerWriters.find(layerPartition)!=LayerWriters.end())
            {
#ifdef VERBOSE_OUTPUT
                std::cout << "Writing on layer " << layerPartition << std::endl;
#endif
                return LayerWriters[layerPartition]->Write(seqNum,payloadBuf,payloadSize);
            }
            else
            {
                std::cout << "Cannot write to layer (writer not present)" << std::endl;
                return false;
            }

            return true;
        }

        bool Write(const char *layerPartitionName,com::xvd::neuron::media::DataUnit &dataUnitSample)
        {
            std::string layerPartition(layerPartitionName);

            if(LayerWriters.find(layerPartition)!=LayerWriters.end())
            {
#ifdef VERBOSE_OUTPUT
                std::cout << "Writing on layer " << layerPartition << std::endl;
#endif
                return LayerWriters[layerPartition]->Write(dataUnitSample);
            }
            else
            {
                std::cout << "Cannot write to layer (writer not present)" << std::endl;
                return false;
            }

            return true;
        }
};

#endif // MEDIAWRITER_H_

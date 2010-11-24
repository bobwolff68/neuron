#ifndef MEDIAREADER_H_
#define MEDIAREADER_H_

#include <map>
#include <string>
#include "MediaLayerReader.h"

class MediaReader
{
    private:

        DDSDomainParticipant                    *pOwnerDP;
        DDSTopic                                *pTopic;
        EventHandler                            *pOwnerEventHandler;
        std::map<std::string,MediaLayerReader*>  LayerReaders;

    public:

        MediaReader(EventHandler            *pOwnerEventHandlerP,
                    DDSDomainParticipant    *pOwnerDPP,
                    const char              *topicName)
        {
            const char         *typeName = NULL;
            DDS_ReturnCode_t    retCode;

            pOwnerDP = pOwnerDPP;
            pOwnerEventHandler = pOwnerEventHandlerP;

            //Register type with domain participant
            typeName = com::xvd::neuron::media::DataUnitTypeSupport::get_type_name();
            retCode = com::xvd::neuron::media::DataUnitTypeSupport::register_type(pOwnerDP,typeName);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot register type" << std::endl;
                exit(0);
            }

            //Create topic for specified topic name
            pTopic = pOwnerDP->create_topic(topicName,typeName,DDS_TOPIC_QOS_DEFAULT,
                                            NULL,DDS_STATUS_MASK_NONE);
            if(pTopic==NULL)
            {
                std::cout << "Cannot create topic" << std::endl;
                exit(0);
            }
        }

        ~MediaReader()
        {
            std::map<std::string,MediaLayerReader*>::iterator    it;
            DDS_ReturnCode_t                                     retCode;

            //Delete all the layer readers
            for(it=LayerReaders.begin(); it!=LayerReaders.end(); it++)
                delete it->second;
            LayerReaders.clear();

            //Delete the topic
            retCode = pOwnerDP->delete_topic(pTopic);
            if(retCode!=DDS_RETCODE_OK)
            {
                std::cout << "Cannot delete topic" << std::endl;
                exit(0);
            }
        }

        bool AddLayerReader(const char *layerPartitionRegExp)
        {
            std::string layerPartition(layerPartitionRegExp);

            if(LayerReaders.find(layerPartition)==LayerReaders.end())
            {
                LayerReaders[layerPartition] = new MediaLayerReader(pOwnerEventHandler,pOwnerDP,
                                                                    pTopic,layerPartitionRegExp);
            }
            else
            {
                std::cout << "Cannot add layer reader (already present)" << std::endl;
                return false;
            }

            return true;
        }

        bool SetLayerReaderPartition(const char *curRegExp,const char *newRegExp)
        {
            std::string curLayerRegExp(curRegExp);
            std::string newLayerRegExp(newRegExp);

            if(LayerReaders.find(curLayerRegExp)!=LayerReaders.end())
            {
                if(LayerReaders.find(newLayerRegExp)==LayerReaders.end())
                {
                    MediaLayerReader   *pLayerReader = NULL;

                    pLayerReader = LayerReaders[curLayerRegExp];
                    LayerReaders.erase(curLayerRegExp);
                    LayerReaders[newLayerRegExp] = pLayerReader;

                    return pLayerReader->SetLayerPartitionRegExp(newRegExp);
                }
                else
                {
                    std::cout << "Cannot add layer reader (already present)" << std::endl;
                    return false;
                }
            }
            else
            {
                std::cout << "Error, trying to change non-existent reader's partition" << std::endl;
                return false;
            }

            return true;
        }
};

#endif // MEDIAREADER_H_

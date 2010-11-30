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
                    DDSTopic                *pTopicP)
        {
            pOwnerDP = pOwnerDPP;
            pOwnerEventHandler = pOwnerEventHandlerP;
            pTopic = pTopicP;
        }

        ~MediaReader()
        {
            std::map<std::string,MediaLayerReader*>::iterator    it;
            DDS_ReturnCode_t                                     retCode;

            //Delete all the layer readers
            for(it=LayerReaders.begin(); it!=LayerReaders.end(); it++)
                delete it->second;
            LayerReaders.clear();
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

#ifndef DDSINPUTOBJECT_H_
#define DDSINPUTOBJECT_H_

#include "MediaInputObject.h"
#include "MediaReader.h"

class DDSInputObject : public MediaInputObject
{
    private:

        MediaReader    *pMediaReader;

    public:

        DDSInputObject(int                      ownerIdP,
                       EventHandler            *pOwnerEventHandlerP,
                       DDSDomainParticipant    *pOwnerDPP,
                       DDSTopic                *pTopic):
        MediaInputObject(pOwnerEventHandler,ownerIdP)
        {
            pMediaReader = new MediaReader(pOwnerEventHandlerP,pOwnerDPP,pTopic);
        }

        ~DDSInputObject()
        {
            delete pMediaReader;
        }

        void StreamMedia(void)
        {
            //Listener thread streams media, so explicit function not required
        }

        bool AddLayerReader(const char *layerPartitionRegExp)
        {
            return pMediaReader->AddLayerReader(layerPartitionRegExp);
        }

        bool SetLayerReaderPartition(const char *curRegExp,const char *newRegExp)
        {
            return pMediaReader->SetLayerReaderPartition(curRegExp,newRegExp);
        }
};

#endif // DDSINPUTOBJECT_H_

#ifndef MEDIAINPUTOBJECT_H_
#define MEDIAINPUTOBJECT_H_

#include "MediaObject.h"
#include "eventhandler.h"

#define MEDIA_INPUT_EVENT   1001

class MediaInputObject : public MediaObject
{
    protected:

        EventHandler   *pOwnerEventHandler;

    public:

        MediaInputObject(EventHandler *pOwnerEventHandlerP,int ownerIdP):MediaObject(ownerIdP)
        {
            pOwnerEventHandler = pOwnerEventHandlerP;
        }

        ~MediaInputObject()
        { }

        virtual void StreamMedia(void) = 0;
};

template<typename MediaType> class MediaInputEvent : public Event
{
    private:

        MediaType   data;

    public:

        MediaInputEvent(MediaType dataP):Event(MEDIA_INPUT_EVENT)
        {
            data = dataP;
        }

        ~MediaInputEvent()
        { }

        MediaType GetData(void)
        {
            return data;
        }
};

#endif // MEDIAINPUTOBJECT_H_

#ifndef MEDIAINPUTOBJECT_H_
#define MEDIAINPUTOBJECT_H_

#include "MediaObject.h"
#include "MediaEvent.h"

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

#endif // MEDIAINPUTOBJECT_H_

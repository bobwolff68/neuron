#ifndef MEDIAOUTPUTOBJECT_H_
#define MEDIAOUTPUTOBJECT_H_

#include "MediaObject.h"

class MediaOutputObject : public MediaObject
{
    public:

        MediaOutputObject(int ownerIdP):MediaObject(ownerIdP)
        { }

        ~MediaOutputObject()
        { }

        virtual void Write(void) = 0;
};

#endif // MEDIAOUTPUTOBJECT_H_

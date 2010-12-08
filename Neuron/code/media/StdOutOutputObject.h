#ifndef STDOUTOUTPUTOBJECT_H_
#define STDOUTOUTPUTOBJECT_H_

#include <iostream>
#include "MediaOutputObject.h"

class StdOutOutputObject : public MediaOutputObject
{
    public:

        StdOutOutputObject(int ownerIdP):MediaOutputObject(ownerIdP)
        { }

        ~StdOutOutputObject()
        { }

        void Write(void)
        { }

        void Write(int data)
        {
            std::cout << MOO_LOG_PROMPT(ownerId) << ": Data=" << data << std::endl;

            return;
        }
};

#endif // STDOUTOUTPUTOBJECT_H_

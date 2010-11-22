#ifndef STDOUTOUTPUTOBJECT_H_
#define STDOUTOUTPUTOBJECT_H_

#include <iostream>
#include "MediaOutputObject.h"

#define MOO_LOG_PROMPT(ownerId) "MOO(" << ownerId << ")"

class StdOutOutputObject : public MediaOutputObject
{
    public:

        StdOutOutputObject(int ownerIdP):MediaOutputObject(ownerIdP)
        { }

        ~StdOutOutputObject()
        { }

        void Write(void *data)
        {
            std::cout << MOO_LOG_PROMPT(ownerId) << ": Data=" << *((int *)data) << endl;

            return;
        }
};

#endif // STDOUTOUTPUTOBJECT_H_

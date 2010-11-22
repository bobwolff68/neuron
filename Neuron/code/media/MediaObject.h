#ifndef MEDIAOBJECT_H_
#define MEDIAOBJECT_H_

class MediaObject
{
    protected:

        int ownerId;

    public:

        MediaObject(int ownerIdP):ownerId(ownerIdP)
        { }

        ~MediaObject()
        { }
};

#endif // MEDIAOBJECT_H_

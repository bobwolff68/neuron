//!
//! \file MediaObject.h
//!
//! \brief Definition of Media Object.
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef MEDIAOBJECT_H_
#define MEDIAOBJECT_H_

//!
//! \class MediaObject
//!
//! \brief Defines a genric media input/output object.
//!
class MediaObject
{
    protected:

        //!
        //! \var ownerId
        //!
        //! \brief Id of the owner session entity.
        //!
        int ownerId;

    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] ownerIdP - Id of the owner session entity.
        //!
        MediaObject(int ownerIdP):ownerId(ownerIdP)
        { }

        //!
        //! \brief Destructor.
        //!
        ~MediaObject()
        { }
};

#endif // MEDIAOBJECT_H_

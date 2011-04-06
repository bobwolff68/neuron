//!
//! \file MediaOutputObject.h
//!
//! \brief Definition of the Media Output Object
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef MEDIAOUTPUTOBJECT_H_
#define MEDIAOUTPUTOBJECT_H_

#include "MediaObject.h"

#define MOO_LOG_PROMPT(ownerId) "MOO(" << ownerId << ")"

//!
//! \class MediaOutputObject
//!
//! \brief Generic media output object.
//!
//! Details: This is an abstract base class that represents the generic
//! structure of an interface between its owner session entity and a media sink.
//!
class MediaOutputObject : public MediaObject
{
    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] ownerIdP - Id of the owner session entity.
        //!
        MediaOutputObject(int ownerIdP):MediaObject(ownerIdP)
        { }

        //!
        //! \brief Destructor.
        //!
        ~MediaOutputObject()
        { }

        //!
        //! \brief Pure virtual function to write media to sink.
        //!
        //! Details: A placeholder pure virtual function that is implemented
        //! as an empty method in the derived class, because input arguments
        //! and return values may vary with the type of media being processed.
        //! Although not necessary to have this function definition, it is
        //! neverthless present to prevent an instantiation of this class.
        //!
        virtual void Write(void) = 0;
};

#endif // MEDIAOUTPUTOBJECT_H_

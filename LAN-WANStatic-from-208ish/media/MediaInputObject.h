//!
//! \file MediaInputObject.h
//!
//! \brief Definition of the Media Input Object
//!
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef MEDIAINPUTOBJECT_H_
#define MEDIAINPUTOBJECT_H_

#include "MediaObject.h"
#include "MediaEvent.h"

#define MIO_LOG_PROMPT(ownerId) "MIO(" << ownerId << ")"

//!
//! \class Media Input Object
//!
//! \brief Generic media input object.
//!
//! Details: This is an abstract base class that represents the generic
//! structure of an interface between its owner session entity and a media source.
//! It communicates with the owner session entity through its event handler by
//! posting data within a MediaInputEvent object and pushing it into the event queue.
//!
class MediaInputObject : public MediaObject
{
    protected:

        //!
        //! \var pOwnerEventHandler
        //!
        //! \brief Pointer to the owner session entity's event handler.
        //!
        EventHandler   *pOwnerEventHandler;

    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] pOwnerEventHandlerP - Pointer to the owner session entity's event handler.
        //! \param [in] ownerIdP - Id of the owner session entity.
        //!
        MediaInputObject(EventHandler *pOwnerEventHandlerP,int ownerIdP):MediaObject(ownerIdP)
        {
            pOwnerEventHandler = pOwnerEventHandlerP;
        }

        //!
        //! \brief Destructor.
        //!
        ~MediaInputObject()
        { }

        //!
        //! \brief Pure virtual function for streaming from a media source.
        //!
        //! Details: This is a pure virtual function that is implemented by the input
        //! object class that derives this class for a particular type of
        //! media (file,stdin,dds,h264 encoder etc.,). This function is to be called in
        //! a different thread so that the main thread can process the media that is
        //! extracted from the source by this function. The exception to this rule is
        //! in the case of DDS where the listener thread performs the task of streaming
        //! media.
        //!
        virtual void StreamMedia(void) = 0;
};

#endif // MEDIAINPUTOBJECT_H_

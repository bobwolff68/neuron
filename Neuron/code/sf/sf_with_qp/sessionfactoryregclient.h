//!
//! \file sessionfactoryregclient.h
//!
//! \brief Asynchronous curl registration client for the Session Factory class.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//! \date 05/24/2011
//!

#include "registration.h"

#ifndef UNIT_TEST_MODE
#include "sfevents.h"
#endif

//!
//! \class SFRegClient
//!
//! \brief Asynchronous curl registration client for the Session Factory class.
//!
class SFRegClient : public RegistrationClientAsync
{
    public:

        //!
        //! \brief Constructor.
        //!
        //! \param [in] pIp_address - IP address of remote location of the registration server.
        //! \param [in] sfid - Id of the session factory instance requesting registration.
        //! \param [in] portnum - Port listened to by the registration server.
        //! \param [in] bIsEndpoint - Boolean specifying whether session factory instance is part of an endpoint.
        //! \param [in] friendlyname - Name of the session factory instance requesting registration.
        //!
        SFRegClient(const char*    pIp_address,
                    uint32_t       sfid,
                    uint32_t       portnum=8181,
                    bool           bIsEndpoint=false,
                    const char*    friendlyname=NULL)
        : RegistrationClientAsync(pIp_address,sfid,portnum,bIsEndpoint,friendlyname)
        { };

        //!
        //! \brief Destructor.
        //!
        virtual ~SFRegClient() { };

        //!
        //! brief Callback for passing the registration server response to the sf registration state machine.
        //!
        //! \param[in] bSuccess - TRUE if registration successful, false if not.
        //!
        //! \return VOID.
        //!
        void ResponseReceived(bool bSuccess)
        {
#ifndef UNIT_TEST_MODE
            RegResponseEvent* pRespEvt = Q_NEW(RegResponseEvent,REG_RESPONSE_RECVD_SIG);
            pRespEvt->bRegSuccess = bSuccess;
            strcpy(pRespEvt->response,response);
            QF::publish(SF_EVENT(pRespEvt));
#else
            cout << "SFRegClient response received..." << endl;
#endif
            return;
        }
        
        //!
        //! \brief Callback for signaling the parent that the abort process is completed.
        //!
        //! \return VOID.
        //!
        void AbortCallback(void)
        {
#ifndef UNIT_TEST_MODE
            QF::publish(Q_NEW(QEvent,REG_ABORT_DONE_SIG));
#else
            cout << "Registration abort completed..." << endl;
#endif
            return;            
        }
};

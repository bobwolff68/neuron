//!
//! \file sessionfactory.h
//!
//! \brief Declaration of the SessionFactory class.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef SESSIONFACTORY_H_
#define SESSIONFACTORY_H_

#include <stdint.h>
#include "sessionfactoryregclient.h"

#ifndef DEFAULT_MINIBRAIN_REG_PORT
#define DEFAULT_MINIBRAIN_REG_PORT 8181
#endif

//!
//! \class SessionFactory
//!
//! \brief Represents the Session Factory singleton responsible for creating and managing sessions.
//!
class SessionFactory
{
    private:

        //!
        //! \var guid
        //!
        //! \brief GUID of this instance of the SessionFactory class.
        //!
        const uint32_t guid;
        
        //!
        //! \var name
        //!
        //! \brief Globally unique name of this instance of SessionFactory class.
        //!        
        const string name;

        //!
        //! \var isEndpoint
        //!
        //! \brief Boolean to denote whether this instance is an endpoint or a mini-brain-initiated factory.
        //!
        const bool isEndpoint;
            
        //!
        //! \var pRegClient
        //!
        //! \brief Registration Client to register with mini-brain.
        //!
        SFRegClient* pRegClient;
    
        //!
        //! \var pTheInstance
        //!
        //! \brief Static singleton instance of the SessionFactory class.
        //!
        static SessionFactory* pTheInstance;
        
        //!
        //! \brief Private constructor to prevent instantiation from outside the class scope
        //!
        //!\param[in] guidP - Factory id parameter.
        //!\param[in] nameP - Factory name parameter.
        //!\param[in] isEndpointP - value for isEndpoint member variable.
        //!
        SessionFactory(uint32_t guidP,string& nameP,bool isEndpointP);
        
        //!
        //! \brief Private destructor to prevent deletion from outside class scope
        //!
        ~SessionFactory();
        
    public:
    
        //!
        //! \brief Static function to create the singleton instance of the SessionFactory class.
        //!
        //!\param[in] guidP - Factory id parameter.
        //!\param[in] nameP - Factory name parameter.
        //!\param[in] isEndpointP - value for isEndpoint member variable.
        //!
        //! \return TRUE upon successful creation, FALSE if instance is already created.
        //!
        static bool CreateTheInstance(uint32_t guidP,string& nameP,bool isEndpointP);

        //!
        //! \brief Static function to destroy the singleton instance of the SessionFactory class.
        //!
        //! \return TRUE if successful, FALSE if instance already destroyed.
        //!
        static bool DestroyTheInstance(void);

        //!
        //! \brief Static function to return the singleton instance of the SessionFactory class.
        //!
        //! \return Singleton instance of the SessionFactory class.
        //!
        static SessionFactory *TheInstance(void);
        
        //!
        //! \brief Function to get the factory id of the SessionFactory singleton instance.
        //!
        //! \return Factory id.
        //!
        uint32_t Guid(void);

        //!
        //! \brief Function to get the factory name of the SessionFactory singleton instance.
        //!
        //! \return Factory name.
        //!
        string Name(void);
        
        //!
        //! \brief Function to return TRUE if session factory is part of an endpoint, FALSE if not.
        //!
        //! \return Bool
        //!
        bool IsEndpoint(void);
        
        //!
        //! \brief Function to configure mini-brain registration client.
        //!
        //! \param[in] mBrainIP - IP address of the host running the mini-brain
        //! \param[in] httpPort - Port of registration at mini-brain host [default 8181]
        //!
        //! \return VOID.
        //!
        void ConfigMbrainRegClient(string& mBrainIP,uint32_t httpPort=DEFAULT_MINIBRAIN_REG_PORT);
        
        //!
        //! \brief Function to register factory with mini-brain.
        //!
        //! \return TRUE if successful, FALSE if not.
        //!
        bool RegWithMbrain(void);
        
        //!
        //! \brief Function to abort registration with mini-brain.
        //!
        //! \return TRUE if successful, FALSE if not.
        //!
        bool ReqAbortReg(void);
        
        //!
        //! \brief Function to stop registration thread.
        //!
        //! \return VOID.
        //!
        //! \note This function is to be called only to set 'isRunning' to 'false'.
        //!
        void StopRegThread(void);
};

#endif //SESSIONFACTORY_H_

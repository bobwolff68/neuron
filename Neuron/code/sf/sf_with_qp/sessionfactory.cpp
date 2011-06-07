//!
//! \file sessionfactory.cpp
//!
//! \brief Definition of the member functions of the SessionFactory class.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#include "sessionfactory.h"

SessionFactory* SessionFactory::pTheInstance = (SessionFactory*)0;

SessionFactory::SessionFactory(uint32_t guidP,string& nameP,bool isEndpointP):
guid(guidP),name(nameP),isEndpoint(isEndpointP)
{
    pRegClient = (SFRegClient*)0;
}

SessionFactory::~SessionFactory()
{
    if(pRegClient!=(SFRegClient*)0)
        delete pRegClient;
}

bool SessionFactory::CreateTheInstance(uint32_t guidP,string &nameP,bool isEndpointP)
{
    if(pTheInstance!=(SessionFactory*)0)
        return false;
    
    pTheInstance = new SessionFactory(guidP,nameP,isEndpointP);
    return true;
}

bool SessionFactory::DestroyTheInstance(void)
{
    if(pTheInstance==(SessionFactory*)0)
        return false;
        
    delete pTheInstance;
    pTheInstance = (SessionFactory*)0;
    return true;
}

SessionFactory *SessionFactory::TheInstance(void)
{
    return pTheInstance;
}

uint32_t SessionFactory::Guid(void)
{
    return guid;
}

string SessionFactory::Name(void)
{
    return name;
}

bool SessionFactory::IsEndpoint(void)
{
    return isEndpoint;
}

void SessionFactory::ConfigMbrainRegClient(string &mBrainIP,uint32_t httpPort)
{
    if(pRegClient!=(SFRegClient*)0)
        delete pRegClient;
    
    pRegClient = new SFRegClient(mBrainIP.c_str(),guid,httpPort,isEndpoint,name.c_str());
    return;
}

bool SessionFactory::RegWithMbrain(void)
{
    return pRegClient->registerClient();
}

bool SessionFactory::ReqAbortReg(void)
{
    // Call non-blocking abort()
    return pRegClient->abort(false);
}

void SessionFactory::StopRegThread(void)
{
    pRegClient->stopThread();
    return;
}

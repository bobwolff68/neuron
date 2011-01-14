//!
//! \file ECPObject.cpp
//!
//! \brief Defintion of ECP Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#include "ndds_cpp.h"
#include "ECPObject.h"
#include "ECPInterface.h"
#include "ECPInterfaceSupport.h"

ECPObject::ECPObject(int _srcId, int _sessionId) 
{
    sessionId = _sessionId;
    srcId = _srcId;
} 

ECPObject::~ECPObject() 
{
}

int ECPObject::GetSessionId() 
{
    return sessionId;
}

int ECPObject::GetSrcId() 
{
    return srcId;
}


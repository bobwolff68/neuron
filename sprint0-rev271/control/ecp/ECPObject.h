//!
//! \file ECPObject.h
//!
//! \brief Defintion of base-class Session Control Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ECPOBJECT_H_
#define ECPOBJECT_H_

#include "ndds_cpp.h"
#include "CPInterfaceT.h"
#include "ECPInterface.h"
#include "ECPInterfaceSupport.h"

class ECPObject : public CPObject {
public:
    
    ECPObject(int _sfId, int _sessionId);
    
    ~ECPObject();
        
    int GetSessionId();

    int GetSrcId();

protected:
    int sessionId;
    int srcId;    
};

#endif


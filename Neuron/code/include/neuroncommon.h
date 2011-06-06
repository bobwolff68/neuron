//
// Include for internal use of neuron common library
//

#ifndef NEURON_COMMON_H
#define NEURON_COMMON_H
//
// RMW mod to remove relative ../neuroncommon/ pathing.
//     Must have -I[...]code/neuroncommon/ in command line now (via CMakeLists.txt)
//
#include "threading/NeuronBasics/ThreadSingle.h"
#include "threading/NeuronBasics/ThreadMultiple.h"
#include "eventhandling/eventhandler.h"
#ifndef SKIP_DDS
#include "dds/DDSEvent.h"
#endif
#include "anyoption/anyoption.h"

#include "testjigsupport.h"

#include <string>
#include <sstream>

using namespace std;

#define coutdbg cout << __PRETTY_FUNCTION__ << "::"

/// \brief FromString<> is a template for converting the frontmost 'word' of a string
///        into the template type. It also allows for failure via the boolean flag.
///
/// Usage -     int i = FromString<int>( stringname,flag );
template<typename T>
T FromString( string& s, bool& bIsOK)
{
    T type;
    istringstream inStream(s);

    bIsOK=true;

    inStream >> type;
    if(!inStream)
        bIsOK=false;

    return type;
}

/// \brief FromStringNoChecking<> is a template for converting the frontmost 'word' of a string
///        into the template type. It does not check for failure-to-parse. Use when you know what's in the string.
///
/// Usage -     int i = FromString<int>( stringname );
template<typename T>
T FromStringNoChecking( string& s )
{
    T type;
    istringstream inStream(s);

    inStream >> type;

    return type;
}

template<typename T> string ToString(T type)
{
	stringstream ss;

	ss << type;
	return ss.str();
}

template<typename T> T ToUpper(T &s)
{
      std::string::iterator i = s.begin();
      std::string::iterator end = s.end();

      while (i != end) {
        *i = std::toupper((unsigned char)*i);
        ++i;
      }

      return s;
}

#ifdef UBRAIN
#include "controlplane.h"

#include "entity.h"

class CallbackBase
{
public:
	CallbackBase() { };
	virtual ~CallbackBase() { };
	virtual void NewSFDetected(int id)=0;
	virtual void NewSFState(com::xvd::neuron::acp::State* state)=0;
	// Process both Select and Offer -- and in the future more items.
	virtual void NewSessionState(com::xvd::neuron::scp::State* state)=0;
	virtual bool MakeLuaCallback(const char* fn, int arg, const char* txt)=0;

};

//!
//! \class ModifierBase
//! 
//! \brief 
//! 
//! \todo 
//! 

class ModifierBase
{
public:
	ModifierBase() { };
	virtual ~ModifierBase() { };
};

#endif

#endif


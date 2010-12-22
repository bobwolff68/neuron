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


#include <string>
#include <sstream>

using namespace std;

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

template<typename T> string ToString(T type)
{
	stringstream ss;

	ss << type;
	return ss.str();
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
	// Process both Select and Offer -- and in the future more items.
	virtual void NewSessionState(com::xvd::neuron::scp::State* state)=0;

};
#endif

#endif


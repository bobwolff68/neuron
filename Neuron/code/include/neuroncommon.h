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
#include "dds/DDSEvent.h"
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

#endif


/*
 * serverbase.h
 *
 *  Created on: Nov 24, 2010
 *      Author: rwolff
 */

#ifndef SERVERBASE_H_
#define SERVERBASE_H_

#include "neuroncommon.h"

class ServerBase: public ThreadSingle
{
public:
	ServerBase();
	virtual ~ServerBase();
protected:
	int workerBee(void);
};

#endif /* SERVERBASE_H_ */

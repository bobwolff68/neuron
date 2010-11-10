/*
 * sshmgmt.cpp
 *
 *  Created on: Nov 10, 2010
 *      Author: rwolff
 */

#include "sshmgmt.h"

SSHManagement::SSHManagement()
{
	// TODO Auto-generated constructor stub

}

SSHManagement::~SSHManagement()
{
	// TODO Auto-generated destructor stub
}

bool SSHManagement::hasLocalKeypair(string &in_name)
{
	fstream testin;
	string fname;

	if (in_name=="")
		fname = "~/.ssh/id_rsa.pub";
	else
		fname = in_name;

	testin.open(fname,ios::in);
	if( testin.is_open() )
	{
		testin.close();
		return true;
	}

	testin.close();
	return false;
}

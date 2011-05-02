/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/************************************************************************
 * LOGICAL_NAME:    CheckStatus.h
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the headers for the error handling operations.
 * 
 ***/

#ifndef __CHECKSTATUS_H__
#define __CHECKSTATUS_H__

//#define RTI_STYLE
#ifdef RTI_STYLE
#include <ndds/ndds_c.h>
#else
#include "dds_dcps.h"
#endif

#include <stdio.h>
#include <stdlib.h>

/* Array to hold the names for all ReturnCodes. */
char *RetCodeName[13];

/**
 * Returns the name of an error code.
 **/
char *getErrorName(DDS_ReturnCode_t);

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
void checkStatus(DDS_ReturnCode_t, const char *);

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
void checkHandle(void *, char *);

#endif
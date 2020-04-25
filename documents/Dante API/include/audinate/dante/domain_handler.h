/*
 * File     : domain_handler.h
 * Created  : August 2016
 * Author   : James Westendorp
 * Synopsis : Domain handling for DAPI clients
 *
 * This software is copyright (c) 2016-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file domain_handler.h
 * Domain Handler definitions
 */

#ifndef _DANTE_DOMAIN_HANDLER_H
#define _DANTE_DOMAIN_HANDLER_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dapi_types.h"
#endif

#ifdef DAPI_ENVIRONMENT
#error DAPI_ENVIRONMENT must be not be defined before including this file
#endif

#define DAPI_ENVIRONMENT DAPI_ENVIRONMENT__STANDALONE
#include "domain_handler_controller.h"

#endif //_DANTE_DOMAIN_HANDLER_H

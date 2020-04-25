/*
 * File     : $RCSfile$
 * Created  : March 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Error codes for the Audinate Cross-Platform API
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file platform_env.h
 * 
 * A cross-platform 'environment' abstraction used throughout the API.
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */
#ifndef _AUD_PLATFORM_ENV_H
#define _AUD_PLATFORM_ENV_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * The "aud env" structure provides an opaque abstraction
 * of the application environment. It encapsulates common
 * components used widely throughout the Dante API and components.
 * In most cases it is sufficient to simply acquire an 'env' handle
 * and pass it around as needed.
 */
typedef struct aud_env aud_env_t;

/**
 * Setup the Audinate environment and obtain a handle the the environment
 * structure. This function should be called once at the beginning of
 * an application, before any other Audinate functions are called.
 *
 * @param env a pointer to an aud_env handle that will contain the
 *   environment handle if initialisation is successful.
 *
 * @return AUD_SUCCESS if the enviroment was successfully initialised, another audinate
 *   error code otherwise.
 */
aud_error_t
aud_env_setup
(
	aud_env_t ** env
);

/**
 * Release the Audinate environment. This function should be called once
 * at the end of the application, when all other Audinate components are finished and cleaned up
 *
 * @param env an audinate environment handle previously obtained by a call to aud_env_setup
 */
void
aud_env_release
(
	aud_env_t * env
);

#ifdef __cplusplus
}
#endif

#endif


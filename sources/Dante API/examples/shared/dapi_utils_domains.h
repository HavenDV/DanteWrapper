/*
 * File     : dapi_utils_domains.h
 * Created  : March 2017
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Helper functions for Dante Domains - I/O and DDM connection management
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#ifndef _DAPI_UTILS_DOMAINS_H
#define _DAPI_UTILS_DOMAINS_H

// include the main API header
#include "dapi_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// I/O functions for domain management
//----------------------------------------------------------

char * 
dapi_utils_domain_info_to_string(const dante_domain_info_t * info, char * buf, size_t len);

void
dapi_utils_print_domain_changes(const ddh_changes_t * changes);

void
dapi_utils_print_domain_handler_info(dante_domain_handler_t * handler);

//----------------------------------------------------------
// A simple event handler that just peints the current state
//----------------------------------------------------------

ddh_change_event_fn dapi_utils_ddh_event_print_changes;

//----------------------------------------------------------
// Support functions for domain management
//----------------------------------------------------------

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

// Somewhat arbitrary lengths for string buffers to parse DDM command line args
#define DAPI_UTILS_DDM_HOSTNAME_CHARS 99
#define DAPI_UTILS_DDM_USERNAME_CHARS 99
#define DAPI_UTILS_DDM_PASSWORD_CHARS 99
#define DAPI_UTILS_DDM_DOMAIN_CHARS 99

#define DAPI_UTILS_DDM_HOSTNAME_LENGTH (DAPI_UTILS_DDM_HOSTNAME_CHARS+1)
#define DAPI_UTILS_DDM_USERNAME_LENGTH (DAPI_UTILS_DDM_USERNAME_CHARS+1)
#define DAPI_UTILS_DDM_PASSWORD_LENGTH (DAPI_UTILS_DDM_PASSWORD_CHARS+1)
#define DAPI_UTILS_DDM_DOMAIN_LENGTH (DAPI_UTILS_DDM_DOMAIN_CHARS+1)

#define DAPI_UTILS_DDM_HOSTNAME_PARAM_ "ddm"
#define DAPI_UTILS_DDM_USERNAME_PARAM_ "user"
#define DAPI_UTILS_DDM_PASSWORD_PARAM_ "pass"
#define DAPI_UTILS_DDM_DOMAIN_PARAM_ "domain"

#define DAPI_UTILS_DDM_HOSTNAME_PARAM "--" DAPI_UTILS_DDM_HOSTNAME_PARAM_
#define DAPI_UTILS_DDM_USERNAME_PARAM "--" DAPI_UTILS_DDM_USERNAME_PARAM_
#define DAPI_UTILS_DDM_PASSWORD_PARAM "--" DAPI_UTILS_DDM_PASSWORD_PARAM_
#define DAPI_UTILS_DDM_DOMAIN_PARAM "--" DAPI_UTILS_DDM_DOMAIN_PARAM_

typedef struct dapi_utils_ddm_config
{
	aud_bool_t connect;
	struct
	{
		char host[DAPI_UTILS_DDM_HOSTNAME_LENGTH];
		uint16_t port;
	} address;

	char username[DAPI_UTILS_DDM_USERNAME_LENGTH];
	char password[DAPI_UTILS_DDM_PASSWORD_LENGTH];
	char domain[DAPI_UTILS_DDM_DOMAIN_LENGTH];

} dapi_utils_ddm_config_t;

/**
 * Try and parse a dapi_utils_ddm_config_t parameter
 * return AUD_TRUE if arg was a domain parameter, AUD_FALSE otherwise,
 * if AUD_TRUE, result is AUD_SUCCESS on correct parsing, another error otherwise
 */
aud_bool_t
dapi_utils_ddm_config_parse_one(dapi_utils_ddm_config_t * config, const char * arg, aud_error_t * result);

/**
 * Parse all dapi_utils_ddm_config_t args at the beginning of an args list until an unmatched arg is found. Optionally remove them from the arg list
 */
aud_error_t
dapi_utils_ddm_config_parse(dapi_utils_ddm_config_t * config, int * argc, char * argv[], int start, aud_bool_t remove_used_options);

/**
 * Apply a dapi_utils_ddm_config_t by manually pushing the handler through the necessary discovery and connection
 * steps, using blocking while() loops to keep driving the runtime through each stage of the process.
 *
 * @note This function is intended as a support for example / utility code on PC / OSX platforms. Customer
 * applications will likely drive domain management from a UI that allows secure user credential input
 *   & interactive domain selection from the list of available domains.
 *
 * Behaviour is as follows:
 *
 * - If the ddm discovery is enabled then begin discovery and drive the runtime until discovery completed
 * - If the ddm address is manually specified then set the address in the domain handler
 * - Once in the disconnected state, if username and password are configured then attempt to connect to the ddm
 *   using the username and password, driving the runtime until connection is completed.
 * - Once connected, if a domain was specified then try to set that domain as the current domain
 *
 * @param config the DDM configuration options
 * @param handler the domain handler to use
 * @param runtime the runtime to use.
 * @param p_running A pointer to a boolean indicating that the program should be running. If p_running is
 *   asynchronously set to FALSE (eg. via a signal handler) the function will return prior to completion.
 *   p_running can safely be set to NULL if desired.
 */
aud_error_t
dapi_utils_ddm_connect_blocking(dapi_utils_ddm_config_t * config, dante_domain_handler_t * handler, dante_runtime_t * runtime, aud_bool_t * p_running);

/*
	Update current domain to the name.
*/
aud_error_t
dapi_utils_update_ddh_current_domain(dante_domain_handler_t * handler, const char * domain);

#endif

#ifdef __cplusplus
}
#endif

#endif


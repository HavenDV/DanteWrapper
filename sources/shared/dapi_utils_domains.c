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
#include "dapi_utils_domains.h"

#define USE_IDENTIFY 1

char * 
dapi_utils_domain_info_to_string(const dante_domain_info_t * info, char * buf, size_t len)
{
	if (IS_ADHOC_DOMAIN_UUID(info->uuid))
	{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		SNPRINTF(buf, len, "ADHOC");
#elif DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
		SNPRINTF(buf, len, "ADHOC,  Remote policy : %s",
			dante_access_policy_to_string(info->access_control_policy_summary));
#endif
	}
	else if (IS_MANAGED_DOMAIN_UUID(info->uuid))
	{
		dante_domain_uuid_string_t uuid_str;
		dante_domain_uuid_to_string(&info->uuid, &uuid_str);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

		SNPRINTF(buf, len, "id=%d name=%s uuid=%s role=\"%s\" access_control=%s clock_subdomain=%s", 
			 info->id, info->name, uuid_str.str,
			 info->role,
			 dante_access_policy_to_string(info->access_control_policy_summary),
			 info->clock_subdomain_name.data);
#elif DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
		SNPRINTF(buf, len, "name=%s, uuid=%s, Remote policy : %s",
			info->name[0] ? info->name : "UNKNOWN",
			uuid_str.str, dante_access_policy_to_string(info->access_control_policy_summary));


#endif
	}
	else if (IS_NO_DOMAIN_UUID(info->uuid))
	{
		SNPRINTF(buf, len, "NONE");
	}
	else
	{
		SNPRINTF(buf, len, "???");
	}
	return buf;
}

void
dapi_utils_print_domain_changes(const ddh_changes_t * changes)
{
	char buf[256];
	ddh_change_flags_t change_flags = ddh_changes_get_change_flags(changes);
	dante_domain_handler_t * handler = ddh_changes_get_domain_handler(changes);
	printf("Domain Hander Event:\n");
	printf("  Flags: %s\n", ddh_change_flags_to_string(change_flags, buf, sizeof(buf)));
	if (change_flags & DDH_CHANGE_FLAG_ERROR)
	{
		aud_error_t result = ddh_changes_get_error_code(changes);
		printf("  Error=%s\n", aud_error_get_name(result));
	}
	if (change_flags & DDH_CHANGE_FLAG_STATE)
	{
		ddh_state_t state = dante_domain_handler_get_state(handler);
		printf("  Domain Handler State: %s\n", ddh_state_to_string(state));
	}
	if (change_flags & DDH_CHANGE_FLAG_CURRENT_DOMAIN)
	{
		dante_domain_info_t current = dante_domain_handler_get_current_domain(handler);
		printf("  Current domain: %s\n", dapi_utils_domain_info_to_string(&current, buf, sizeof(buf)));
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (change_flags & DDH_CHANGE_FLAG_AVAILABLE_DOMAINS)
	{
		printf("  Available domains:\n");
		unsigned int d, nd = dante_domain_handler_num_available_domains(handler);
		for (d = 0; d < nd; d++)
		{
			dante_domain_info_t available = dante_domain_handler_available_domain_at_index(handler, d);
			printf("    %s\n", dapi_utils_domain_info_to_string(&available, buf, sizeof(buf)));
		}
	}
#elif DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	if (((change_flags & DDH_CHANGE_FLAG_CURRENT_DOMAIN) == 0) && (change_flags & DDH_CHANGE_FLAG_REMOTE_CONTROL_POLICY))
	{
		printf("  Remote policy: %s\n", dante_access_policy_to_string(dante_domain_handler_get_remote_control_policy(handler)));
	}
	if (change_flags & DDH_CHANGE_FLAG_LOCAL_CONTROL_POLICY)
	{
		printf("  Local policy: %s\n", dante_access_policy_to_string(dante_domain_handler_get_local_control_policy(handler)));
	}
#endif
	printf("\n");
}

void
dapi_utils_print_domain_handler_info(dante_domain_handler_t * handler)
{
	char buf[256];

	printf("Domain Handler Summary:\n");

	ddh_state_t state = dante_domain_handler_get_state(handler);
	printf("  State: %s\n", ddh_state_to_string(state));

	dante_domain_info_t current = dante_domain_handler_get_current_domain(handler);
	printf("  Current domain: %s\n", dapi_utils_domain_info_to_string(&current, buf, sizeof(buf)));

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	printf("  Available domains:\n");
	unsigned int d, nd = dante_domain_handler_num_available_domains(handler);
	for (d = 0; d < nd; d++)
	{
		dante_domain_info_t available = dante_domain_handler_available_domain_at_index(handler, d);
		printf("    %s\n", dapi_utils_domain_info_to_string(&available, buf, sizeof(buf)));
	}
#endif
	printf("\n");
}

void
dapi_utils_ddh_event_print_changes(const ddh_changes_t * ev)
{
	dapi_utils_print_domain_changes(ev);
}


#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

#define DAPI_UTILS_DDM_HOSTNAME_CHARS_STR DAPI_UTILS_STRINGIFY(DAPI_UTILS_DDM_HOSTNAME_CHARS)
#define DAPI_UTILS_DDM_USERNAME_CHARS_STR DAPI_UTILS_STRINGIFY(DAPI_UTILS_DDM_USERNAME_CHARS)
#define DAPI_UTILS_DDM_PASSWORD_CHARS_STR DAPI_UTILS_STRINGIFY(DAPI_UTILS_DDM_PASSWORD_CHARS)
#define DAPI_UTILS_DDM_DOMAIN_CHARS_STR   DAPI_UTILS_STRINGIFY(DAPI_UTILS_DDM_DOMAIN_CHARS)


static void
invalid_parameter(const char * arg, aud_error_t * result)
{
	fprintf(stderr, "Invalid parameter: '%s'\n", arg);
	* result = AUD_ERR_INVALIDPARAMETER;
}

aud_bool_t
dapi_utils_ddm_config_parse_one(dapi_utils_ddm_config_t * config, const char * arg, aud_error_t * result)
{
	if (!(config && arg && result))
	{
		return AUD_FALSE;
	}
	const char * value = NULL;
	size_t prefix_len;
	for(prefix_len = 0; arg[prefix_len]; prefix_len++)
	{
		if (arg[prefix_len] == '=')
		{
			value = arg + prefix_len + 1;
			break;
		}
	}
		// roll our own rather than use strchr so we can get length at the
		// same time

	*result = AUD_SUCCESS;

	if (!strncmp(arg, DAPI_UTILS_DDM_HOSTNAME_PARAM, prefix_len))
	{
		if (value)
		{
			unsigned int port;
			if (sscanf(value, "%" DAPI_UTILS_DDM_HOSTNAME_CHARS_STR "[^:]:%5u", config->address.host, &port) == 2)
			{
				config->address.port = (uint16_t) port;
			}
			else
			{
				invalid_parameter(arg, result);
				return AUD_TRUE;
			}
		}
		if (config->connect)
		{
			*result = AUD_ERR_INVALIDPARAMETER;
		}
		config->connect = AUD_TRUE;
		return AUD_TRUE;
	}
	else if (!strncmp(arg, DAPI_UTILS_DDM_USERNAME_PARAM, prefix_len))
	{
		if (value && (sscanf(value, "%" DAPI_UTILS_DDM_USERNAME_CHARS_STR "s", config->username) == 1))
		{}
		else
		{
			invalid_parameter(arg, result);
		}
		return AUD_TRUE;
	}
	else if (!strncmp(arg, DAPI_UTILS_DDM_PASSWORD_PARAM, prefix_len))
	{
		if (value && (sscanf(value, "%" DAPI_UTILS_DDM_PASSWORD_CHARS_STR "s", config->password) == 1))
		{}
		else
		{
			invalid_parameter(arg, result);
		}
		return AUD_TRUE;
	}
	else if (!strncmp(arg, DAPI_UTILS_DDM_DOMAIN_PARAM, prefix_len))
	{
		if (value && value[0])
		{
			size_t len = aud_strlcpy(config->domain, value, DAPI_UTILS_DDM_DOMAIN_CHARS);
			if (len >= DAPI_UTILS_DDM_DOMAIN_CHARS)
			{
				config->domain[0] = 0;
				invalid_parameter(arg, result);
			}
		}
		else
		{
			config->domain[0] = 0;
		}
		return AUD_TRUE;
	}
	else
	{
		return AUD_FALSE;
	}
}

aud_error_t
dapi_utils_ddm_config_parse(dapi_utils_ddm_config_t * config, int * argc, char * argv[], int from, aud_bool_t remove_used_options)
{
	memset(config, 0, sizeof(*config));

	int a = from;
	while (a < *argc)
	{
		char * arg = argv[a];

		aud_error_t result;
		if (!dapi_utils_ddm_config_parse_one(config, arg, &result))
		{
			break;
		}
		if (remove_used_options)
		{
			// shift remaining options down...
			int j;
			for (j = a; j < (*argc) - 1; j++)
			{
				argv[j] = argv[j+1];
			}
			(*argc)--;
		}
		else
		{
			a++;
		}
	}
	return AUD_SUCCESS;
}

aud_error_t
dapi_utils_ddm_connect_blocking(dapi_utils_ddm_config_t * config, dante_domain_handler_t * handler, dante_runtime_t * runtime, aud_bool_t * p_running)
{
	aud_bool_t temp_running = AUD_TRUE;
	if (!p_running) p_running = &temp_running;

	if (!(config->connect))
	{
		return AUD_FALSE;
	}
	aud_error_t result;

	if (config->address.host[0] && config->address.port)
	{
		printf("Setting Domain Manager Address\n");
		result = dante_domain_handler_set_manual_ddm(handler, config->address.host, config->address.port);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting ddm address: %s\n", aud_error_get_name(result));
			return result;
		}
		//PRINT_EVENT(pevent, dante_domain_handler_set_manual_ddm)
	}
	else
	{
		printf("Discovering Domain Manager\n");
		// Note: may need to add interface here at some point...
		result = dante_domain_handler_start_discovery(handler, NULL);

		if (result != AUD_SUCCESS)
		{
			printf("Error starting ddm discovery: %s\n", aud_error_get_name(result));
			return result;
		}
		//PRINT_EVENT(pevent, dante_domain_handler_start_discovery);
		while (*p_running && dante_domain_handler_get_state(handler) == DDH_STATE_DISCOVERING)
		{
			dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		}
		if (dante_domain_handler_get_state(handler) != DDH_STATE_DISCONNECTED)
		{
			printf("Unable to discover ddm\n");
			return AUD_ERR_NOTFOUND;
		}
		printf("Discovered Domain Manager\n");
	}

#if USE_IDENTIFY == 1

	result = dante_domain_handler_identify(handler);
	if (result != AUD_SUCCESS)
	{
		printf("Error initiating identification of the Domain Manager: %s\n", aud_error_get_name(result));
		return result;
	}
	//PRINT_EVENT(pevent, dante_domain_handler_identify)

	while (*p_running && dante_domain_handler_get_state(handler) == DDH_STATE_IDENTIFYING)
	{
		dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
	}

	if (dante_domain_handler_get_state(handler) != DDH_STATE_IDENTIFIED)
	{
		printf("Unable to identify the DDM\n");
		return AUD_ERR_NOTFOUND;
	}

	const char* identity = NULL;
	result = dante_domain_handler_get_identity(handler, &identity);
	if (result != AUD_SUCCESS)
	{
		printf("Error getting identity of the Domain Manager: %s\n", aud_error_get_name(result));
		return result;
	}
	else if (identity == NULL || *identity == 0) {
		printf("Empty identity for the Domain Manager: %s\n", aud_error_get_name(result));
		return result;
	}

	printf("Server Identity: %s\n", identity);

#endif 

	if (config->username[0] && config->password[0])
	{

		printf("Connecting to Domain Manager using credentials provided\n");		
		result = dante_domain_handler_connect(handler, config->username, config->password);
		if (result != AUD_SUCCESS)
		{
			printf("Error initiating connection to Domain Manager: %s\n", aud_error_get_name(result));
			return result;
		}
		
		//PRINT_EVENT(pevent, dante_domain_handler_connect)

		while (*p_running && dante_domain_handler_get_state(handler) == DDH_STATE_CONNECTING)
		{
			dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		}

		if (dante_domain_handler_get_state(handler) != DDH_STATE_CONNECTED)
		{
			printf("Unable to connect to the DDM\n");
			return AUD_ERR_NOTFOUND;
		}

		printf("Connected to Domain Manager\n");
		if (config->domain[0])
		{
			dapi_utils_update_ddh_current_domain(handler, config->domain);
		}
	}
	return AUD_SUCCESS;
}

aud_error_t 
dapi_utils_update_ddh_current_domain(dante_domain_handler_t * handler, const char * domain)
{
	aud_error_t result;

	dante_domain_info_t info = dante_domain_handler_available_domain_with_name(handler, domain);
	if (info.id != DANTE_DOMAIN_ID_NONE)
	{
		// The domain is available !
		printf("Setting current domain to %s\n", info.name);
		result = dante_domain_handler_set_current_domain_by_uuid(handler, info.uuid);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting current domain: %s\n", aud_error_get_name(result));
			return result;
		}
		printf("\nSet current domain to %s \n", domain);
	}
	else
	{
		printf("\nWARNING: Domain %s not available\n", domain);
		info = dante_domain_handler_get_current_domain(handler);
		if (info.name[0])
		{
			printf("WARNING: Current domain set to %s\n\n", info.name);
		}
		else if (IS_ADHOC_DOMAIN_UUID(info.uuid))
		{
			printf("WARNING: Current domain set to ADHOC\n\n");
		}
	}
	return AUD_SUCCESS;
}

#endif



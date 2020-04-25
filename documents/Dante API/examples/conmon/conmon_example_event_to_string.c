/*
 * File     : conmon_examples.c
 * Created  : July 2008
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Common information for the conmon example clients
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#include "conmon_examples.h"
#include <stdio.h>


//----------------------------------------------------------
// Printing functions for the example clients
//----------------------------------------------------------

char *
conmon_example_event_to_string
(
	const conmon_client_event_t * ev,
	char * buf,
	size_t len
) {
	size_t offset = 0;

	conmon_client_event_flags_t flags = conmon_client_event_get_flags(ev);
	conmon_client_t * client = conmon_client_event_get_client(ev);
	
	offset += SNPRINTF(buf+offset, len-offset, "Conmon event received:\n");
	if (flags & CONMON_CLIENT_EVENT_FLAG__DOMAIN_CHANGING)
	{
		dante_domain_uuid_t uuid = conmon_client_get_domain_uuid(client);
		dante_domain_uuid_string_t uuid_str;
		dante_domain_uuid_to_string(&uuid, &uuid_str);
		offset += SNPRINTF(buf+offset, len-offset, "  DOMAIN_CHANGING (domain=%s)\n", uuid_str.str);
	}
	if (flags & CONMON_CLIENT_EVENT_FLAG__DOMAIN_CHANGED)
	{
		dante_domain_uuid_t uuid = conmon_client_get_domain_uuid(client);
		dante_domain_uuid_string_t uuid_str;
		dante_domain_uuid_to_string(&uuid, &uuid_str);
		offset += SNPRINTF(buf+offset, len-offset, "  DOMAIN_CHANGED (domain=%s)\n", uuid_str.str);
	}

	if (flags & CONMON_CLIENT_EVENT_FLAG__DDM_CONNECTION_CHANGED)
	{
		offset += SNPRINTF(buf+offset, len-offset, "  DDM_CONNECTION_CHANGED (connected=%s)\n", conmon_client_get_ddm_is_connected(client) ? "TRUE" : "FALSE");
	}
	if (flags & CONMON_CLIENT_EVENT_FLAG__LOCAL_CONNECTION_CHANGED)
	{
		offset += SNPRINTF(buf+offset, len-offset, "  LOCAL_CONNECTION_CHANGED (connected=%s)\n", (conmon_client_state(client) == CONMON_CLIENT_CONNECTED) ? "TRUE" : "FALSE");
	}

	if (flags &CONMON_CLIENT_EVENT_FLAG__LOCAL_READY_CHANGED)
	{
		offset += SNPRINTF(buf+offset, len-offset, "  LOCAL_READY_CHANGED (ready=%s)\n", conmon_client_is_local_ready(client) ? "TRUE" : "FALSE");
	}
	if (flags & CONMON_CLIENT_EVENT_FLAG__REMOTE_READY_CHANGED)
	{
		offset += SNPRINTF(buf+offset, len-offset, "  REMOTE_READY_CHANGED (ready=%s)\n", conmon_client_is_remote_ready(client) ? "TRUE" : "FALSE");
	}
	if (flags & CONMON_CLIENT_EVENT_FLAG__MAX_BODY_SIZE_CHANGED)
	{
		offset += SNPRINTF(buf+offset, len-offset, "  MAX_BODY_SIZE_CHANGED (control=%u monitoring=%u)\n", 
			(unsigned int) conmon_client_control_message_max_body_size(client),
			(unsigned int) conmon_client_monitoring_message_max_body_size(client));
	}
	

	return buf;
}

//----------------------------------------------------------

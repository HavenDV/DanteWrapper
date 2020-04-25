/*
 * File     : $RCSfile$
 * Created  : September 2008
 * Updated  : $Date$
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Passive conmon client that listens to Audinate events
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */


//----------
// Include

#include "dapi_utils_domains.h"
#include "conmon_examples.h"

#ifdef WIN32
#else
	#include <libgen.h>
	#include <errno.h>
#endif

//----------
// Types and Constants

static char * g_progname;

enum
{
	LISTEN_MAX_TARGETS = 16,
	FILTER_MAX_FILTERS = 16
};

typedef enum message_filter_mode
{
	MESSAGE_FILTER_MODE_PASS = 0,
	MESSAGE_FILTER_MODE_FAIL
} message_filter_mode_t;

typedef enum print_mode_raw
{
	PRINT_MODE_RAW_NONE = 0,
		// Don't print raw messages
	PRINT_MODE_RAW_UNKNOWN,
		// Print raw messages if printing fails
	PRINT_MODE_RAW_ALWAYS
		// Always print raw messages
} print_mode_raw_t;

typedef struct conmon_info conmon_info_t;

struct conmon_info
{
	// conmon connectivity
	aud_env_t * env;
	dapi_t * dapi;
	dante_runtime_t * runtime;
	dante_domain_handler_t * handler;
	uint16_t server_port;
	conmon_client_t * client;
	uint32_t server_address;

	// runtime
	aud_bool_t running;
	int result;
	conmon_client_state_t curr_state;
	
	// Target devices
	aud_bool_t all;
	conmon_client_request_id_t all_req_id;

	// options
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config;
#endif
	aud_bool_t quiet;
	
	unsigned int n_targets;
	struct conmon_target
	{
		const char * name;
		const conmon_client_subscription_t * sub;
		conmon_rxstatus_t old_status;
		aud_bool_t found;

		conmon_client_request_id_t req_id;

		const conmon_instance_id_t * conmon_id;
		char id_buf [64];
	} targets [LISTEN_MAX_TARGETS];

	unsigned n_filters;
	message_filter_mode_t filter_mode;
	struct message_filter
	{
		conmon_audinate_message_type_t mtype;
	} filter [FILTER_MAX_FILTERS];

	struct conmon_info_raw
	{
		print_mode_raw_t mode;
		aud_bool_t offsets;
		unsigned n_filters;
		struct message_filter filter[FILTER_MAX_FILTERS];
	} raw;
};

typedef struct conmon_target cm_target_t;


//----------
// Local function prototypes

AUD_INLINE const char *
pname (void)
{
#ifdef WIN32
	return g_progname;
#else
	return basename (g_progname);
#endif
}


static void
timestamp_event (void);

static void
timestamp_error (void);


static int
handle_args (conmon_info_t * cm, unsigned int argc, char ** argv);

static aud_error_t
setup_conmon (conmon_info_t * cm);

static aud_error_t
shutdown_conmon (conmon_info_t * cm);

static void
setup_local_registrations(conmon_info_t * cm);

static void
setup_remote_registrations(conmon_info_t * cm);

static void
setup_remote_subscriptions(conmon_info_t * cm);

static cm_target_t *
target_for_sub (conmon_info_t * cm, const conmon_client_subscription_t * sub);

static aud_bool_t
can_setup_local_registrations(conmon_info_t * cm);

//----------
// Callback prototypes

static conmon_client_connection_state_changed_fn conmon_cb_connection_state_changed;

static conmon_client_response_fn
	conmon_cb_reg_monitoring,
	conmon_cb_reg_sub, conmon_cb_reg_sub_all;

static conmon_client_handle_networks_changed_fn conmon_cb_network;
static conmon_client_handle_dante_device_name_changed_fn conmon_cb_dante_device_name;
static conmon_client_handle_dns_domain_name_changed_fn conmon_cb_dns_domain_name;
//static conmon_client_handle_subscriptions_changed_fn conmon_cb_subscriptions;

static conmon_client_handle_monitoring_message_fn conmon_cb_monitoring;

static conmon_client_handle_subscriptions_changed_fn conmon_cb_cm_sub_changed;

static conmon_client_event_fn conmon_cb_client_event;

static ddh_change_event_fn conmon_cb_handle_ddh_changes;

//----------
// Main

int
main (int argc, char ** argv)
{
	aud_error_t result;
	conmon_info_t info = { 0 };
	
	result = handle_args (& info, argc, argv);
	if (result != 0)
	{
		return result;
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	result = setup_conmon (& info);
	if (result != AUD_SUCCESS)
	{
		return 1;
	}

	info.running = AUD_TRUE;
	while (info.running)
	{
		result = dapi_utils_step(info.runtime, AUD_SOCKET_INVALID, NULL);
		if (result != AUD_SUCCESS)
		{
			timestamp_error();
			fprintf(stderr,
				"event loop failed: %s (%d)\n"
				, aud_error_get_name(result)
				, (int) result
			);
			break;
		}
	}
	
	shutdown_conmon (& info);
	
	return info.result;
}


//----------
// Local functions

static void
timestamp_event ()
{
	aud_ctime_buf_t buf;
	printf ("#EVENT %s: ", aud_utime_ctime_no_newline (NULL, buf));
}


static void
timestamp_error ()
{
	aud_ctime_buf_t buf;
	fprintf (stderr, "#ERROR %s: ", aud_utime_ctime_no_newline (NULL, buf));
}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
//check that we are in required domain
static aud_bool_t
conmon_client_in_config_domain(conmon_info_t * info)
{
	dante_domain_info_t current = dante_domain_handler_get_current_domain(info->handler);

	//Is current domain the one that we want to be on?
	if (info->ddm_config.domain[0] && strcmp(current.name, info->ddm_config.domain))
	{
		return AUD_FALSE;
	}
	return AUD_TRUE;
}

#endif

static aud_error_t
setup_conmon (conmon_info_t * cm)
{
	conmon_client_config_t * config = NULL;
	aud_error_t result;
	aud_errbuf_t ebuf;

	cm->env = NULL;
	cm->client = NULL;
	cm->running = NO;
	cm->result = 0;

	result = dapi_new(&cm->dapi);
	if (result != AUD_SUCCESS)
	{
		timestamp_error ();
		fprintf (stderr,
			"%s: conmon failed to initialise environment: %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
		return result;
	}

	cm->env = dapi_get_env(cm->dapi);
	cm->runtime = dapi_get_runtime(cm->dapi);
	cm->handler = dapi_get_domain_handler(cm->dapi);

	printf("Created Domain Handler, current state is:\n");
	dapi_utils_print_domain_handler_info(cm->handler);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&cm->ddm_config, cm->handler, cm->runtime, NULL);
	if (result != AUD_SUCCESS)
	{
		goto l__error;
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(cm->handler);
	dante_domain_handler_set_context(cm->handler, cm);
	dante_domain_handler_set_event_fn(cm->handler, conmon_cb_handle_ddh_changes);

	//conmon_client_env_set_network_logging (
	//	cm->env, 9889, NULL
	//);

	config = conmon_client_config_new("demo_listener");
	if (!config)
	{
		timestamp_error ();
		fprintf (stderr,
			"%s: conmon failed to initialise client: NO_MEMORY\n"
			, pname ()
		);
		goto l__error;
	}
	if (cm->server_port)
	{
		conmon_client_config_set_server_port(config, cm->server_port);
	}
	if (cm->server_address)
	{
		conmon_client_config_set_server_host_ipv4(config, cm->server_address);
	}
	result = conmon_client_new_dapi (cm->dapi, config, &cm->client);
	conmon_client_config_delete(config);
	config = NULL;

	if (result != AUD_SUCCESS)
	{
		timestamp_error ();
		fprintf (stderr,
			"%s: conmon failed to initialise client: %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
		goto l__error;
	}

	// setup callbacks
	conmon_client_set_context (cm->client, cm);
	conmon_client_set_connection_state_changed_callback(cm->client, conmon_cb_connection_state_changed);
	conmon_client_set_networks_changed_callback (cm->client, & conmon_cb_network);
	conmon_client_set_dante_device_name_changed_callback (cm->client, & conmon_cb_dante_device_name);
	conmon_client_set_dns_domain_name_changed_callback (cm->client, & conmon_cb_dns_domain_name);
	conmon_client_set_subscriptions_changed_callback (cm->client, conmon_cb_cm_sub_changed);
	conmon_client_set_event_callback(cm->client, conmon_cb_client_event);

	result = conmon_client_auto_connect(cm->client);
	if (result != AUD_SUCCESS)
	{
		timestamp_error ();
		fprintf (stderr,
			"%s: conmon failed to initiate connection: %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
		goto l__error;
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (!conmon_client_in_config_domain(cm))
	{
		printf("WARNING: Unable to Register subscriptions as requested domain is not available\n");
		return AUD_SUCCESS;
	}
#endif

	// See if we are ready to set anything up yet...
	if (conmon_client_is_local_ready(cm->client) && can_setup_local_registrations(cm))
	{
		setup_local_registrations(cm);
	}
	if (conmon_client_is_remote_ready(cm->client))
	{
		setup_remote_registrations(cm);
		setup_remote_subscriptions(cm);
	}

	return AUD_SUCCESS;

l__error:
	shutdown_conmon (cm);
	
	return result;
}


static aud_error_t
shutdown_conmon (conmon_info_t * cm)
{
	if (cm->client)
	{
		conmon_client_delete (cm->client);
		cm->client = NULL;
	}
	
	if (cm->dapi)
	{
		dapi_delete(cm->dapi);
		cm->handler = NULL;
		cm->runtime = NULL;
		cm->env = NULL;
		cm->dapi = NULL;
	}
	
	return AUD_SUCCESS;
}

static aud_bool_t
can_setup_local_registrations(conmon_info_t * cm)
{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (IS_MANAGED_DOMAIN_UUID(dante_domain_handler_get_current_domain_uuid(cm->handler)))
	{
		//do not subscribe to local channel(s) if in a managed domain in standalone mode.
		return AUD_FALSE;
	}

#endif
	return AUD_TRUE;
}

static void
setup_local_registrations(conmon_info_t * cm)
{
	aud_error_t result;
	// Register for outgoing (TX) status messages
	printf("Registering for TX status messages\n");
	result =
		conmon_client_register_monitoring_messages (
			cm->client, & conmon_cb_reg_monitoring, NULL,
			CONMON_CHANNEL_TYPE_STATUS, CONMON_CHANNEL_DIRECTION_TX,
			& conmon_cb_monitoring
		);
	if (result != AUD_SUCCESS)
	{
		aud_errbuf_t ebuf;
		
		timestamp_error ();
		fprintf (stderr,
			"%s: failed to register monitoring channel (status): %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
	}

	// Register for outgoing (TX) broadcast messages
	printf("Registering for TX broadcast messages\n");
	result =
		conmon_client_register_monitoring_messages (
			cm->client, & conmon_cb_reg_monitoring, NULL,
			CONMON_CHANNEL_TYPE_BROADCAST, CONMON_CHANNEL_DIRECTION_TX,
			& conmon_cb_monitoring
		);
	if (result != AUD_SUCCESS)
	{
		aud_errbuf_t ebuf;
		
		timestamp_error ();
		fprintf (stderr,
			"%s: failed to register monitoring channel (broadcast): %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
	}

	// Register for "outgoing" (TX) local messages
	printf("Registering for TX local messages\n");
	result =
		conmon_client_register_monitoring_messages (
			cm->client, & conmon_cb_reg_monitoring, NULL,
			CONMON_CHANNEL_TYPE_LOCAL, CONMON_CHANNEL_DIRECTION_TX,
			& conmon_cb_monitoring
		);
	if (result != AUD_SUCCESS)
	{
		aud_errbuf_t ebuf;
		
		timestamp_error ();
		fprintf (stderr,
			"%s: failed to register monitoring channel (local): %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
	}
}

static void
setup_remote_registrations(conmon_info_t * cm)
{
	aud_error_t result;
	// register for incoming (RX) status messages
	dante_domain_uuid_t uuid = conmon_client_get_domain_uuid(cm->client);
	dante_domain_uuid_string_t uuid_str;
	dante_domain_uuid_to_string(&uuid, &uuid_str);

	printf("Registering for RX status messages (domain=%s)\n", uuid_str.str);
	result =
		conmon_client_register_monitoring_messages(
		cm->client, &conmon_cb_reg_monitoring, NULL,
		CONMON_CHANNEL_TYPE_STATUS, CONMON_CHANNEL_DIRECTION_RX,
		&conmon_cb_monitoring
		);
	if (result != AUD_SUCCESS)
	{
		aud_errbuf_t ebuf;
		timestamp_error();
		fprintf(stderr,
			"%s: failed to register external monitoring channel (status): %s (%d)\n"
			, pname()
			, aud_error_message(result, ebuf), result
			);
	}

	// register for incoming (RX) broadcast messages
	printf("Registering for RX broadcast messages (domain=%s)\n", uuid_str.str);
	result =
		conmon_client_register_monitoring_messages(
		cm->client, &conmon_cb_reg_monitoring, NULL,
		CONMON_CHANNEL_TYPE_BROADCAST, CONMON_CHANNEL_DIRECTION_RX,
		&conmon_cb_monitoring
		);
	if (result != AUD_SUCCESS)
	{
		aud_errbuf_t ebuf;

		timestamp_error();
		fprintf(stderr,
			"%s: failed to register external monitoring channel (rx broadcast): %s (%d)\n"
			, pname()
			, aud_error_message(result, ebuf), result
			);
	}
}

static void
setup_remote_subscriptions (conmon_info_t * cm)
{
	const char * name = conmon_client_get_dante_device_name(cm->client);
	aud_error_t result;
	unsigned int i;

	dante_domain_uuid_t uuid = conmon_client_get_domain_uuid(cm->client);
	dante_domain_uuid_string_t uuid_str;
	dante_domain_uuid_to_string(&uuid, &uuid_str);
	
	if (cm->all)
	{
		printf("Subscribing for status messages from ALL devices (domain=%s)\n", uuid_str.str);
		result =
			conmon_client_subscribe_global (
				cm->client,
				conmon_cb_reg_sub_all,
				& (cm->all_req_id),
				CONMON_CHANNEL_TYPE_STATUS
			);
		if (result != AUD_SUCCESS)
		{
			return;
		}
	}
	
	for (i = 0; i < cm->n_targets; i++)
	{
		if (cm->targets[i].name == NULL)
		{
			continue;
		}
		// Don't try and subscribe to ourself
		if (name && !strcmp(name, cm->targets[i].name))
		{
			continue;
		}
		printf("Subscribing for status messages from \"%s\" (domain=%s)\n", cm->targets[i].name, uuid_str.str);
		result =
			conmon_client_subscribe (
				cm->client,
				conmon_cb_reg_sub,
				& (cm->targets [i].req_id),
				CONMON_CHANNEL_TYPE_STATUS,
				cm->targets [i].name
			);
		if (result != AUD_SUCCESS)
		{
			return;
		}
	}
}


static cm_target_t *
target_for_sub (conmon_info_t * cm, const conmon_client_subscription_t * sub)
{
	unsigned int i;
	const unsigned int n = cm->n_targets;
	
	for (i = 0; i < n; i++)
	{
		cm_target_t * target = cm->targets + i;
		if (target->name && (target->sub == sub))
		{
			return target;
		}
	}
	
	return NULL;
}


static void
print_raw_body
(
	const conmon_message_body_t * aud_msg,
	size_t body_size,
	const conmon_info_t * info
)
{
	unsigned i;

	if (info->raw.n_filters)
	{
		uint16_t aud_type = conmon_audinate_message_get_type(aud_msg);

		for (i = 0; i < info->raw.n_filters; i++)
		{
			if (aud_type == info->raw.filter[i].mtype)
			{
				goto l__pass_filter;
			}
		}
		return;
	}
	l__pass_filter:

	printf("> Body length: %u bytes:", (unsigned) body_size);
	for (i = 0; i < body_size; i++)
	{
		if ((i & 0xf) == 0)
		{
			if (info->raw.offsets)
			{
				printf("\n %04x: ", i);
			}
			else
			{
				fputs("\n\t", stdout);
			}
		}
		else if ((i & 0x3) == 0)
		{
			putchar(' ');
		}
		printf("%02x", aud_msg->data[i]);
	}
	putchar('\n');
}


//----------
// Callbacks

static void
conmon_cb_connection_state_changed
(
	conmon_client_t * client
)
{
	conmon_info_t * info = conmon_client_context (client);
	conmon_client_state_t state = conmon_client_state(client);

	//assert(info);
	if (! info)
	{
		timestamp_error ();
		fprintf (stderr, "Invalid program state - no info in conmon_cb_connection_state_changed\n");
		return;
	}

	if (state == CONMON_CLIENT_CONNECTED)
	{
		timestamp_event ();
		puts ("Conmon connection successful");

		if (info->curr_state == CONMON_CLIENT_CONNECTED)
		{
			timestamp_error ();
			fprintf (stderr, "Already connected\n");
			return;
		}
		info->curr_state = state;

		puts ("Networks:\n");
		conmon_aud_print_networks (conmon_client_get_networks (client), " ");

		printf("Dante device name '%s'\n", conmon_client_get_dante_device_name (client));
		printf("DNS domain name '%s'\n", conmon_client_get_dns_domain_name (client));

		// Note: All setup is now handled in the event callback
	}
	else if (info->curr_state != state)
	{
		if (info->curr_state == CONMON_CLIENT_CONNECTED)
		{
			timestamp_error();
			fprintf(stderr, "Conmon disconnected - attempting to reconnect\n");
		}
		info->curr_state = state;
	}
}


static void
conmon_cb_reg_monitoring (
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	(void) client;
	(void) request_id;
	
	if (result == AUD_SUCCESS)
	{
		//conmon_info_t * info = conmon_client_context (client);

		timestamp_event ();
		puts ("Conmon status registration successful");
	}
	else
	{
		aud_errbuf_t ebuf;
		
		timestamp_error ();
		fprintf (stderr,
			"%s: failed to subscribe to conmon status channel: %s (%d)\n"
			, pname ()
			, aud_error_message (result, ebuf), result
		);
	}
}


static void
conmon_cb_reg_sub (
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	struct conmon_target * target;
	
	conmon_info_t * info = conmon_client_context (client);

	aud_errbuf_t ebuf;

	if (request_id != CONMON_CLIENT_NULL_REQ_ID)
	{
		const unsigned int n = info->n_targets;
		const char * p_name;
		unsigned int i;
		
		// find the remote target we were trying to subscribe
		for (i = 0; i < n; i++)
		{
			if (request_id == info->targets [i].req_id)
			{
				target = info->targets + i;
				goto l__found_target;
			}
		}
		
		timestamp_error ();
		fprintf (stderr,
			"%s: invalid subscription request ID: 0x%p\n"
			, (p_name = pname ()), request_id
		);
		if (result != AUD_SUCCESS)
		{
			fprintf (stderr,
				"%s: error %s (%d)\n"
				, p_name
				, aud_error_message (result, ebuf), result
			);
		}
	}
	else
	{
		fprintf (stderr,
			"%s: unexpected subscription response\n"
			, pname ()
		);
	}
	
	return;
	
l__found_target:
	if (result == AUD_SUCCESS)
	{
		target->sub =
			conmon_client_get_subscription (
				info->client,
				CONMON_CHANNEL_TYPE_STATUS,
				target->name
			);
		if (! target->sub)
		{
			timestamp_error ();
			fprintf (stderr,
				"Conmon subscription registration to %s - server error\n"
				, target->name
			);
		}
	}
	else
	{
		timestamp_error ();
		fprintf (stderr,
			"%s: failed to subscribe to remote conmon device %s: %s (%d)\n"
			, pname ()
			, target->name
			, aud_error_message (result, ebuf), result
		);
	}
	
	target->req_id = CONMON_CLIENT_NULL_REQ_ID;
}


static void
conmon_cb_reg_sub_all (
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	conmon_info_t * info = conmon_client_context (client);

	aud_errbuf_t ebuf;
		
	if (request_id != CONMON_CLIENT_NULL_REQ_ID)
	{
		// find the remote target we were trying to subscribe
		if (request_id == info->all_req_id)
		{
			if (result != AUD_SUCCESS)
			{
				timestamp_error ();
				fprintf (stderr,
					"%s: failed to subscribe to all devices: %s (%d)\n"
					, pname ()
					, aud_error_message (result, ebuf), result
				);
			}

			info->all_req_id = CONMON_CLIENT_NULL_REQ_ID;
			return;
		}
	}

	fprintf (stderr,
		"%s: unexpected subscription response\n"
		, pname ()
	);

	return;
}


void
conmon_cb_cm_sub_changed (
	conmon_client_t * client,
	unsigned int num_changes,
	const conmon_client_subscription_t * const * changes
)
{
	unsigned int i;
	conmon_info_t * cm = conmon_client_context (client);
	
	for (i = 0; i < num_changes; i++)
	{
		const conmon_client_subscription_t * sub = changes [i];
		
		cm_target_t * target = target_for_sub (cm, sub);
		if (target)
		{
			conmon_rxstatus_t rxstatus =
				conmon_client_subscription_get_rxstatus (sub);

			if (target->old_status == rxstatus)
			{
				continue;
			}
			target->old_status = rxstatus;

			switch (rxstatus)
			{
			case CONMON_RXSTATUS_UNICAST:
			case CONMON_RXSTATUS_MULTICAST:
			case CONMON_RXSTATUS_DOMAIN:
				if (!target->found)
				{
					target->found = YES;
					target->conmon_id = conmon_client_subscription_get_instance_id (sub);
					conmon_example_instance_id_to_string (
						target->conmon_id,
						target->id_buf,
						sizeof(target->id_buf)
					);
					timestamp_event ();
					printf ("Subscription to '%s' active, id=%s\n"
						, target->name
						, target->id_buf
					);
				}
				break;

			case CONMON_RXSTATUS_UNRESOLVED:
				target->found = NO;
				timestamp_event ();
				printf ("Subscription to '%s' is now UNRESOLVED\n", target->name);
				break;

			// transient states, don't print anything
			case CONMON_RXSTATUS_PREPARING:
			case CONMON_RXSTATUS_RESOLVED:
				target->found = NO;
				target->id_buf [0] = 0;
				target->conmon_id = NULL;

				timestamp_event ();
				printf ("Subscription to '%s' has entered transient state 0x%04x (%s)\n", 
					target->name, rxstatus, conmon_example_rxstatus_to_string(rxstatus)
				);
				break;

			default:
				target->found = NO;
				target->id_buf [0] = 0;
				target->conmon_id = NULL;

				timestamp_event ();
				printf ("Subscription to '%s' has entered error state 0x%04x (%s)\n", 
					target->name, rxstatus, conmon_example_rxstatus_to_string(rxstatus)
				);
			}
		}
		// else we ignore because we're not interested in this subscription
	}
}


static void
conmon_cb_network (conmon_client_t * client)
{
	//conmon_info_t * info = conmon_client_context (client);
	const conmon_networks_t * networks;

	timestamp_event ();
	puts ("Addresses changed");

	puts ("Networks:");
	networks = conmon_client_get_networks (client);
	conmon_aud_print_networks (networks, " ");
}

static void
conmon_cb_dante_device_name (conmon_client_t * client)
{
	timestamp_event ();
	fprintf (stdout,
		"Dante device name changed to '%s'\n"
		, conmon_client_get_dante_device_name (client)
	);
}

static void
conmon_cb_dns_domain_name (conmon_client_t * client)
{
	timestamp_event ();
	fprintf (stdout,
		"DNS domain name changed to '%s'\n"
		, conmon_client_get_dns_domain_name (client)
	);
}


void 
conmon_cb_monitoring
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
)
{
	aud_bool_t want_to_print = AUD_FALSE;
	//conmon_info_t * info = conmon_client_context (client);
	conmon_instance_id_t id;
	char id_buf[64];
	//const conmon_audinate_message_head_t * aud_msg = (void *) body;
	uint16_t aud_type = conmon_audinate_message_get_type(body);

	conmon_info_t * info = conmon_client_context(client);

	conmon_message_head_get_instance_id(head, &id);

	if (info->n_filters)
	{
		unsigned i;
		for (i = 0; i < info->n_filters; i++)
		{
			if (aud_type == info->filter[i].mtype)
			{
				if (info->filter_mode == MESSAGE_FILTER_MODE_PASS)
					goto l__pass_filter;
				else
					return;
			}
		}
		if (info->filter_mode == MESSAGE_FILTER_MODE_PASS)
			return;
	}
	l__pass_filter:

	if (info->all)
	{
		want_to_print = AUD_TRUE;
	}
	else if (!info->quiet)
	{
		want_to_print = AUD_TRUE;
	}
	else
	{
		// in 'quiet' mode we only print things we know we have specifically requested
		const char * name = conmon_client_device_name_for_instance_id(client, &id);
		if (name)
		{
			if (info->n_targets)
			{
				unsigned int i;
				for (i = 0; i < info->n_targets; i++)
				{
					if (!STRCASECMP(name, info->targets[i].name))
					{
						want_to_print = AUD_TRUE;
						break;
					}
				}
			}
			else if (!STRCASECMP(name, conmon_client_get_dante_device_name(info->client)))
			{
				want_to_print = AUD_TRUE;
			}
		}
	}
	if (!want_to_print)
	{
		return;
	}

	timestamp_event ();
	
	
	{
		aud_bool_t printed;
		uint16_t body_size = conmon_message_head_get_body_size(head);
		const char * device_name = conmon_client_device_name_for_instance_id(client, &id);
		uint16_t aud_version = conmon_audinate_message_get_version(body);
		printf (
			"Received status message from %s (%s)\n:"
			"  chan=%s (%s) size=%d aud-version=0x%04x aud-type=0x%04x\n"
			, conmon_example_instance_id_to_string(&id, id_buf, sizeof(id_buf))
			, (device_name ? device_name : "[unknown device]")
			, conmon_example_channel_type_to_string(channel_type)
			, (channel_direction == CONMON_CHANNEL_DIRECTION_TX ? "tx" : "rx")
			, conmon_message_head_get_body_size(head)
			, (unsigned int) aud_version
			, (unsigned int) aud_type
		);
		
		printed = conmon_aud_print_msg (body, body_size);
		if (info->raw.mode == PRINT_MODE_RAW_ALWAYS ||
			(info->raw.mode == PRINT_MODE_RAW_UNKNOWN && !printed)
		)
		{
			print_raw_body(body, body_size, info);
		}
	}
}

void conmon_cb_client_event(const conmon_client_event_t * ev)
{
	conmon_client_t * client = conmon_client_event_get_client(ev);
	conmon_info_t * info = conmon_client_context(client);
	conmon_client_event_flags_t flags = conmon_client_event_get_flags(ev);

	printf("Got a conmon client event, flags=0x%08x\n", flags);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (!conmon_client_in_config_domain(info))
	{
		return;
	}
#endif

	if (flags & CONMON_CLIENT_EVENT_FLAG__LOCAL_READY_CHANGED)
	{
		if (conmon_client_is_local_ready(client) && can_setup_local_registrations(info))
		{
			setup_local_registrations(info);
		}
	}

	if (flags & CONMON_CLIENT_EVENT_FLAG__REMOTE_READY_CHANGED)
	{
		if (conmon_client_is_remote_ready(client))
		{
			setup_remote_registrations(info);
			setup_remote_subscriptions(info);
		}
	}
}

static void 
conmon_cb_handle_ddh_changes
(
	const ddh_changes_t * changes
) {


	dapi_utils_print_domain_changes(changes);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

	dante_domain_handler_t * handler = ddh_changes_get_domain_handler(changes);
	ddh_change_flags_t change_flags = ddh_changes_get_change_flags(changes);
	conmon_info_t * cm = dante_domain_handler_get_context(handler);

	if (change_flags & DDH_CHANGE_FLAG_AVAILABLE_DOMAINS && cm->ddm_config.domain[0])
	{
		dante_domain_info_t info = dante_domain_handler_get_current_domain(cm->handler);
		if (strcmp(info.name, cm->ddm_config.domain))
		{
			dapi_utils_update_ddh_current_domain(handler, cm->ddm_config.domain);
		}
	}

#endif
}


//----------
// Args handling

static int
usage (const char *);

static int
handle_args (conmon_info_t * cm, unsigned int argc, char ** argv)
{
	unsigned int i;
	unsigned int curr_arg_index = 1;
	unsigned int more_opts = 1;

	g_progname = argv [0];
	
	// with only help option
	if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0))
	{
		usage(NULL);
		return 1;
	}

	while (more_opts && curr_arg_index < argc)
	{
		char * arg = argv [curr_arg_index];

		if (arg [0] == '-')
		{
			message_filter_mode_t fmode = MESSAGE_FILTER_MODE_PASS;
				// for f/x case below

			curr_arg_index ++;

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
			{
				aud_error_t result;
				if (dapi_utils_ddm_config_parse_one(&cm->ddm_config, arg, &result))
				{
					if (result != AUD_SUCCESS)
					{
						return usage("Invalid domain argument");
					}
					continue;
				}
			}
#endif
			
			switch (arg [1])
			{
			case '-':
				more_opts = 0;
				break;

			case 'p':
				if (curr_arg_index >= argc)
				{
					return usage("Missing argument to -p");
				}
				else
				{
					cm->server_port = (uint16_t) atoi(argv[curr_arg_index++]);
				}
				break;

			case 'H':
				if (curr_arg_index >= argc)
				{
					return usage("Missing argument to -H");
				}
				else
				{
					aud_inet_aton((argv[curr_arg_index++]), &cm->server_address);
				}
				break;

			case 'q':
				cm->quiet = AUD_TRUE;
				break;

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE			
			case 'a':
				cm->all = AUD_TRUE;
				break;
#endif
			
			case 'x':
				fmode = MESSAGE_FILTER_MODE_FAIL;
			case 'f':
				if (cm->n_filters)
				{
					if (fmode != cm->filter_mode)
						return usage("Cannot use both -f and -x");
				}
				else
				{
					cm->filter_mode = fmode;
				}
				if (curr_arg_index >= argc)
				{
					return usage("Missing argument to -f");
				}
				else if (cm->n_filters >= FILTER_MAX_FILTERS)
				{
					return usage("Too many filters");
				}
				else
				{
					char * val = argv[curr_arg_index++];
					unsigned long argval = strtoul(val, NULL, 0);
					if ((! argval) || argval > 0xFFFF)
					{
						return usage("Invalid filter argument");
					}

					cm->filter[cm->n_filters++].mtype = (conmon_audinate_message_type_t)argval;
				}
				break;

			case 'R':
				if (cm->raw.mode < PRINT_MODE_RAW_ALWAYS)
					cm->raw.mode++;
				else
					return usage("Unknown raw mode (use -R at most twice)");
				break;

			case 'F':
				cm->raw.mode = PRINT_MODE_RAW_ALWAYS;
				if (cm->raw.n_filters >= FILTER_MAX_FILTERS)
				{
					return usage("Too many raw print filters");
				}
				else
				{
					char * val = argv[curr_arg_index++];
					unsigned long argval = strtoul(val, NULL, 0);
					if ((! argval) || argval > 0xFFFF)
					{
						return usage("Invalid raw print filter argument");
					}

					cm->raw.filter[cm->raw.n_filters++].mtype = (conmon_audinate_message_type_t)argval;
				}
				break;

			case 'O':
				cm->raw.offsets = AUD_TRUE;
				break;

			default:
				fprintf (stderr, "%s: Unknown option '%s'\n"
					, pname (), arg
				);
				return usage (NULL);
			}
		}
		else
		{
			break;
		}
	}
	
	cm->n_targets = argc - curr_arg_index;
	for (i = 0; i < cm->n_targets; i++)
	{
		cm->targets [i].name = argv [curr_arg_index ++];
		cm->targets [i].req_id = CONMON_CLIENT_NULL_REQ_ID;
		cm->targets [i].found = NO;
	}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (!cm->all && !cm->n_targets)
	{
		return usage("Missing arguments: Requires either device(s) subscription or global subscription\n");
	}
#endif
	
	return 0;
}


static int
usage (const char * msg)
{
	const char * name = pname ();

	if (msg)
	{
		fprintf (stderr, "%s: %s\n", name, msg);
	}

	fprintf (stderr,
		"Usage: %s [Options] [device ...]\n"
		, name
	);
	fprintf(stderr, "Client options:\n");
	fprintf(stderr, "  -H ADDR  Set server address (debugging purposes only)\n");
	fprintf(stderr, "  -p PORT  Set server port (for debugging purposes only)\n");
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	fprintf(stderr, "  -a  Do a global rather than per-device status subscription\n");
#endif
	fprintf(stderr, "\n");
	fprintf(stderr, "Message print options:\n");
	fprintf(stderr, "  -q  Quiet mode - don't print messages from unknown sources\n");
	fprintf(stderr, "  -x MSG_TYPE  don't print messages of type MSG_TYPE\n");
	fprintf(stderr, "  -f MSG_TYPE  onlyprint messages of type MSG_TYPE\n");
	fprintf(stderr, "  -R  Print packets in raw form\n");
	fprintf(stderr, "  -F MSG_TYPE  Print packets of type MSG_TYPE in raw form\n");
	fprintf(stderr, "  -O MSG_TYPE  Print raw offsets\n");
	fprintf(stderr, "\n");
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	fprintf(stderr, "Domain options\n");
	fprintf(stderr, "  --ddm  Enable domains, using discovery to find the DDM\n");
	fprintf(stderr, "  --ddm=HOST:PORT  Enable domains, using the specified DDM address (bypasses DDM discovery)\n");
	fprintf(stderr, "  --user=USER  Specify the username for DDM authentication\n");
	fprintf(stderr, "  --pass=PASS  Specify a password for DDM authentication\n");
	fprintf(stderr, "  --domain=DOMAIN  Specify a domain to use once connected to the DDM\n");
#endif

	return 2;
}


//----------

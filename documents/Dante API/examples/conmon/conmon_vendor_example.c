/*
 * Created  : 21 Aug 2015
 * Author   : Jerry Kim <jerry.kim@audinate.com>
 * Synopsis : Test program to send a vendor specific conmon control message and receive response
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#include "dapi_utils_domains.h"
#include "conmon_examples.h"

// Vendor id example
conmon_vendor_id_t vendor_example_id = { { 'E', 'x', 'a', 'm', 'p', 'l', 'e', 0 } };

// Callback functions
static conmon_client_handle_networks_changed_fn handle_networks_changed;
static conmon_client_handle_dante_device_name_changed_fn handle_dante_device_name_changed;
static conmon_client_connection_state_changed_fn handle_connection_state_changed;
static conmon_client_handle_monitoring_message_fn handle_monitoring_message;

static conmon_client_event_fn conmon_cb_client_event;

// Test direction
typedef enum
{
	VENDOR_SPECIFIC_TX,
	VENDOR_SPECIFIC_RX
}vendor_msg_dir_t;

// Testing vendor command
enum
{
	VENDOR_COMMAND_1 = 100,
	VENDOR_COMMAND_2
};

// Testing vendor message type
enum
{
	VENDOR_MSG_TYPE_1 = 200,
	VENDOR_MSG_TYPE_2
};

// how long to wait for a response from the server
const aud_utime_t g_comms_timeout = { 2, 0 };

aud_bool_t g_communicating = AUD_FALSE;

aud_error_t g_last_result = AUD_SUCCESS;

// example of vendor defined message structure
// Whole payload limit must be equal or less that CONMON_MESSAGE_MAX_BODY_SIZE
typedef struct vendor_message_st
{
	uint16_t message_command;
	uint16_t message_type;
	uint8_t payload[512];
}vendor_message_payload_t;

// Vendor client structure
typedef struct vendor_client
{
	aud_env_t * env;
	conmon_client_config_t * config;
	conmon_client_t * conmon;
	conmon_name_t device_name;

	dante_runtime_t * runtime;

	aud_bool_t register_control_monitering_done;
	aud_bool_t register_client_subscribe_done;
	aud_bool_t register_status_monitering_done;
	aud_bool_t vendor_specific_response_done;
	vendor_msg_dir_t message_direction;

	conmon_message_body_t body;
	vendor_message_payload_t vendor_message_st;
} vendor_client_t;

static void usage(char *cmd)
{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	fprintf(stderr, "%s %s", cmd, "[domain options] [device name] [command options] \n\n");
	fprintf(stderr, "domain options:\n");
	fprintf(stderr, "  --ddm  Enable domains, using discovery to find the DDM\n");
	fprintf(stderr, "  --ddm=HOST:PORT  Enable domains, using the specified DDM address (bypasses DDM discovery)\n");
	fprintf(stderr, "  --user=USER  Specify the username for DDM authentication\n");
	fprintf(stderr, "  --pass=PASS  Specify a password for DDM authentication\n");
	fprintf(stderr, "  --domain=DOMAIN  Specify a domain to use once connected to the DDM\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "command options:\n");
	fprintf(stderr, "  tx : For sending control packet & receiving status message\n");
	fprintf(stderr, "  rx : For receiving control packet & response status message\n");
#else
	printf("Usage: %s [device name] tx	: For sending control packet & receiving status message\n", cmd);
	printf("Usage: %s [device name] rx	: For receiving control packet & response status message\n", cmd);
#endif
}

/*
 * call back function for sending control message
 */
static void
handle_reg_response
(
	conmon_client_t * conmon,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	aud_errbuf_t errbuf;

	AUD_UNUSED(conmon);

	printf ("Got response for vendor request %p: %s\n", request_id, aud_error_message(result, errbuf));
}

// compose vendor specific packet and send it over conmon
static aud_error_t send_vendor_specific_control_msg_over_conmon(vendor_client_t *client)
{
	int i;
	uint16_t body_size;
	aud_error_t result;
	aud_errbuf_t errbuf;
	aud_utime_t control_timeout = { 5, 0 };
	conmon_client_request_id_t req_id = CONMON_CLIENT_NULL_REQ_ID;

	// compose vendor specific packet & populate payload packet
	client->vendor_message_st.message_type = ntohs(VENDOR_MSG_TYPE_1);
	client->vendor_message_st.message_command = ntohs(VENDOR_COMMAND_1);

	for(i = 0; i < sizeof(client->vendor_message_st.payload); i++)
		client->vendor_message_st.payload[i] = (uint8_t) i;

	memcpy(&client->body, (uint8_t *)&client->vendor_message_st, sizeof(vendor_message_payload_t));

	body_size = sizeof(vendor_message_payload_t);

	// we're connected, send control message
	result = conmon_client_send_control_message(client->conmon,
			handle_reg_response, &req_id,
			client->device_name, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &vendor_example_id,
			&client->body, body_size,
			&control_timeout
				 // the server will give up trying after this long, so that we don't wait forever for a response
		);

	if (result != AUD_SUCCESS)
	{
		printf ("Error sending vendor specific control message (request): %s\n", aud_error_message(result, errbuf));
	}
	else
	{
		printf("sent vendor specific control message with request id %p\n", req_id);
	}

	return result;
}

// compose vendor specific packet and send it over conmon
static aud_error_t send_vendor_specific_status_msg_over_conmon(vendor_client_t *client)
{
	int i;
	uint16_t body_size;
	aud_error_t result;
	aud_errbuf_t errbuf;

	// compose vendor specific packet & populate payload packet
	client->vendor_message_st.message_type = ntohs(VENDOR_MSG_TYPE_1);
	client->vendor_message_st.message_command = ntohs(VENDOR_COMMAND_1);

	for(i = 0; i < sizeof(client->vendor_message_st.payload); i++)
		client->vendor_message_st.payload[i] = (uint8_t) (i+1);

	memcpy(&client->body, (uint8_t *)&client->vendor_message_st, sizeof(vendor_message_payload_t));

	body_size = sizeof(vendor_message_payload_t);

	result = conmon_client_send_monitoring_message(client->conmon, NULL, NULL,
			CONMON_CHANNEL_TYPE_STATUS, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC,
			&vendor_example_id, &client->body, body_size);

	if (result != AUD_SUCCESS)
	{
		printf ("Error sending status message (request): %s\n", aud_error_message(result, errbuf));
	}
	else
	{
		printf("sent status message with request id \n");
	}

	return result;
}

/*
 * Status change callback functions
 *
 * For this example, they simply log to stdout
 */
static void
handle_connection_state_changed
(
	conmon_client_t * conmon
)
{
	printf("  Connection state changed, now %s\n", conmon_example_client_state_to_string(conmon_client_state(conmon)));
}

static void
handle_networks_changed
(
	conmon_client_t * conmon
)
{
	char buf[BUFSIZ];
	printf("  Networks changed, now %s\n",
		conmon_example_networks_to_string(
			conmon_client_get_networks(conmon), buf, sizeof(buf)
		)
	);
}

static void
handle_dante_device_name_changed
(
	conmon_client_t * conmon
)
{
	printf("  Dante device name changed, now \"%s\"\n", conmon_client_get_dante_device_name(conmon));
}

/*
 * callback function for handing monitering status messages
 * Filters for vendor specific messages matching our matching vendor id
 */
static void
handle_monitoring_message
(
	conmon_client_t * conmon,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
)
{
	vendor_client_t * client = (vendor_client_t *) conmon_client_context(conmon);
	conmon_instance_id_t id;
	char id_buf[64];
	const char *device_name;

	conmon_message_head_get_instance_id(head, &id);

	// in 'quiet' mode we only print things we know we have specifically requested
	const char * name = conmon_client_device_name_for_instance_id(conmon, &id);

	if (! name)
	{
		return;
	}
	else
	{
		device_name = client->device_name;

		// interesting device name?
		if(device_name && !STRCASECMP(device_name, name))
		{
			// comparing vendor id with example vendor id
			if (!conmon_vendor_id_equals(conmon_message_head_get_vendor_id(head), &vendor_example_id))
			{
				return;
			}
		}
	}

	{
		const char * device_name = conmon_client_device_name_for_instance_id(conmon, &id);
		uint16_t aud_version = conmon_audinate_message_get_version(body);
		printf (
			"\n\nReceived status message from %s (%s)\n:"
			"  chan=%s (%s) size=%d aud-version=0x%04x\n"
			, conmon_example_instance_id_to_string(&id, id_buf, sizeof(id_buf))
			, (device_name ? device_name : "[unknown device]")
			, conmon_example_channel_type_to_string(channel_type)
			, (channel_direction == CONMON_CHANNEL_DIRECTION_TX ? "tx" : "rx")
			, conmon_message_head_get_body_size(head)
			, (unsigned int) aud_version
		);

		{
			// vendor specific control message parsing
			uint16_t vendor_payload_size = conmon_message_head_get_body_size(head);
			vendor_message_payload_t *vendor_message_payload = (vendor_message_payload_t *)body;

			// dump vendor specific message payload
			if(vendor_payload_size && vendor_message_payload)
			{
				int i;

				printf("> Vendor specific message length: %u bytes:\n", (unsigned) vendor_payload_size);
				printf("> Vendor specific message type %4x\n", ntohs(vendor_message_payload->message_type));
				printf("> Vendor specific message cmd  %4x\n", ntohs(vendor_message_payload->message_command));

				// Show all raw data except vendor example info (4bytes)
				for (i = 0; i < (vendor_payload_size-4); i++)
				{
					if ((i & 0xf) == 0)
					{
						fputs("\n\t", stdout);
					}
					else if ((i & 0x3) == 0)
					{
						putchar(' ');
					}
					printf("%02x", vendor_message_payload->payload[i]);
				}
				putchar('\n');
			}
		}
		// set flag to exit this program
		client->vendor_specific_response_done = AUD_TRUE;
	}
}

/*
 * Call back function for register monitoring
 */
static void
conmon_cb_reg_monitoring (
	conmon_client_t * conmon,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	vendor_client_t * client = (vendor_client_t *) conmon_client_context(conmon);
	(void) client;
	(void) request_id;

	if (result == AUD_SUCCESS)
	{
		puts ("Conmon status registration successful");
		client->register_status_monitering_done = AUD_TRUE;
	}
	else
	{
		aud_errbuf_t ebuf;

		fprintf (stderr,
			"failed to subscribe to conmon status channel: %s (%d)\n"
			, aud_error_message (result, ebuf), result
		);
	}
}

/*
 * Call back function for control monitoring
 */
static void
conmon_cb_reg_control (
	conmon_client_t * conmon,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	vendor_client_t * client = (vendor_client_t *) conmon_client_context(conmon);
	(void) client;
	(void) request_id;

	if (result == AUD_SUCCESS)
	{
		puts ("Conmon control registration successful");
		client->register_control_monitering_done = AUD_TRUE;
	}
	else
	{
		aud_errbuf_t ebuf;

		fprintf (stderr,
			"failed to subscribe to conmon status channel: %s (%d)\n"
			, aud_error_message (result, ebuf), result
		);
	}
}

/*
 * callback function for control conmon vendor specific message
 */
static void
vendor_client_handle_control_message
(
	conmon_client_t * conmon,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
) {
	vendor_client_t * client = (vendor_client_t *) conmon_client_context(conmon);
	conmon_instance_id_t instant_id;

	// is the message of interest to us?
	if (conmon_message_head_get_message_class(head) != CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC)
	{
		return;
	}
	if (!conmon_vendor_id_equals(conmon_message_head_get_vendor_id(head), &vendor_example_id))
	{
		return;
	}
	if (conmon_message_head_get_body_size(head) < conmon_audinate_message_get_head_size())
	{
		return;
	}

	conmon_message_head_get_instance_id(head, &instant_id);

	const char * device_name = conmon_client_device_name_for_instance_id(conmon, &instant_id);

	if(device_name)
	{
		printf("> Vendor specific message device name - %s\n", device_name);
	}
	else printf("> Vendor specific message device name - Unknown device\n");

	// vendor specific control message parsing
	uint16_t vendor_payload_size = conmon_message_head_get_body_size(head);
	vendor_message_payload_t *vendor_message_payload = (vendor_message_payload_t *)body;

	if(vendor_payload_size && vendor_message_payload)
	{
		int i;

		printf("> Vendor specific message length: %u bytes:\n", (unsigned) vendor_payload_size);
		printf("> Vendor specific message type %4x\n", ntohs(vendor_message_payload->message_type));
		printf("> Vendor specific message cmd  %4x\n", ntohs(vendor_message_payload->message_command));

		// Show all raw data except vendor example info (4bytes)
		for (i = 0; i < (vendor_payload_size-4); i++)
		{
			if ((i & 0xf) == 0)
			{
				fputs("\n\t", stdout);
			}
			else if ((i & 0x3) == 0)
			{
				putchar(' ');
			}
			printf("%02x", vendor_message_payload->payload[i]);
		}
		putchar('\n');
	}

	// send vendor specific status message back as response
	if(client->message_direction == VENDOR_SPECIFIC_RX)
	{
		// reuse body for response back
		send_vendor_specific_status_msg_over_conmon(client);

		client->vendor_specific_response_done = AUD_TRUE;
	}
}


/*
 * Call back function for client subscribe
 */
static void
conmon_cb_reg_client_subscribe (
	conmon_client_t * conmon,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	vendor_client_t * client = (vendor_client_t *) conmon_client_context(conmon);
	(void) request_id;

	if (result == AUD_SUCCESS)
	{
		puts ("Conmon device subscribe registration successful");
		client->register_client_subscribe_done = AUD_TRUE;
	}
	else
	{
		aud_errbuf_t ebuf;

		fprintf (stderr,
			"failed to subscribe to conmon status channel: %s (%d)\n"
			, aud_error_message (result, ebuf), result
		);
	}
}

/*
 * handler for conmon connected
 * register for RX status messages and subscribe device conmon stautus message
 */
static void
handle_connect_response
(
	conmon_client_t * conmon,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	aud_errbuf_t errbuf;

	vendor_client_t * client = (vendor_client_t *)conmon_client_context(conmon);
	AUD_UNUSED(client);
	printf("Got response for connection request 0x%p: %s\n", request_id, aud_error_message(result, errbuf));
	g_communicating = AUD_FALSE;
	g_last_result = result;
}

static void 
setup_local(vendor_client_t * client)
{
	aud_error_t result;
	aud_errbuf_t errbuf;
	conmon_client_request_id_t request_id;

	// Register for incoming conmon control message
	result = conmon_client_register_control_messages(client->conmon,
		&conmon_cb_reg_control, &request_id,
		&vendor_client_handle_control_message);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Error registering (request): %s\n", aud_error_message(result, errbuf));
	}

}

static void
setup_remote(vendor_client_t * client)
{
	aud_error_t result;
	aud_errbuf_t errbuf;
	conmon_client_request_id_t request_id;

	// Register for ingoing (RX) status messages
	result =
		conmon_client_register_monitoring_messages (
			client->conmon, & conmon_cb_reg_monitoring, NULL,
			CONMON_CHANNEL_TYPE_STATUS, CONMON_CHANNEL_DIRECTION_RX,
			& handle_monitoring_message
		);
	if (result != AUD_SUCCESS)
	{
		fprintf (stderr,
			"failed to register to conmon monitoring messages %s (%d)\n"
			, aud_error_message(result, errbuf), result
		);
	}


	// subscribe status from give device
	result =
		conmon_client_subscribe (
			client->conmon,
			&conmon_cb_reg_client_subscribe,
			&request_id,
			CONMON_CHANNEL_TYPE_STATUS,
			client->device_name
		);

	if (result != AUD_SUCCESS)
	{
		fprintf (stderr,
			"failed to subscribe to conmon status channel messages %s (%d)\n"
			, aud_error_message(result, errbuf), result
		);
	}
}

void conmon_cb_client_event(const conmon_client_event_t * ev)
{
	conmon_client_t * conmon = conmon_client_event_get_client(ev);
	vendor_client_t * client = conmon_client_context(conmon);
	conmon_client_event_flags_t flags = conmon_client_event_get_flags(ev);

	printf("Got a conmon client event, flags=0x%08x\n", flags);

	if (flags & CONMON_CLIENT_EVENT_FLAG__LOCAL_READY_CHANGED)
	{
		if (conmon_client_is_local_ready(conmon))
		{
			setup_local(client);
		}
	}

	if (flags & CONMON_CLIENT_EVENT_FLAG__REMOTE_READY_CHANGED)
	{
		if (conmon_client_is_remote_ready(conmon))
		{
			setup_remote(client);
		}
	}
}

static aud_error_t
wait_for_response
(
dante_runtime_t * runtime,
const aud_utime_t * timeout
) {
	aud_utime_t now, then;
	aud_utime_get(&now);
	then = now;
	aud_utime_add(&then, timeout);

	g_communicating = AUD_TRUE;
	while (g_communicating && aud_utime_compare(&now, &then) < 0)
	{
		aud_error_t result;
		aud_utime_t delay = then;
		aud_utime_sub(&delay, &now);
#if 1
		result = dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
#else
		result = conmon_examples_event_loop_run_once(&delay);
		if (conmon_examples_event_loop_failed(result))
		{
			return result;
		}

		aud_utime_get(&now);
#endif
	}
	if (g_communicating)
	{
		return AUD_ERR_TIMEDOUT;
	}
	else
	{
		return g_last_result;
	}
}

/*
 * Wait within given time for expected conmon response message
 */
static aud_error_t
wait_for_vendor_conmon_response
(
	vendor_client_t * client,
	const aud_utime_t * timeout
)
{
	aud_error_t result;
	aud_utime_t now, then;

	// figure when we timeout based on now
	aud_utime_get(&now);
	then = now;
	aud_utime_add(&then, timeout);

	// This example only handles one set of transactions, so we embed a little
	// event loop here.  A full example would have this as the main run loop
	// (and handle timeouts via timers rather than as a loop condition)

	while(!client->vendor_specific_response_done && aud_utime_compare(&now, &then) < 0)
	{
		aud_utime_t delay = then;
		aud_utime_sub(&delay, &now);
#if 1
		result = dapi_utils_step(client->runtime, AUD_SOCKET_INVALID, NULL);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
#else
		result = conmon_examples_event_loop_run_once(&delay);
		if (conmon_examples_event_loop_failed(result))
		{
			return result;
		}
#endif
		aud_utime_get(&now);
	}

	if (!client->vendor_specific_response_done)
	{
		return AUD_ERR_TIMEDOUT;
	}
	else
	{
		return AUD_SUCCESS;
	}
}

int main(int argc, char **argv)
{
	aud_error_t result;
	aud_errbuf_t errbuf;
	conmon_client_request_id_t req_id = CONMON_CLIENT_NULL_REQ_ID;
	aud_utime_t control_timeout = { 5, 0 };
	vendor_client_t client;

	dapi_t * dapi = NULL;
	dante_domain_handler_t * handler = NULL;

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config = { 0 };
#endif
	memset(&client, 0, sizeof(client));

	if(argc < 3)
	{
		usage(argv[0]);
		exit(1);
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_config_parse(&ddm_config, &argc, argv, 1, AUD_TRUE);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif

	// get the name of the device to be controlled
	if (!strcmp(argv[1], "localhost"))
	{
		client.device_name[0] = '\0';
	}
	else
	{
		memset(client.device_name, 0, sizeof(client.device_name));
		SNPRINTF(client.device_name, CONMON_NAME_LENGTH, "%s", argv[1]);
	}

	// Testing direction
	if(!strcmp(argv[2], "tx"))
	{
		client.message_direction = VENDOR_SPECIFIC_TX;
	}
	else if(!strcmp(argv[2], "rx"))
	{
		client.message_direction = VENDOR_SPECIFIC_RX;
	}
	else
	{
		usage(argv[0]);
		exit(1);
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	result = dapi_new(&dapi);
	if (result != AUD_SUCCESS)
	{
		printf("Error initialising conmon client library: %s\n",
			aud_error_message(result, errbuf));
		goto cleanup;
	}
	client.env = dapi_get_env(dapi);
	client.runtime = dapi_get_runtime(dapi);
	handler = dapi_get_domain_handler(dapi);

	client.config = conmon_client_config_new("vendor_contoller");
	if (client.config == NULL)
	{
		printf("Error creating client config: NO_MEMORY\n");
		goto cleanup;
	}

	result = conmon_client_new_dapi(dapi, client.config, &client.conmon);

	if (result != AUD_SUCCESS)
	{
		printf("Error creating client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}

	// set vendor controller structure to context
	conmon_client_set_context(client.conmon, &client);
	// set callback functions before connecting to avoid possible race conditions / missed notifications
	conmon_client_set_connection_state_changed_callback(client.conmon, handle_connection_state_changed);
	conmon_client_set_networks_changed_callback(client.conmon, handle_networks_changed);
	conmon_client_set_dante_device_name_changed_callback(client.conmon, handle_dante_device_name_changed);
	conmon_client_set_event_callback(client.conmon, conmon_cb_client_event);

	// connect conmon client
	result = conmon_client_connect (client.conmon, & handle_connect_response, & req_id);
	if (result == AUD_SUCCESS)
	{
		printf("Connecting, request id is 0x%p\n", req_id);
	}
	else
	{
		printf("Error connecting client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}

	result = wait_for_response(client.runtime, &g_comms_timeout);
	if (result != AUD_SUCCESS)
	{
		printf("Error connecting client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}

	// See if we are ready to set anything up yet...
	if (conmon_client_is_local_ready(client.conmon))
	{
		setup_local(&client);
	}
	if (conmon_client_is_remote_ready(client.conmon))
	{
		setup_remote(&client);
	}
	

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&ddm_config, handler, client.runtime, NULL);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(handler);
	dante_domain_handler_set_event_fn(handler, dapi_utils_ddh_event_print_changes);


	while (!client.register_control_monitering_done || !client.register_status_monitering_done || !client.register_client_subscribe_done)
	{
		result = dapi_utils_step(client.runtime, AUD_SOCKET_INVALID, NULL);

		if (result != AUD_SUCCESS)
		{
			printf ("Error setting up registrations %s\n", aud_error_message(result, errbuf));
			goto cleanup;
		}
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

	if (ddm_config.domain[0])
	{
		dante_domain_info_t current = dante_domain_handler_get_current_domain(handler);
		if (strcmp(current.name, ddm_config.domain))
		{
			printf("WARNING: Unable to send vendor specific control message as requested domain %s is not available\n Exiting \n", ddm_config.domain);
			goto cleanup;
		}
	}

#endif

	// All control & status & subscribe registration to device is done.
	if(client.message_direction == VENDOR_SPECIFIC_TX)
	{
		result = send_vendor_specific_control_msg_over_conmon(&client);

		// send control command for tx testing
		if (result != AUD_SUCCESS)
		{
			printf ("Error sending control message (request): %s\n", aud_error_message(result, errbuf));
			goto cleanup;
		}

		// For tx side, 5 sec timeout
		control_timeout.tv_sec = 5;
		control_timeout.tv_usec = 0;
	}
	else
	{
		// For rx side, 20 sec timeout
		control_timeout.tv_sec = 20;
		control_timeout.tv_usec = 0;
	}

	// wait for expected vendor specific conmon control response messages up to 5 sec
	if(wait_for_vendor_conmon_response(&client, &control_timeout) != AUD_SUCCESS)
	{
		printf("Vendor specific conmon control message timed out. - request id:0x%p\n", req_id);
	}
	else printf("Vendor specific conmon control message successful - request id:0x%p\n", req_id);

cleanup:
	if (client.conmon)
	{
		conmon_client_delete(client.conmon);
			// Note: this calls conmon_client_disconnect as part of clean-up
	}
	if (dapi)
	{
		dapi_delete(dapi);
	}
	return result;
}

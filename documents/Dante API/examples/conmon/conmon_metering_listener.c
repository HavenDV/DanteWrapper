/*
 * File     : $RCSfile$
 * Created  : September 2008
 * Updated  : $Date$
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : An example conmon client capable of controlling a remote device
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#include "dapi_utils_domains.h"
#include "conmon_examples.h"
#include <signal.h>

#ifdef WIN32
#else
#include <libgen.h>
#endif

//----------
// Types and Constants

static char * g_progname;
typedef struct conmon_info conmon_info_t;

struct conmon_info
{
	// conmon connectivity
	aud_env_t * env;
	dapi_t * dapi;
	dante_runtime_t * runtime;
	dante_domain_handler_t * handler;

	conmon_client_t * client;

	// options
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config;
#endif
	aud_bool_t allow_metering_failure;
	uint16_t metering_port;
	const char * device;
	aud_bool_t print_raw_bytes;
	//runtime
	conmon_client_state_t curr_state;

	conmon_client_request_id_t _req_id;
};

//----------
// Local function prototypes

AUD_INLINE const char *
pname(void)
{
#ifdef WIN32
	return g_progname;
#else
	return basename(g_progname);
#endif
}

//----------
// Callback prototypes

static conmon_client_connection_state_changed_fn conmon_cb_connection_state_changed;
static conmon_client_handle_networks_changed_fn handle_networks_changed;
static conmon_client_handle_subscriptions_changed_fn handle_subscriptions_changed;
static conmon_client_event_fn handle_event;
static conmon_client_handle_monitoring_message_fn handle_metering_message;

static ddh_change_event_fn conmon_cb_handle_ddh_changes;
//----------------------------------------------------------
// Synchronous communications for simplicity
//----------------------------------------------------------




static conmon_client_response_fn handle_response;

static void
handle_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
) {
	// TODO: should check request id codes to ensure correct matching
	AUD_UNUSED(client);
	aud_errbuf_t errbuf;
	printf ("Got response for request %p: %s\n",
		request_id, aud_error_message(result, errbuf));

}


//----------
// Local functions

static void
timestamp_event(void)
{
	aud_ctime_buf_t buf;
	printf("#EVENT %s: ", aud_utime_ctime_no_newline(NULL, buf));
}

static void
timestamp_error(void)
{
	aud_ctime_buf_t buf;
	fprintf(stderr, "#ERROR %s: ", aud_utime_ctime_no_newline(NULL, buf));
}

static aud_error_t
shutdown_conmon(conmon_info_t * cm)
{
	if (cm->client)
	{
		conmon_client_delete(cm->client);
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

static aud_error_t
setup_remote(conmon_info_t * cm)
{
	char const * name = conmon_client_get_dante_device_name(cm->client);

	if (!(cm->device && cm->device[0] && strcmp(cm->device, name)))
	{
		// nothing to do
		return AUD_SUCCESS;
	}

	aud_error_t result = AUD_SUCCESS;
	aud_errbuf_t ebuf;
	
	result = conmon_client_register_monitoring_messages(cm->client,
		&handle_response, NULL,
		CONMON_CHANNEL_TYPE_METERING, CONMON_CHANNEL_DIRECTION_RX,
		handle_metering_message);
	if (result == AUD_SUCCESS)
	{
		printf("Registering for rx metering messages\n");
	}
	else
	{
		timestamp_error();
		fprintf(stderr,
			"%s: failed to register for rx metering messages(request): %s (%d)\n"
			, pname()
			, aud_error_message(result, ebuf), result
			);
		return result;

	}
	result = conmon_client_subscribe(cm->client,
		&handle_response, &cm->_req_id,
		CONMON_CHANNEL_TYPE_METERING, cm->device);
	if (result == AUD_SUCCESS)
	{
		printf("Subscribing, request id is %p\n", cm->_req_id);
	}
	else
	{
		printf("Error subscribing to metering channel(request): %s\n",
			aud_error_message(result, ebuf));
		return result;
	}

	return AUD_SUCCESS;
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
setup_local(conmon_info_t * cm)
{
	char const * name = conmon_client_get_dante_device_name(cm->client);

	if (cm->device && cm->device[0] && strcmp(cm->device, name))
	{
		// nothing to do
		return AUD_SUCCESS;
	}

	aud_errbuf_t ebuf;
	aud_error_t result = conmon_client_register_monitoring_messages(cm->client,
		&handle_response, &cm->_req_id,
		CONMON_CHANNEL_TYPE_METERING, CONMON_CHANNEL_DIRECTION_TX,
		handle_metering_message);
	if (result != AUD_SUCCESS)
	{
		printf("Error registering for tx metering messages(request): %s\n",
			aud_error_message(result, ebuf));
	}
	return result;
}

static aud_error_t
setup_conmon(conmon_info_t * cm)
{
	conmon_client_config_t * config = NULL;
	aud_error_t result;
	aud_errbuf_t ebuf;

	cm->env = NULL;
	cm->client = NULL;

	result = dapi_new(&cm->dapi);
	if (result != AUD_SUCCESS)
	{
		timestamp_error();
		fprintf(stderr,
			"%s: conmon failed to initialise environment: %s (%d)\n"
			, pname()
			, aud_error_message(result, ebuf), result
			);
		return result;
	}

	cm->env = dapi_get_env(cm->dapi);
	cm->runtime = dapi_get_runtime(cm->dapi);
	cm->handler = dapi_get_domain_handler(cm->dapi);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&cm->ddm_config, cm->handler, cm->runtime, NULL);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(cm->handler);
	dante_domain_handler_set_context(cm->handler, cm);
	dante_domain_handler_set_event_fn(cm->handler, conmon_cb_handle_ddh_changes);

	config = conmon_client_config_new("metering_listener");
	if (!config)
	{
		result = AUD_ERR_NOMEMORY;
		printf("Error initialising conmon client config: %s\n",
			aud_error_message(result, ebuf));
		goto cleanup;
	}
	conmon_client_config_set_metering_channel_enabled(config, AUD_TRUE);
	if (cm->metering_port)
	{
		conmon_client_config_set_metering_channel_port(config, cm->metering_port);
	}
	if (cm->allow_metering_failure)
	{
		conmon_client_config_allow_metering_channel_init_failure(config, AUD_TRUE);
	}
	result = conmon_client_new_dapi(cm->dapi, config, &cm->client);
	if (cm->client == NULL)
	{
		printf("Error creating client: %s\n", aud_error_message(result, ebuf));
		goto cleanup;
	}

	if (!conmon_client_is_metering_channel_active(cm->client))
	{
		printf("Metering channel configuration failed\n");
	}

	conmon_client_set_context(cm->client, cm);
	conmon_client_set_connection_state_changed_callback(cm->client, conmon_cb_connection_state_changed);
	conmon_client_set_networks_changed_callback(cm->client, handle_networks_changed);
	conmon_client_set_subscriptions_changed_callback(cm->client, handle_subscriptions_changed);
	conmon_client_set_event_callback(cm->client, handle_event);

	result = conmon_client_auto_connect(cm->client);
	if (result != AUD_SUCCESS)
	{
		timestamp_error();
		fprintf(stderr,
			"%s: conmon failed to initiate connection: %s (%d)\n"
			, pname()
			, aud_error_message(result, ebuf), result
			);
		goto cleanup;
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (!conmon_client_in_config_domain(cm))
	{
		printf("WARNING: Unable to Register subscriptions as requested domain is not available\n");
		return AUD_SUCCESS;
	}
#endif
	// See if we are ready to set anything up yet...
	if (conmon_client_is_local_ready(cm->client))
	{
		setup_local(cm);
	}
	if (conmon_client_is_remote_ready(cm->client))
	{
		setup_remote(cm);
	}

	return AUD_SUCCESS;
cleanup:
	shutdown_conmon(cm);

	return result;
}

//----------------------------------------------------------
// Message handler
//----------------------------------------------------------


/**
 * Handle an incoming metering message. This function simply
 * checks that the message is valid (ie. it has the right vendor ID)
 * and then prints out the current levels to stdout
 */

static void
handle_metering_message
(
	conmon_client_t * client,
	conmon_channel_type_t type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
) {
	const conmon_metering_message_peak_t * tx_peaks = NULL;
	const conmon_metering_message_peak_t * rx_peaks = NULL;
	conmon_metering_message_version_t version;
	uint16_t num_txchannels;
	uint16_t num_rxchannels;
	conmon_instance_id_t instance_id;
	aud_error_t result;
	aud_errbuf_t errbuf;
	
	char buf[1024*2];	// Buffer size is for maximum 256 x 256. Need to increase for 512 x 512.

	const char * name;
	conmon_info_t * info = conmon_client_context(client);

	AUD_UNUSED(channel_direction);
	AUD_UNUSED(type);
	if (!conmon_vendor_id_equals(conmon_message_head_get_vendor_id(head), CONMON_VENDOR_ID_AUDINATE))
	{
		return;
	}
	
	conmon_message_head_get_instance_id(head, &instance_id);
	name = conmon_client_device_name_for_instance_id(client, &instance_id);


	result = conmon_metering_message_parse(body, &version, &num_txchannels, &num_rxchannels);
	if (result != AUD_SUCCESS)
	{
		printf("Error parsing metering message header: %s\n", aud_error_message(result, errbuf));
		return;
	}
	printf("version=%u ntx=%u nrx=%u\n", version, num_txchannels, num_rxchannels);
	tx_peaks = conmon_metering_message_get_peaks_const(body, CONMON_CHANNEL_DIRECTION_TX);
	rx_peaks = conmon_metering_message_get_peaks_const(body, CONMON_CHANNEL_DIRECTION_RX);
	
	// Print the raw packet
	if (info->print_raw_bytes)
	{
		unsigned int i, meter_head, size = conmon_message_head_get_body_size(head);

		if(version < 3)
			meter_head = sizeof(conmon_metering_message_v1_v2_t);
		else meter_head = sizeof(conmon_metering_message_v3_t);

		printf("PAYLOAD: size=%u ntx=%u nrx=%u\n", size, num_txchannels, num_rxchannels);
		for (i = 0; i < size; i++) 
		{
			if (((i-meter_head) % 16) == 0)
			{
				printf("\n");
			}
			printf(" %02X", body->data[i]);
		}
		printf("\n");
	}
	else
	{
		printf("RECV METERING(%s.%d): TX=%s\n",
			(name ? name : "???"), conmon_message_head_get_seqnum(head),
			conmon_example_metering_peaks_to_string(tx_peaks, num_txchannels, buf, sizeof(buf)));
		printf("RECV METERING(%s,%d): RX=%s\n",
			(name ? name : "???"), conmon_message_head_get_seqnum(head),
			conmon_example_metering_peaks_to_string(rx_peaks, num_rxchannels, buf, sizeof(buf)));
	}
	fflush(stdout);
}

//----------------------------------------------------------
// System callbacks
//----------------------------------------------------------

static void
handle_networks_changed
(
	conmon_client_t * client
) {
	char buf[1024];
	const conmon_networks_t * networks = conmon_client_get_networks(client);
	timestamp_event();
	conmon_example_networks_to_string(networks, buf, 1024);
	printf("NETWORKS CHANGED: %s\n", buf);
}

static void
client_subscription_to_string
(
	const conmon_client_subscription_t * sub,
	char * buf,
	size_t len
) {
	if (sub)
	{
		char id_buf[64];
		const char * channel = conmon_example_channel_type_to_string(conmon_client_subscription_get_channel_type(sub));
		const char * device = conmon_client_subscription_get_device_name(sub);
		const char * status = conmon_example_rxstatus_to_string(conmon_client_subscription_get_rxstatus(sub));
		conmon_example_instance_id_to_string(conmon_client_subscription_get_instance_id(sub), id_buf, 64);
		SNPRINTF(buf, len, "%s@%s: status=%s instance_id=%s", channel, device, status, id_buf);
	}
	else
	{
		SNPRINTF(buf, len, "[NULL]");
	}
	
}

static void
handle_subscriptions_changed
(
	conmon_client_t * client,
	unsigned int num_changes,
	const conmon_client_subscription_t * const * changes
) {
	char buf[1024];
	unsigned int i;
	AUD_UNUSED(client);
	printf("SUBSCRIPTION CHANGES:\n");
	for (i = 0; i < num_changes; i++)
	{
		client_subscription_to_string(changes[i], buf, 1024);
		printf("%d: %s\n", i, buf);
	}
}

static void handle_event(const conmon_client_event_t * ev)
{
	conmon_client_t * client = conmon_client_event_get_client(ev);
	conmon_info_t * info = conmon_client_context(client);

	conmon_client_event_flags_t flags = conmon_client_event_get_flags(ev);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (!conmon_client_in_config_domain(info))
	{
		return;
	}
#endif

	if (flags & CONMON_CLIENT_EVENT_FLAG__LOCAL_READY_CHANGED)
	{
		if (conmon_client_is_local_ready(client))
		{
			setup_local(info);
		}
	}

	if (flags & CONMON_CLIENT_EVENT_FLAG__REMOTE_READY_CHANGED)
	{
		if (conmon_client_is_remote_ready(client))
		{
			setup_remote(info);
		}
	}
}

static void
conmon_cb_connection_state_changed
(
conmon_client_t * client
)
{
	conmon_info_t * info = conmon_client_context(client);
	conmon_client_state_t state = conmon_client_state(client);

	//assert(info);
	if (!info)
	{
		timestamp_error();
		fprintf(stderr, "Invalid program state - no info in conmon_cb_connection_state_changed\n");
		return;
	}

	if (state == CONMON_CLIENT_CONNECTED)
	{

		timestamp_event();
		puts("Conmon connection successful");

		if (info->curr_state == CONMON_CLIENT_CONNECTED)
		{
			timestamp_error();
			fprintf(stderr, "Already connected\n");
			return;
		}
		info->curr_state = state;
		// All setup now triggered off conmon_client_event handler
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

//---------------------------------------------------

//----------------------------------------------------------
// Main
//----------------------------------------------------------

static int
usage(const char * bin)
{
	const char * name = pname();

	if (bin)
	{
		fprintf(stderr, "%s: %s\n", name, bin);
	}
	fprintf(stderr,
		"Usage: %s [Options]\n"
		, name
		);
	fprintf(stderr, "  Listen to metering messages from a given device\n");
	fprintf(stderr, "  If no transmitter specified then listen for local metering messages\n");
	fprintf(stderr, "  -f allow metering channel configuration to fail (useful for debugging)\n");
	fprintf(stderr, "  -p=PORT: specify the client's metering port as PORT\n");
	fprintf(stderr, "  -d=DEVICE: listen to metering from device DEVICE\n");
	fprintf(stderr, "  -raw: print raw packet data\n");
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


/**
 * The main function. The command line must provide the
 * name of the conmon device that will be controlled
 */
int main(int argc, char * argv[])
{
	aud_error_t result;
	conmon_info_t info = { 0 };
	g_progname = argv[0];
	int a;
	aud_bool_t running;

	printf("conmon metering listener, build timestamp %s %s\n", __DATE__, __TIME__);

	for (a = 1; a < argc; a++)
	{
		const char * arg = argv[a];

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		if (dapi_utils_ddm_config_parse_one(&info.ddm_config, arg, &result))
		{
			if (result != AUD_SUCCESS)
			{
				return usage("Invalid domain argument");
			}
			continue;
		}
		else
#endif
		if (!strncmp(arg, "-d=", 3) && strlen(arg) > 3)
		{
			info.device = arg + 3;
		}
		else if (!strncmp(arg, "-p=", 3) && strlen(arg) > 3)
		{
			info.metering_port = (uint16_t)atoi(arg + 3);
		}
		else if (!strcmp(arg, "-f"))
		{
			info.allow_metering_failure = AUD_TRUE;
		}
		else if (!strcmp(arg, "-raw"))
		{
			info.print_raw_bytes = AUD_TRUE;
		}
		else
		{
			usage(argv[0]);
			exit(1);
		}
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	result = setup_conmon(&info);
	if (result != AUD_SUCCESS)
	{
		return 1;
	}

	running = AUD_TRUE;
	while (running)
	{
		result = dapi_utils_step(info.runtime, AUD_SOCKET_INVALID, NULL);
		if (result != AUD_SUCCESS)
		{
			timestamp_error();
			fprintf(stderr,
				"event loop failed: %s (%d)\n"
				, aud_error_get_name(result)
				, (int)result
				);
			break;
		}
	}

	shutdown_conmon(&info);

	return 0;
}

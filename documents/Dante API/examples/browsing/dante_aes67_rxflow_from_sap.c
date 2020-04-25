/*
 * File     : $RCSfile$
 * Created  : Apr 2015
 * Updated  : $Date$
 * Author   : Varuni Witana
 * Synopsis : Example application creating an AES67 Rx flow from a SAP announcement
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#include "audinate/dante_api.h"
#include "dapi_utils.h"

#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <ctype.h>

#define DR_TEST_REQUEST_DESCRIPTION_LENGTH 64
#define DR_TEST_MAX_REQUESTS 128
#define DR_TEST_MAX_INTERFACES 2


#ifdef WIN32
#define SNPRINTF _snprintf
#include <conio.h>
#else
#include <stdlib.h>
#define SNPRINTF snprintf
#endif

#define MAX_RESOLVES 16
#define MAX_SOCKETS (MAX_RESOLVES+2) // need space for device and channel browsing

const char * g_progname = "";

static aud_bool_t g_running = AUD_TRUE;
// Static buffers: save  stack memory by sharing these buffers
aud_errbuf_t g_test_errbuf;
char g_input_buf[BUFSIZ];

static db_browse_node_changed_fn aes67_test_node_changed;
static dr_device_response_fn aes67_test_on_response;

void sig_handler(int sig);

void sig_handler(int sig)
{
	AUD_UNUSED(sig);
	signal(SIGINT, sig_handler);
	g_running = AUD_FALSE;
}

typedef struct dr_test_request
{
	dante_request_id_t id;
	char description[DR_TEST_REQUEST_DESCRIPTION_LENGTH];
} dr_test_request_t;

typedef struct aes67_test_device aes67_device_info_t;

typedef struct aes67_test
{
	dapi_t * dapi;
	aud_env_t * env;
	dante_runtime_t * runtime;

	db_browse_t * browse;
	dr_devices_t * devices;
	dr_test_request_t requests[DR_TEST_MAX_REQUESTS];

	uint16_t txlabels_buflen;
	dr_txlabel_t * txlabels_buf;

	// Command line params
	struct aes67_test_target
	{
		uint16_t flow_id;
		const char * device_name;
		const char * filename;
		const char * tx_addr;
		struct in_addr tx_inaddr;
	} source, target, *curr_target;

	// for interface-aware / redundant name-based connection
	unsigned int num_local_interfaces;
	aud_interface_identifier_t local_interfaces[DR_TEST_MAX_INTERFACES];

	// Runtime state
	struct
	{
		struct aes67_test_device
		{
			struct aes67_test * test;
			dr_device_t * device;
			aud_bool_t device_is_ready;
		} source, target;
	} device_info;
	const dante_sdp_descriptor_t * sdp_descriptor;

	dante_sdp_descriptor_ref_t * sdp_descriptor_ref;
} aes67_test_t;


//--------------------

/*
	After changing runtime state, call this to decide what to do next
 */
static void
aes67_test_process
(
	aes67_test_t * test
);


//--------------------
// Saving and loading to/from file

#define SDP_BUFLEN 256
typedef uint8_t sdpbuf_t[SDP_BUFLEN];
typedef char sdpcharbuf_t[SDP_BUFLEN * 2 + 1];

static char
to_hex(uint8_t n)
{
	if (n < 10)
	{
		return '0' + n;
	}
	else
	{
		return 'a' + (n-10);
	}
}

static uint8_t
from_hex(char ch)
{
	if (ch <= '9')
	{
		return ch - '0';
	}
	else if ('a' <= ch && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	else
	{
		return ch - 'A' + 10;
	}
}


static void
sdp_to_file
(
	const dante_sdp_descriptor_t * d,
	const char * filename
)
{
	aud_error_t result;

	assert(d);
	assert(filename);

	// convert to buffer
	sdpbuf_t buf;
	size_t buflen = sizeof(buf);

	result = dante_sdp_descriptor_serialise(d, buf, &buflen);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Serialise failed: %d\n", result);
		exit(1);
	}


	// To make stdout work nicely, we'll convert to hex characters
	sdpcharbuf_t cbuf;
	size_t i;
	for (i = 0; i < buflen; i++)
	{
		cbuf[2*i] = to_hex(buf[i] >> 4);
		cbuf[2*i + 1] = to_hex(buf[i] & 0xf);
	}
	cbuf[2*i] = 0;

	FILE * fp;

	if (! filename[0] || strcmp(filename,"-") == 0)
	{
		fp = stdout;
		filename = "<stdout>";
	}
	else
	{
		fp = fopen(filename, "w");
		if (! fp)
		{
			perror("Failed to open file for writing");
			exit(1);
		}
	}

	fputs(cbuf, fp);
	fputc('\n', fp);
	fclose(fp);
	printf("Wrote %u bytes to %s\n", (unsigned) i, filename);
}


static int
validate_char(char ch, size_t pos)
{
	if (! ch)
		return 0;
	if (isxdigit(ch))
		return 1;
	if (isspace(ch))
		return 0;
	fprintf(stderr, "Invalid character '%c' at position %u\n"
		, ch, (unsigned) pos
	);
	exit(1);
}

static void
sdp_from_file(
	dante_sdp_descriptor_ref_t * r,
	const char * filename
)
{
	aud_error_t result;

	assert(r);
	assert(filename);

	FILE * fp;

	if (! filename[0] || strcmp(filename,"-") == 0)
	{
		fp = stdin;
		filename = "<stdin>";
	}
	else
	{
		fp = fopen(filename, "r");
		if (! fp)
		{
			perror("Failed to open file for reading");
			exit(1);
		}
	}

	sdpcharbuf_t cbuf;
	{
		char * read_result = fgets(cbuf, sizeof(cbuf), fp);
		if (! read_result)
		{
			if (ferror(fp))
			{
				perror("Reading failed");
				exit(1);
			}
		}

		size_t len = strlen(cbuf);
		if (! len)
		{
			fprintf(stderr, "Error reading %s\n", filename);
			exit(1);
		}
		if (cbuf[len - 1] != '\n')
		{
			fprintf(stderr,
				"Failed to read entire file %s (read %u characters)\n"
				, filename
				, (unsigned) len
			);
			exit(1);
		}
	}

	sdpbuf_t buf;
	size_t i = 0;
	size_t buflen = 0;
	for(;;)
	{
		uint8_t val;
		char ch = cbuf[i];
		if (! validate_char(ch, i))
			break;
		val = from_hex(ch) << 4;
		i++;

		ch = cbuf[i];
		if (! validate_char(ch, i))
		{
			fprintf(stderr, "Unexpected end of input at position %u\n"
				, (unsigned) i
			);
			exit(1);
		}
		val |= from_hex(ch);
		i++;

		if (buflen < sizeof(buf))
		{
			buf[buflen++] = val;
		}
		else
		{
			fprintf(stderr, "Input from %s exceeds buffer length (%u bytes)"
				, filename, (unsigned) sizeof(buf)
			);
			exit(1);
		}
	}

	result = dante_sdp_descriptor_deserialise(r, buf, buflen);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Deserialise failed: %u", result);
		exit(1);
	}
}


//--------------------

static dr_test_request_t *
aes67_test_allocate_request
(
	aes67_test_t * test,
	const char * description
) {
	unsigned int i;
	for (i = 0; i < DR_TEST_MAX_REQUESTS; i++)
	{
		if (test->requests[i].id == DANTE_NULL_REQUEST_ID)
		{
			aud_strlcpy(test->requests[i].description, description ? description : "", DR_TEST_REQUEST_DESCRIPTION_LENGTH);
			return test->requests + i;
		}
	}
	printf("error allocating request '%s': no more requests\n", description);
	return NULL;
}

static void
aes67_test_request_release
(
	dr_test_request_t * request
) {
	request->id = DANTE_NULL_REQUEST_ID;
	request->description[0] = '\0';
}

void
aes67_test_on_response
(
	dr_device_t * device,
	dante_request_id_t request_id,
	aud_error_t result
) {
	unsigned int i;
	aes67_device_info_t * device_info =
		(aes67_device_info_t *) dr_device_get_context(device);
	aes67_test_t * test = device_info->test;
	AUD_UNUSED(result);

	for (i = 0; i < DR_TEST_MAX_REQUESTS; i++)
	{
		if (test->requests[i].id == request_id)
		{
			aes67_test_request_release(test->requests+i);
			break;
		}
	}

	dr_device_component_t c;
	for (c = 0; c < DR_DEVICE_COMPONENT_COUNT; c++)
	{
		if (dr_device_is_component_stale(device, c))
		{
			device_info->device_is_ready = AUD_FALSE;
			return;
		}
	}

	device_info->device_is_ready = AUD_TRUE;
	aes67_test_process(test);
}

static aud_error_t
aes67_test_query_capabilities
(
	aes67_test_t * test,
	dr_device_t * device
) {
	aud_error_t result;
	dr_test_request_t * request = aes67_test_allocate_request(test, "QueryCapabilities");
	if (!request)
	{
		return AUD_ERR_NOBUFS;
	}

	result = dr_device_query_capabilities(device, &aes67_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		printf("Error sending query capabilities: %s\n",
			dr_error_message(result, g_test_errbuf));
		aes67_test_request_release(request);
		return result;
	}
	return result;
}

static aud_error_t
aes67_test_update_device
(
	aes67_device_info_t * device_info
)
{
	aud_error_t result;
	dr_device_component_t c;
	dr_device_t * device = device_info->device;
	for (c = 0; c < DR_DEVICE_COMPONENT_COUNT; c++)
	{
		dr_test_request_t * request;
		if (!dr_device_is_component_stale(device, c))
		{
			continue;
		}

		request = aes67_test_allocate_request(device_info->test, NULL);
		if (!request)
		{
			return AUD_ERR_NOBUFS;
		}
		SNPRINTF(request->description, DR_TEST_REQUEST_DESCRIPTION_LENGTH, "Update %s", dr_device_component_to_string(c));

		result = dr_device_update_component(device, &aes67_test_on_response, &request->id, c);
		if (result != AUD_SUCCESS)
		{
			printf("Error sending update %s: %s\n",
				dr_device_component_to_string(c), dr_error_message(result, g_test_errbuf));
			aes67_test_request_release(request);
			return result;
		}
	}
	return AUD_SUCCESS;
}





static const char * aes67_test_print_sdp_stream_dir
(
	dante_sdp_stream_dir_t dir
)
{
	switch(dir)
	{
		case DANTE_SDP_DIR__RECV_ONLY: return "SDP_STREAM_DIRECTION_RECV_ONLY";
		case DANTE_SDP_DIR__SEND_RECV: return "SDP_STREAM_DIRECTION_SEND_RECV";
		case DANTE_SDP_DIR__SEND_ONLY: return "SDP_STREAM_DIRECTION_SEND_ONLY";
		default : return "SDP_STREAM_DIRECTION_UNKNOWN";
	}
}

static void
aes67_test_create_rxflow_from_sdp
(
	aes67_test_t * test,
	const dante_sdp_descriptor_t *sdp_desc
)
{
	dr_device_t * device = test->device_info.target.device;
	assert(device);

	const dante_clock_grandmaster_uuid_t *gmid;
	const dante_clock_subdomain_name_t *sub_domain;

	uint16_t i;
	aud_error_t result;
	dr_rxflow_config_t * config = NULL;

	uint16_t num_slots;
	dr_test_request_t * request;
	struct in_addr a;
	char addrbuf[128];

	num_slots = dante_sdp_get_stream_num_chans(sdp_desc);


	// SDP body print
	a.s_addr = dante_sdp_get_origin_addr(sdp_desc);
	printf("SDP origin username %s, session name:%s, session id:%llx, session originator address:%s\n",
		dante_sdp_get_origin_username(sdp_desc),
		dante_sdp_get_session_name(sdp_desc),
		(unsigned long long) dante_sdp_get_session_id(sdp_desc),
		aud_inet_ntoa(a.s_addr, addrbuf,sizeof(addrbuf))
		);
	a.s_addr = dante_sdp_get_session_conn_addr(sdp_desc);
	printf("SDP RTP stream addr\t\t %s:%d, media_clock_offset:%u, media payload type:%d\n",
		aud_inet_ntoa(a.s_addr, addrbuf, sizeof(addrbuf)),
		dante_sdp_stream_get_port(sdp_desc),
		dante_sdp_get_media_clock_offset(sdp_desc),
		dante_sdp_get_stream_payload_type(sdp_desc)
		);

	gmid = dante_sdp_get_network_clock_ref(sdp_desc);
	sub_domain = dante_sdp_get_network_clock_ref_domain(sdp_desc);

	printf("SDP RTP session GMID:Domain \t %02x:%02x:%02x:%02x:%02x:%02x:0:0:%s\n",
			gmid->data[0]&0xff, gmid->data[1]&0xff, gmid->data[2]&0xff, gmid->data[3]&0xff, gmid->data[4]&0xff, gmid->data[5]&0xff,
			(sub_domain ? sub_domain->data : "NULL")
			);

	printf("SDP RTP sample rate %d, encoding %d, num_ch %d\n", dante_sdp_get_stream_sample_rate(sdp_desc), dante_sdp_get_stream_encoding(sdp_desc), dante_sdp_get_stream_num_chans(sdp_desc));
	printf("SDP RTP stream direction %s\n", aes67_test_print_sdp_stream_dir(dante_sdp_get_stream_dir(sdp_desc)));
	printf("\n");

	printf("Creating Rx flow with %d slots\n", dante_sdp_get_stream_num_chans(sdp_desc));

	result = dr_rxflow_config_new_aes67_multicast(device, test->target.flow_id, num_slots, &config);
	if (result != AUD_SUCCESS)
	{
		printf("Error creating flow config object: %s\n", dr_error_message(result, g_test_errbuf));
		exit(1);
	}

	// One-to-one channel to slot mapping
	for (i = 0; i < num_slots; i++)
	{
		dr_rxchannel_t * rx = dr_device_rxchannel_with_id(device, (dante_id_t) i+1);
		if (rx)
		{
			dr_rxflow_config_add_aes67_channel(config, rx, i);
			if (result != AUD_SUCCESS)
			{
				printf("Error setting rxflow config slot: %s\n",
					dr_error_message(result, g_test_errbuf));
				dr_rxflow_config_discard(config);
				exit(1);
			}
		}
	}

	result = dr_rxflow_config_set_aes67_params_from_sap(config, sdp_desc);
	if (result != AUD_SUCCESS)
	{
		printf("Error setting aes67 params: %s\n", dr_error_message(result, g_test_errbuf));
		exit(1);
	}

	request = aes67_test_allocate_request(test, "AddMulticastAES67Flow");
	if (!request)
	{
		exit(1);
	}
	result = dr_rxflow_config_commit(config, aes67_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		printf("Error sending multicast aes67 flow  create request: %s\n", dr_error_message(result, g_test_errbuf));
		aes67_test_request_release(request);
		exit(1);
	}

	const uint8_t * addr_bytes = (const uint8_t *) &a.s_addr;
	printf("RX Flow receiving from address %u.%u.%u.%u successfully created\n"
		, addr_bytes[0]
		, addr_bytes[1]
		, addr_bytes[2]
		, addr_bytes[3]
	);
	exit(0);
}


static void
aes67_test_descriptor_from_device
(
	aes67_test_t * test
)
{
	aud_error_t result;

	assert(test);
	assert(test->device_info.source.device);
	assert(test->device_info.source.device_is_ready);

	dr_rxflow_t * flow;

	result = dr_device_rxflow_with_id(
		test->device_info.source.device,
		test->source.flow_id,
		&flow
	);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Unknown flow %u on device '%s'"
			, (unsigned) test->source.flow_id
			, test->source.device_name
		);
		exit(1);
	}

	test->sdp_descriptor_ref = dante_sdp_descriptor_alloc(NULL);
	result = dr_rxflow_synthesise_sdp_descriptor(flow, test->sdp_descriptor_ref);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Failed to extract descriptor from flow\n");
		exit(1);
	}
	test->sdp_descriptor = dante_sdp_descriptor_from_ref(test->sdp_descriptor_ref);

	aes67_test_process(test);
}


static void
aes67_test_process_sdp_advert
(
	aes67_test_t * test,
	const db_browse_aes67_t * aes67_device
)
{
	const dante_sdp_descriptor_t *sdp_desc;

	db_browse_aes67_get_sdp_descriptor(aes67_device, &sdp_desc);
	if(dante_sdp_get_session_conn_addr(sdp_desc) == test->source.tx_inaddr.s_addr)
	{
		if (test->target.filename)
		{
			sdp_to_file(sdp_desc, test->target.filename);
			exit(0);
		}
		else
		{
			// We want to make sure our copy of the descriptor doesn't go out
			// of scope, so create a descriptor ref to back it
			test->sdp_descriptor_ref = dante_sdp_descriptor_alloc(sdp_desc);
			test->sdp_descriptor = dante_sdp_descriptor_from_ref(test->sdp_descriptor_ref);
			aes67_test_process(test);
		}
	}
}


static void
aes67_test_node_changed
(
	db_browse_t * browse,
	const db_node_t * node,
	db_node_change_t node_change
) {
	aes67_test_t * test = (aes67_test_t *) db_browse_get_context(browse);

	if (node->type == DB_NODE_TYPE_SDP)
	{
		aes67_test_process_sdp_advert(test, node->_.aes67);
	}
}


static void
aes67_test_process
(
	aes67_test_t * test
)
{
	if (test->device_info.source.device)
	{
		if (test->device_info.source.device_is_ready)
		{
			if (! test->sdp_descriptor)
			{
				aes67_test_descriptor_from_device(test);
			}
		}
	}

	if (! test->sdp_descriptor)
		return;

	if (test->device_info.target.device)
	{
		if (test->device_info.target.device_is_ready)
		{
			aes67_test_create_rxflow_from_sdp(test, test->sdp_descriptor);
		}
	}
	else if (test->target.filename)
	{
		sdp_to_file(test->sdp_descriptor, test->target.filename);
		exit(0);
	}
}


//--------------------
// Main

static aud_error_t
aes67_test_main_loop
(
	aes67_test_t * test
) {

	if (test->browse)
	{
		printf("Running main loop waiting for SAP announcement for Tx flow address %s -  this could take upto 30 seconds\n"
			, test->source.tx_addr
		);
	}

#ifdef  _WIN32
	// set to line buffered mode.
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
#else
	dante_sockets_t select_sockets;
#endif

	while(g_running)
	{

#ifdef _WIN32
		dapi_utils_step(test->runtime, AUD_SOCKET_INVALID, NULL);
#else
		dapi_utils_step(test->runtime, 0, &select_sockets);
#endif


	}
	return AUD_SUCCESS;
}

static void usage(void)
{
	printf("Usage: %s [-ii=interface_index] source target\n", g_progname);
	fputs(
		"  Source: (SDP advert)\n"
		"    -a=n.n.n.n.       : browse for multicast flow with this destination address\n"
		"    -d=flow_id@device : copy SDP information from existing AES67 RX flow on device\n"
		"    -f=filename       : load SDP advert from file\n"
		"  Target:\n"
		"    -d=flow_id@device : create new AES67 RX flow on device\n"
		"    -f=filanme.       : save SDP advert to file\n"
		"  OPTIONS:\n"
		"    -ii=index         : operate on network with interface index\n"
		"                        required for browsing, otherwise optional\n"
		, stdout
	);
	printf("\n");
}

static void
aes67_test_on_device_state_changed
(
	aes67_device_info_t * device_info
)
{
	assert(device_info);
	assert(device_info->test);
	assert(device_info->device);

	dr_device_t * device = device_info->device;
	dr_device_state_t state = dr_device_get_state(device);

	const char * name;
	if (dr_device_is_local(device))
	{
		name = "local";
	}
	else
	{
		name = dr_device_get_connect_name(device);
	}
	printf("Device '%s' state is now '%s'\n"
		, name
		, dr_device_state_to_string(state)
	);

	switch (state)
	{
	case DR_DEVICE_STATE_RESOLVED:
		// query capabilities
		aes67_test_query_capabilities(device_info->test, device);
		return;

	case DR_DEVICE_STATE_ACTIVE:
		{
			// update all components when we enter the active state, unless we have a strange status
			dr_device_status_flags_t status_flags;
			aud_error_t result = dr_device_get_status_flags(device, &status_flags);
			if (result != AUD_SUCCESS)
			{
				printf("Error getting status flags: %s\n", dr_error_message(result, g_test_errbuf));
				return;
			}
			if (status_flags)
			{
				return;
			}
			aes67_test_update_device(device_info);
			return;
		}
	case DR_DEVICE_STATE_ERROR:
		{
			aud_error_t error_state_error = dr_device_get_error_state_error(device);
			const char * error_state_action = dr_device_get_error_state_action(device);

			printf("device has entered the ERROR state: error=%s, action='%s'\n",
				dr_error_message(error_state_error, g_test_errbuf), (error_state_action ? error_state_action : ""));
			return;
		}
	default:
		// nothing to do
		;
	}
}


static void
aes67_test_on_device_changed
(
	dr_device_t * device,
	dr_device_change_flags_t change_flags
) {
	aes67_device_info_t * device_info =
		(aes67_device_info_t *) dr_device_get_context(device);

	if (change_flags & DR_DEVICE_CHANGE_FLAG_STATE)
	{
		aes67_test_on_device_state_changed(device_info);
	}
}

static aud_error_t
aes67_test_open_device
(
	aes67_test_t * test,
	aes67_device_info_t * device_info,
	const char * device_name
)
{
	assert(test);
	assert(device_name);
	assert(device_info);

	aud_error_t result = AUD_SUCCESS;
	dr_device_open_t * config = NULL;
	unsigned i;

	if (device_name[0])
	{
		config = dr_device_open_config_new(device_name);
		if (! config)
		{
			result = AUD_ERR_NOMEMORY;
			goto l__error;
		}

		if (test->num_local_interfaces)
		{
			printf("Opening connection to remote device %s using %d interfaces\n",
				device_name, test->num_local_interfaces);

			for (i = 0; i < test->num_local_interfaces; i++)
			{
				if (test->local_interfaces[i].flags == AUD_INTERFACE_IDENTIFIER_FLAG_NAME)
				{
					dr_device_open_config_enable_interface_by_name(
						config, i, test->local_interfaces[i].name
					);
				}
				else if (test->local_interfaces[i].flags == AUD_INTERFACE_IDENTIFIER_FLAG_INDEX)
				{
					dr_device_open_config_enable_interface_by_index(
						config, i, test->local_interfaces[i].index
					);
				}
			}
		}
		result = dr_device_open_with_config(test->devices, config, &device_info->device);

		dr_device_open_config_free(config);
		config = NULL;
	}
	else
	{
		result = dr_device_open_local(test->devices, &device_info->device);
	}
	if (result != AUD_SUCCESS)
	{
	l__error:
		printf("Error creating device: %s\n",
			dr_error_message(result, g_test_errbuf));
		return result;
	}

	device_info->test = test;
	dr_device_set_context(device_info->device, device_info);
	dr_device_set_changed_callback(device_info->device, aes67_test_on_device_changed);

	// if we are connecting locally we may need to trigger the next state transition,
	// if not this is a no-op
	aes67_test_on_device_state_changed(device_info);

	return AUD_SUCCESS;
}


static void
aes67_test_get_descriptor_from_file
(
	aes67_test_t * test
)
{
	assert(test);
	assert(test->source.filename);

	test->sdp_descriptor_ref = dante_sdp_descriptor_alloc(NULL);
	if (! test->sdp_descriptor_ref)
	{
		fprintf(stderr, "Failed to allocate descriptor\n");
		exit(1);
	}

	sdp_from_file(test->sdp_descriptor_ref, test->source.filename);

	test->sdp_descriptor = dante_sdp_descriptor_from_ref(test->sdp_descriptor_ref);
		// Need to keep the ref as it backs the sdp_descriptor memory

	aes67_test_process(test);
}


static aud_bool_t
verify_arguments
(
	const aes67_test_t * test
)
{
	if (test->curr_target != &test->target)
	{
		fprintf(stderr, "Missing target\n");
		return AUD_FALSE;
	}

	if (test->source.tx_addr)
	{
		if (! test->num_local_interfaces)
		{
			fputs("Missing interface/s\n", stderr);
			return AUD_FALSE;
		}
	}

	return AUD_TRUE;
}


static struct aes67_test_target *
next_target
(
	aes67_test_t * test,
	const char * input
)
{
	if (! test->curr_target)
	{
		test->curr_target = &test->source;
	}
	else if (test->curr_target == &test->source)
	{
		test->curr_target = &test->target;
	}
	else
	{
		fprintf(stderr, "Too many targets: %s\n", input);
		usage();
		exit(1);
	}
	return test->curr_target;
}


int main(int argc, char * argv[])
{
	aes67_test_t test;
	aud_error_t result = AUD_SUCCESS;
	int i;
	aud_bool_t first_interface = AUD_TRUE;

	g_progname = argv[0];

	db_browse_config_t browse_config;
	db_browse_types_t types = 0;

	memset(&test, 0, sizeof(aes67_test_t));
	db_browse_config_init_defaults(&browse_config);

	types = DB_BROWSE_TYPE_AES67_FLOW;

	aud_bool_t options = AUD_TRUE;
	i = 1;
	while(i < argc)
	{
		const char * arg = argv[i];
		i++;

		if (options && arg[0] == '-')
		{
			const char * argval;
			if (strcmp(arg, "--") == 0)
			{
				options = AUD_FALSE;
			}
			else if (!strncmp(arg, "-a=", 3))
			{
				struct aes67_test_target * target = next_target(&test, arg);
				if (target != &test.source)
				{
					fprintf(stderr, "Multicast address can only be source\n");
					usage();
					exit(1);
				}
				target->tx_addr = arg + 3;
			}
			else if (!strncmp(arg, "-d=", 3))
			{
				struct aes67_test_target * target = next_target(&test, arg);
				const char * sep = strchr(arg+3, '@');
				if (! sep)
				{
					fprintf(stderr, "Device specifier without '@': %s\n", arg);
					usage();
					exit(1);
				}
				target->flow_id = (uint16_t) atoi(arg + 3);
				if (! target->flow_id)
				{
					fprintf(stderr, "Invalid or missing flow ID: %s\n", arg);
					usage();
					exit(1);
				}
				target->device_name = sep + 1;
			}
			else if (!strncmp(arg, "-ii=", 4))
			{
				argval = arg + 4;
				if (first_interface)
				{
					browse_config.num_interface_indexes = 0;
					first_interface = AUD_FALSE;
				}
				if (browse_config.num_interface_indexes < DB_BROWSE_MAX_INTERFACE_INDEXES)
				{
					int local_interface_index = atoi(argval);
					browse_config.interface_indexes[browse_config.num_interface_indexes] = atoi(argval);
					browse_config.num_interface_indexes++;
					test.local_interfaces[test.num_local_interfaces].index = local_interface_index;
					test.local_interfaces[test.num_local_interfaces].flags = AUD_INTERFACE_IDENTIFIER_FLAG_INDEX;
					test.num_local_interfaces++;
				}
				else
				{
					fprintf(stderr, "Too many interface indexes (max %d)\n", DB_BROWSE_MAX_INTERFACE_INDEXES);
					exit(1);
				}
			}
			else if (!strncmp(arg, "-f=", 3))
			{
				struct aes67_test_target * target = next_target(&test, arg);
				target->filename = arg + 3;
			}
			else
			{
				fprintf(stderr, "Invalid option: %s\n", arg);
				usage();
				exit(1);
			}
		}
		else
		{
			fprintf(stderr, "Too many arguments\n");
			usage();
			exit(1);
		}
	}

	// Verify arguments
	if(! verify_arguments(&test))
	{
		usage();
		exit(1);
	}


	// Parse multicast address
	if (test.source.tx_addr)
	{
		uint32_t addr;
		result = aud_inet_aton(test.source.tx_addr, &addr);
		test.source.tx_inaddr.s_addr = addr;

		if (result != AUD_SUCCESS)
		{
			fprintf(stderr, "Cannot parse flow address %s\n", test.source.tx_addr);
			exit(1);
		}
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	// create an environment
	result = dapi_new(&test.dapi);
	if (result != AUD_SUCCESS)
	{
		printf("Error initialising environment: %s\n", aud_error_message(result, g_test_errbuf));
		goto cleanup;
	}
	printf("Created environment\n");

	test.env = dapi_get_env(test.dapi);
	test.runtime = dapi_get_runtime(test.dapi);

	// Setup device/s
	result = dr_devices_new_dapi(test.dapi, &test.devices);
	if (result != AUD_SUCCESS)
	{
		printf("Error creating device factory: %s\n", dr_error_message(result, g_test_errbuf));
		goto cleanup;
	}
	dr_devices_set_context(test.devices, &test);

	if (test.source.device_name)
	{
        // open a device connection
        result = aes67_test_open_device(&test, &test.device_info.source, test.source.device_name);
        if (result != AUD_SUCCESS)
        {
                goto cleanup;
        }
    }

	if (test.target.device_name)
	{
        // open a device connection
        result = aes67_test_open_device(&test, &test.device_info.target, test.target.device_name);
        if (result != AUD_SUCCESS)
        {
                goto cleanup;
        }
    }


	// Setup browse
	if (test.source.tx_addr)
	{
		result = db_browse_new_dapi(test.dapi, types, &test.browse);
		if (result != AUD_SUCCESS)
		{
			printf("Error creating browse: %s\n", aud_error_message(result, g_test_errbuf));
			goto cleanup;
		}


		result = db_browse_set_max_sockets(test.browse, MAX_SOCKETS);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting max browse sockets: %s\n", aud_error_message(result, g_test_errbuf));
			goto cleanup;
		}

		db_browse_set_node_changed_callback(test.browse, aes67_test_node_changed);
		db_browse_set_context(test.browse, &test);

		result = db_browse_start_config(test.browse, &browse_config);
		if (result != AUD_SUCCESS)
		{
			printf("Error starting browse: %s\n", aud_error_message(result, g_test_errbuf));
			goto cleanup;
		}
	}

	// Check files
	if (test.source.filename)
	{
		aes67_test_get_descriptor_from_file(&test);
		if (test.target.filename)
		{
			sdp_to_file(test.sdp_descriptor, test.target.filename);
			exit(0);
		}
	}

	result = aes67_test_main_loop(&test);
	printf("Finished main loop\n");

cleanup:
	if (test.browse)
	{
		db_browse_delete(test.browse);
	}
	if (test.dapi)
	{
		dapi_delete(test.dapi);
	}
	return result;
}

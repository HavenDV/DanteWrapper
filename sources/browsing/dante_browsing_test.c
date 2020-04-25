/*
 * File     : dante_browsing_test.c
 * Created  : January 2007
 * Author   : James Westendorp
 * Synopsis : A test harness for the audinate DLL demonstrating its use.
 *
 * This software is copyright (c) 2007-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#include "audinate/dante_api.h"
#include "dapi_utils_domains.h"

#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>

// Used by .Net Wrapper
#include <strsafe.h>

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#ifdef WIN32
#define SNPRINTF _snprintf
#include <conio.h>
#else
#include <stdlib.h>
#define SNPRINTF snprintf
#endif

#define MAX_RESOLVES 16
#define MAX_SOCKETS MAX_RESOLVES + 6 // need space for up to 5 kinds of device adverts (media, conmon, safe, upgrade, via) and AES67

#define DB_TEST_DEBUG printf
#define DB_TEST_PRINT printf
#define DB_TEST_ERROR printf

static aud_bool_t g_running = AUD_TRUE;

db_browse_node_changed_fn db_test_node_changed;
db_browse_network_changed_fn db_test_network_changed;

void sig_handler(int sig);

void sig_handler(int sig)
{
	AUD_UNUSED(sig);
	signal(SIGINT, sig_handler);
	g_running = AUD_FALSE;
}

typedef struct db_browse_test
{
	dapi_t * dapi;
	aud_env_t * env;
	dante_runtime_t * runtime;
	dante_domain_handler_t * handler;
	db_browse_t * browse;
	aud_bool_t running;

	aud_bool_t print_node_changes;
	aud_bool_t print_network_changes;

	aud_bool_t network_changed;

	aud_errbuf_t errbuf;

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config;
#endif

	// Config
	db_browse_config_t   browse_config;
	db_browse_types_t    types;
	const char *         browse_filter;
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	uint16_t mdns_server_port;
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	union {
		uint16_t port_no;
		const char * socket_path;
	} domain_handler;
#endif
	char lastDomain[DANTE_DOMAIN_NAME_LENGTH];
} db_browse_test_t;

void db_test_print_sdp(db_browse_test_t * test, const db_browse_sdp_t * sdp);

ddh_change_event_fn db_test_event_handle_ddh_changes;

#define STRINGIFY(X) #X

#define CHECK_RESULT(FUNC) \
	if (result != AUD_SUCCESS)  \
		{ \
		DB_TEST_ERROR("Barfed on function %s (%s)!\n", STRINGIFY(FUNC), aud_error_get_name(result)); \
		goto cleanup; \
		}

#if 1
#define PRINT_EVENT(EV,FUNC) \
	assert(EV); \
	DB_TEST_PRINT("%s triggered events 0x%08x\n", STRINGIFY(FUNC), ddh_changes_get_change_flags(EV));

#define PRINT_LAST_CHANGES(HANDLER,FUNC) \
	{ \
		const ddh_changes_t * changes = dante_domain_handler_get_last_changes(HANDLER); \
		PRINT_EVENT(changes,FUNC); \
	}
#else
#define PRINT_EVENT(EV,FUNC) \
	assert(EV); \
	if (ddh_event_get_flags(EV)) \
		{ \
		char buf[256]; \
		DB_TEST_PRINT("%s triggered events %s\n", STRINGIFY(FUNC), ddh_event_to_string(EV, buf, sizeof(buf))); \
		}
#endif

//----------------------------------------------------------
// Print functions
//----------------------------------------------------------

static const char *
db_test_browse_types_to_string
(
	db_browse_types_t browse_types,
	char * buf,
	size_t len
) {
	size_t offset = 0;
	aud_bool_t first = AUD_TRUE;

	buf[0] = '\0';

	if (browse_types & DB_BROWSE_TYPE_MEDIA_DEVICE)
	{
		offset += SNPRINTF(buf+offset, len-offset, "%sMEDIA", (first ? "" : "|"));
		first = AUD_FALSE;
	}
	if (browse_types & DB_BROWSE_TYPE_CONMON_DEVICE)
	{
		offset += SNPRINTF(buf+offset, len-offset, "%sCONMON", (first ? "" : "|"));
		first = AUD_FALSE;
	}
	if (browse_types & DB_BROWSE_TYPE_SAFE_MODE_DEVICE)
	{
		offset += SNPRINTF(buf+offset, len-offset, "%sSAFE_MODE", (first ? "" : "|"));
		first = AUD_FALSE;
	}
	if (browse_types & DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE)
	{
		offset += SNPRINTF(buf + offset, len - offset, "%sUPGRADE_MODE", (first ? "" : "|"));
		first = AUD_FALSE;
	}
	if (browse_types & DB_BROWSE_TYPE_VIA_DEVICE)
	{
		offset += SNPRINTF(buf + offset, len - offset, "%sVIA", (first ? "" : "|"));
		first = AUD_FALSE;
	}
	return buf;
}

static const char *
db_test_node_change_to_string
(
	db_node_change_t node_change
) {
	switch (node_change)
	{
	case DB_NODE_CHANGE_ADDED: return "ADDED";
	case DB_NODE_CHANGE_MODIFIED: return "MODIFIED";
	case DB_NODE_CHANGE_REMOVED: return "REMOVED";
	default: return "?";
	};
}

static const char *
db_test_node_type_to_string
(
	db_node_type_t node_type
) {
	switch (node_type)
	{
	case DB_NODE_TYPE_DEVICE: return "DEVICE";
	case DB_NODE_TYPE_SDP:    return "SDP";

	default: return "?";
	};
}

static void
db_test_print_device
(
	db_browse_test_t * test,
	const db_browse_device_t * device
)  {
	char temp[64];
	const char * name = db_browse_device_get_name(device);

	const char * default_name = db_browse_device_get_default_name(device);
	db_browse_types_t all_types, network_types[DB_BROWSE_MAX_INTERFACE_INDEXES], localhost_types;
	unsigned int n, nn = db_browse_num_interface_indexes(test->browse);

	printf("name=\"%s\"", name);

	all_types = db_browse_device_get_browse_types(device);
	printf(" all_types=%s", db_test_browse_types_to_string(all_types, temp, sizeof(temp)));
	for (n = 0; n < nn; n++)
	{
		network_types[n] = db_browse_device_get_browse_types_on_network(device, n);
		printf(" network_types[%d]=%s", n, db_test_browse_types_to_string(network_types[n], temp, sizeof(temp)));
	}
	if (db_browse_using_localhost(test->browse))
	{
		localhost_types = db_browse_device_get_browse_types_on_localhost(device);
		printf(" localhost_types=%s", db_test_browse_types_to_string(localhost_types, temp, sizeof(temp)));
	}

	if (all_types)
	{
		printf(" default_name=\"%s\"", default_name);
	}
	if (all_types & DB_BROWSE_TYPE_MEDIA_DEVICE)
	{
		const dante_version_t * router_version = db_browse_device_get_router_version(device);
		const dante_version_t * arcp_version = db_browse_device_get_arcp_version(device);
		const dante_version_t * arcp_min_version = db_browse_device_get_arcp_min_version(device);
		const char * router_info = db_browse_device_get_router_info(device);

		printf(" router_version=%u.%u.%u", router_version->major, router_version->minor, router_version->bugfix);
		printf(" arcp_version=%u.%u.%u", arcp_version->major, arcp_version->minor, arcp_version->bugfix);
		printf(" arcp_min_version=%u.%u.%u", arcp_min_version->major, arcp_min_version->minor, arcp_min_version->bugfix);
		printf(" router_info=\"%s\"", router_info ? router_info : "");
	}
	if (all_types & DB_BROWSE_TYPE_SAFE_MODE_DEVICE)
	{
		uint16_t safe_mode_version = db_browse_device_get_safe_mode_version(device);
		printf("Safe mode %u",safe_mode_version);
	}
	if (all_types & DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE)
	{
		uint16_t upgrade_mode_version = db_browse_device_get_upgrade_mode_version(device);
		printf("Upgrade mode %u", upgrade_mode_version);
	}

	if (all_types & DB_BROWSE_TYPE_CONMON_DEVICE)
	{
		const conmon_instance_id_t * instance_id = db_browse_device_get_instance_id(device);
		const uint8_t * d = instance_id->device_id.data;

		const dante_id64_t * vendor_id = db_browse_device_get_vendor_id(device);
		const uint8_t * v = vendor_id ? vendor_id->data : NULL;

		uint32_t vendor_broadcast_address = db_browse_device_get_vendor_broadcast_address(device);

		printf(" instance_id=%02x%02x%02x%02x%02x%02x%02x%02x/%d",
			d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], instance_id->process_id);
		if (v)
		{
			printf(" vendor_id=%02x%02x%02x%02x%02x%02x%02x%02x",
				v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
		}
		if (vendor_broadcast_address)
		{
			uint8_t * a = (uint8_t *) &vendor_broadcast_address;
			printf(" vba=%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
		}
	}
	if (all_types & (DB_BROWSE_TYPE_CONMON_DEVICE | DB_BROWSE_TYPE_MEDIA_DEVICE))
	{
		const dante_id64_t * mf_id = db_browse_device_get_manufacturer_id(device);
		const dante_id64_t * model_id = db_browse_device_get_model_id(device);
		char id_buf[DANTE_ID64_DNSSD_BUF_LENGTH];

		if (mf_id)
		{
			dante_id64_to_dnssd_text(mf_id, id_buf);
			printf(" mf=%s", id_buf);
		}
		if (model_id)
		{
			dante_id64_to_dnssd_text(model_id, id_buf);
			printf(" model=%s", id_buf);
		}
	}
	if (all_types & DB_BROWSE_TYPE_VIA_DEVICE)
	{
		const dante_version_t * via_min_version = db_browse_device_get_via_min_version(device);
		const dante_version_t * via_curr_version = db_browse_device_get_via_curr_version(device);
		const uint16_t via_port = db_browse_device_get_via_port(device);

		printf(" via_min_version=%u.%u.%u", via_min_version->major, via_min_version->minor, via_min_version->bugfix);
		printf(" via_curr_version=%u.%u.%u", via_curr_version->major, via_curr_version->minor, via_curr_version->bugfix);
		printf(" via_port=%d", via_port);
	}
}

static const char * db_test_print_sdp_stream_dir
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
db_test_print_sdp_descriptor
(
	const dante_sdp_descriptor_t * sdp_desc
)
{
	const dante_clock_grandmaster_uuid_t *gmid;
	const dante_clock_subdomain_name_t *sub_domain;
	struct in_addr a;

	a.s_addr = dante_sdp_get_origin_addr(sdp_desc);

	// SDP body print

	printf("SDP origin username %s, session name:%s, session id:%llx, session originator address:%s",
			dante_sdp_get_origin_username(sdp_desc),
			dante_sdp_get_session_name(sdp_desc),
			(unsigned long long) dante_sdp_get_session_id(sdp_desc),
			inet_ntoa(a)
			);
	if (dante_sdp_source_is_dante(sdp_desc))
	{
		printf(" (Dante)");
	}
	putchar('\n');

	printf("SDP RTP media stream:  clock_offset:%u  payload type: %d\n",
		dante_sdp_get_media_clock_offset(sdp_desc),
		dante_sdp_get_stream_payload_type(sdp_desc)
	);
	uint8_t n_groups = dante_sdp_get_group_mdesc_count(sdp_desc);
	if (n_groups)
	{
		printf("SDP RTP stream addresses:");
		uint8_t i;
		for (i = 0; i < n_groups; i++)
		{
			a.s_addr = dante_sdp_get_mdesc_conn_addr(sdp_desc, i);
			printf("  %s:%d (%s)",
				inet_ntoa(a),
				dante_sdp_get_mdesc_stream_port(sdp_desc, i),
				dante_sdp_get_mdesc_id(sdp_desc, i)
			);
		}
		putchar('\n');
	}
	else
	{
		a.s_addr = dante_sdp_get_session_conn_addr(sdp_desc);
		printf("SDP RTP stream addr: %s:%d\n",
			inet_ntoa(a),
			dante_sdp_stream_get_port(sdp_desc)
		);
	}

	gmid = dante_sdp_get_network_clock_ref(sdp_desc);
	sub_domain = dante_sdp_get_network_clock_ref_domain(sdp_desc);

	printf("SDP RTP session GMID:Domain \t %02x:%02x:%02x:%02x:%02x:%02x:0:0:%s\n",
			gmid->data[0]&0xff, gmid->data[1]&0xff, gmid->data[2]&0xff, gmid->data[3]&0xff, gmid->data[4]&0xff, gmid->data[5]&0xff,
			(sub_domain ? sub_domain->data : "NULL")
			);

	printf("SDP RTP sample rate %d, encoding %d, num_ch %d\n", dante_sdp_get_stream_sample_rate(sdp_desc), dante_sdp_get_stream_encoding(sdp_desc), dante_sdp_get_stream_num_chans(sdp_desc));
	printf("SDP RTP stream direction %s\n", db_test_print_sdp_stream_dir(dante_sdp_get_stream_dir(sdp_desc)));
}


void
db_test_print_sdp
(
	db_browse_test_t * test,
	const db_browse_sdp_t * sdp
)
{
	const dante_sdp_descriptor_t *sdp_desc;

	AUD_UNUSED(test);

	db_browse_sdp_get_descriptor(sdp, &sdp_desc);

	printf("\n");

	db_test_print_sdp_descriptor(sdp_desc);
	printf("\n\n\n");
}


static void
db_test_print_node_change
(
	db_browse_test_t * test,
	const db_node_t * node,
	db_node_change_t node_change
) {
	printf("%s NODE %s: ", db_test_node_type_to_string(node->type), db_test_node_change_to_string(node_change));
	switch (node->type)
	{
	case DB_NODE_TYPE_DEVICE:
		db_test_print_device(test, node->_.device);
		break;

	case DB_NODE_TYPE_SDP:
		db_test_print_sdp(test, node->_.sdp);
		break;

	default:
		;
	}
	printf("\n");

	fflush(stdout);
}

//----------------------------------------------------------
// Callbacks
//----------------------------------------------------------

void
db_test_node_changed
(
	db_browse_t * browse,
	const db_node_t * node,
	db_node_change_t node_change
) {
	db_browse_test_t * test = (db_browse_test_t *) db_browse_get_context(browse);
	if (test->print_node_changes)
	{
		db_test_print_node_change(test, node, node_change);
	}
}

void
db_test_network_changed
(
	const db_browse_t * browse
) {
	db_browse_test_t * test = (db_browse_test_t *) db_browse_get_context(browse);
	test->network_changed = AUD_TRUE;
}


void db_test_event_handle_ddh_changes
(
	const ddh_changes_t * changes
) {


	dapi_utils_print_domain_changes(changes);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE

	dante_domain_handler_t * handler = ddh_changes_get_domain_handler(changes);
	ddh_change_flags_t change_flags = ddh_changes_get_change_flags(changes);
	db_browse_test_t * test = dante_domain_handler_get_context(handler);

	if (change_flags & DDH_CHANGE_FLAG_AVAILABLE_DOMAINS && aud_str_is_non_empty(test->lastDomain))
	{
		dante_domain_info_t current = dante_domain_handler_get_current_domain(handler);
		if (aud_str_is_empty(current.name))
		{
			dapi_utils_update_ddh_current_domain(handler, test->lastDomain);
		}
	}
#endif
}

static aud_error_t
db_browse_test_process_line(db_browse_test_t * test)
{
	aud_error_t result;

	unsigned int i;
	const db_browse_network_t* network = db_browse_get_network(test->browse);
	for (i = 0; i < db_browse_network_get_num_devices(network); i++)
	{
		db_browse_device_t* device = db_browse_network_device_at_index(network, i);
		const char* name = db_browse_device_get_name(device);
		result = db_browse_device_reconfirm(device, 0, AUD_FALSE);
		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	return AUD_SUCCESS;
}

//----------------------------------------------------------
// Helper functions for marshaling
//----------------------------------------------------------

static void set_output_array_length
(
	/*[in]*/ int value,
	/*[out]*/ char*** array,
	/*[out]*/ int* count
) 
{
	*count = value;
	size_t sizeOfArray = sizeof(char*) * value;
	*array = (char**)CoTaskMemAlloc(sizeOfArray);
	memset(*array, 0, sizeOfArray);
}

static void copy_to_output_array
(
	/*[in]*/ int i,
	/*[in]*/ const char* value,
	/*[out]*/ char*** array,
	/*[out]*/ int* count
)
{
	(*array)[i] = (char*)CoTaskMemAlloc(strlen(value) + 1);
	strcpy((*array)[i], value);
}

//----------------------------------------------------------
// Entry point
//----------------------------------------------------------

__declspec(dllexport) int GetDeviceNames
(
	/*[out]*/ char*** array,
	/*[out]*/ int* count
)
{
	signal(SIGINT, sig_handler);

	db_browse_test_t test;
	aud_error_t result = AUD_SUCCESS;

	memset(&test, 0, sizeof(db_browse_test_t));
	db_browse_config_init_defaults(&test.browse_config);

	test.types |= DB_BROWSE_TYPE_CONMON_DEVICE;
#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	if (test.types == 0)
	{
		test.types = DB_BROWSE_TYPES_ALL_DEVICES | DB_BROWSE_TYPE_AES67_FLOW;
	}

	result = dapi_new(&test.dapi);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Error initialising environment: %s\n", aud_error_message(result, test.errbuf));
		goto cleanup;
	}
	test.env = dapi_get_env(test.dapi);
	test.runtime = dapi_get_runtime(test.dapi);
	test.handler = dapi_get_domain_handler(test.dapi);
	assert(test.env);
	assert(test.dapi);
	assert(test.runtime);
	assert(test.handler);
	DB_TEST_DEBUG("Created environment\n");

	dante_domain_handler_set_context(test.handler, &test);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&test.ddm_config, test.handler, test.runtime, &g_running);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Error initiating ddm connection: %s\n", aud_error_message(result, test.errbuf));
		goto cleanup;
	}
	else
	{
		strcpy(test.lastDomain, test.ddm_config.domain);
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(test.handler);
	dante_domain_handler_set_event_fn(test.handler, db_test_event_handle_ddh_changes);

	// an interface index number must be provided for browsing AES67 SAP/SDP packet
	if ((test.types & DB_BROWSE_TYPE_AES67_FLOW) == DB_BROWSE_TYPE_AES67_FLOW)
	{
		if (!test.browse_config.interface_indexes[0])
		{
			DB_TEST_PRINT("\nBrowsing for AES67 SAP/SDP : Interface index number must be provided. i.e) -ii=[index number] \n\n");
			exit(0);
		}
	}

	result = db_browse_new(test.env, test.types, &test.browse);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Error creating browse: %s\n", aud_error_message(result, test.errbuf));
		goto cleanup;
	}

	result = db_browse_set_max_sockets(test.browse, MAX_SOCKETS);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Error setting max browse sockets: %s\n", aud_error_message(result, test.errbuf));
		goto cleanup;
	}

	db_browse_set_node_changed_callback(test.browse, db_test_node_changed);
	db_browse_set_network_changed_callback(test.browse, db_test_network_changed);
	db_browse_set_context(test.browse, &test);

	if (test.browse_filter && test.browse_filter[0])
	{
		dante_id64_t filter_id;
		if (dante_id64_from_dnssd_text(&filter_id, test.browse_filter))
		{
			db_browse_set_id64_filter(test.browse, &filter_id);
		}
	}

#ifdef DANTE_BROWSING_TEST_CUSTOM_CONFIG_OPTIONS
	DANTE_BROWSING_TEST_CUSTOM_CONFIG_OPTIONS(&test);
#endif

	result = db_browse_start_config(test.browse, &test.browse_config);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Error starting browse: %s\n", aud_error_message(result, test.errbuf));
		goto cleanup;
	}
	test.running = AUD_TRUE;

	dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);

	if (test.network_changed)
	{
		test.network_changed = AUD_FALSE;
	}

	STRSAFE_LPSTR temp;
	const size_t alloc_size = sizeof(char) * 101;

	for (size_t i = 0; i < 15; i++)
	{
		dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);
	}




	unsigned int i;
	const db_browse_network_t* network = db_browse_get_network(test.browse);
	unsigned int devices_count = db_browse_network_get_num_devices(network);
	set_output_array_length(devices_count, array, count);
	for (i = 0; i < devices_count; i++)
	{
		db_browse_device_t* device = db_browse_network_device_at_index(network, i);
		const char* name = db_browse_device_get_name(device);
		result = db_browse_device_reconfirm(device, 0, AUD_FALSE);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		temp = (STRSAFE_LPSTR)CoTaskMemAlloc(alloc_size);
		StringCchCopyA(temp, alloc_size, name);

		copy_to_output_array(i, name, array, count);
	}

	/*
	unsigned n = db_browse_get_num_sdp_descriptors(test.browse);
	if (n == 0)
	{
		fputs("No AES67 flows discovered\n", stdout);
	}
	else
	{
		unsigned i;
		for (i = 0; i < n; i++)
		{
			const dante_sdp_descriptor_t* sdp =
				db_browse_sdp_descriptor_at_index(test.browse, i);

			db_test_print_sdp_descriptor(sdp);
			putchar('\n');
		}
	}*/

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

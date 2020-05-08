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


typedef struct sdp_descriptor_group_info
{
	const char*          address;
	uint16_t             port;
	const char*          id;
} sdp_descriptor_group_info_t;

typedef struct sdp_descriptor_info
{
	const char*              username;
	const char*              session_name;
	unsigned long long       session_id;
	const char*              session_originator_address;
	aud_bool_t               is_dante;
	uint32_t                 media_clock_offset;
	uint8_t                  stream_payload_type;
	int                      groups_count;
	void**                   groups;
	const char*              gmid;
	const char*              sub_domain;
	uint32_t                 stream_sample_rate;
	uint16_t                 stream_encoding;
	uint16_t                 stream_num_chans;
	dante_sdp_stream_dir_t   stream_dir;
} sdp_descriptor_info_t;


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
// Helper functions for marshaling
//----------------------------------------------------------

static void set_output_array_length
(
	/*[in]*/ int size,
	/*[in]*/ int n,
	/*[out]*/ void*** array,
	/*[out]*/ int* count
)
{
	*count = n;
	size_t sizeOfArray = size * n;
	*array = (void**)CoTaskMemAlloc(sizeOfArray);
	memset(*array, 0, sizeOfArray);
}

static void copy_to_output_array
(
	/*[in]*/ int i,
	/*[in]*/ const void* value,
	/*[in]*/ int size,
	/*[out]*/ void*** array
)
{
	(*array)[i] = (void*)CoTaskMemAlloc(size);
	memcpy((*array)[i], value, size);
}

static void copy_string_to_output_array
(
	/*[in]*/ int i,
	/*[in]*/ const char* value,
	/*[out]*/ void*** array
)
{
	(*array)[i] = (char*)CoTaskMemAlloc(strlen(value) + 1);
	strcpy((*array)[i], value);
}


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
	const dante_sdp_descriptor_t * sdp_desc,
	sdp_descriptor_info_t* info
)
{
	const dante_clock_grandmaster_uuid_t *gmid;
	const dante_clock_subdomain_name_t *sub_domain;
	struct in_addr a;

	a.s_addr = dante_sdp_get_origin_addr(sdp_desc);

	// SDP body print

	info->username = dante_sdp_get_origin_username(sdp_desc);
	info->session_name = dante_sdp_get_session_name(sdp_desc);
	info->session_id = (unsigned long long) dante_sdp_get_session_id(sdp_desc);
	info->session_originator_address = inet_ntoa(a);
	printf("SDP origin username %s, session name:%s, session id:%llx, session originator address:%s",
		    info->username,
		    info->session_name,
		    info->session_id,
		    info->session_originator_address
			);
	info->is_dante = dante_sdp_source_is_dante(sdp_desc);
	if (info->is_dante)
	{
		printf(" (Dante)");
	}
	putchar('\n');

	info->media_clock_offset = dante_sdp_get_media_clock_offset(sdp_desc);
	info->stream_payload_type = dante_sdp_get_stream_payload_type(sdp_desc);
	printf("SDP RTP media stream:  clock_offset:%u  payload type: %d\n",
		info->media_clock_offset,
		info->stream_payload_type
	);
	uint8_t n_groups = dante_sdp_get_group_mdesc_count(sdp_desc);
	if (n_groups)
	{
		printf("SDP RTP stream addresses:");
		uint8_t i;
		set_output_array_length(sizeof(sdp_descriptor_group_info_t), n_groups, &info->groups, &info->groups_count);
		for (i = 0; i < n_groups; i++)
		{
			sdp_descriptor_group_info_t group_info;

			group_info.address = inet_ntoa(a);
			group_info.port = dante_sdp_get_mdesc_stream_port(sdp_desc, i);
			group_info.id = dante_sdp_get_mdesc_id(sdp_desc, i);
			a.s_addr = dante_sdp_get_mdesc_conn_addr(sdp_desc, i);
			printf("  %s:%d (%s)",
				group_info.address,
				group_info.port,
				group_info.id
			);

			copy_to_output_array(i, &group_info, sizeof(sdp_descriptor_group_info_t), &info->groups);
		}
		putchar('\n');
	}
	else
	{
		set_output_array_length(sizeof(sdp_descriptor_group_info_t), 1, &info->groups, &info->groups_count);
		a.s_addr = dante_sdp_get_session_conn_addr(sdp_desc);

		sdp_descriptor_group_info_t group_info;
		group_info.address = inet_ntoa(a);
		group_info.port = dante_sdp_stream_get_port(sdp_desc);
		printf("SDP RTP stream addr: %s:%d\n",
			group_info.address,
			group_info.port
		);

		copy_to_output_array(0, &group_info, sizeof(sdp_descriptor_group_info_t), &info->groups);
	}

	gmid = dante_sdp_get_network_clock_ref(sdp_desc);
	sub_domain = dante_sdp_get_network_clock_ref_domain(sdp_desc);

	char buffer[256];
	snprintf(buffer, 256, "%02x:%02x:%02x:%02x:%02x:%02x:0:0",
		gmid->data[0] & 0xff, gmid->data[1] & 0xff, gmid->data[2] & 0xff, gmid->data[3] & 0xff, gmid->data[4] & 0xff, gmid->data[5] & 0xff);
	info->gmid = buffer;
	info->sub_domain = (sub_domain ? sub_domain->data : "NULL");

	printf("SDP RTP session GMID:Domain \t %02x:%02x:%02x:%02x:%02x:%02x:0:0:%s\n",
			gmid->data[0]&0xff, gmid->data[1]&0xff, gmid->data[2]&0xff, gmid->data[3]&0xff, gmid->data[4]&0xff, gmid->data[5]&0xff,
			(sub_domain ? sub_domain->data : "NULL")
			);

	info->stream_sample_rate = dante_sdp_get_stream_sample_rate(sdp_desc);
	info->stream_encoding = dante_sdp_get_stream_encoding(sdp_desc);
	info->stream_num_chans = dante_sdp_get_stream_num_chans(sdp_desc);
	printf("SDP RTP sample rate %d, encoding %d, num_ch %d\n", 
		info->stream_sample_rate,
		info->stream_encoding,
		info->stream_num_chans);

	info->stream_dir = dante_sdp_get_stream_dir(sdp_desc);
	printf("SDP RTP stream direction %s\n", 
		db_test_print_sdp_stream_dir(info->stream_dir));
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

	sdp_descriptor_info_t info;
	db_test_print_sdp_descriptor(sdp_desc, &info);
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

//----------------------------------------------------------
// Main functionality
//----------------------------------------------------------

#if 0
static aud_error_t step(db_browse_test_t * test, aud_socket_t test_socket, dante_sockets_t * select_sockets)
{
	dante_sockets_t temp_sockets;
	if (!select_sockets)
	{
		select_sockets = &temp_sockets;
	}

	int select_result;
	struct timeval timeout;
	aud_error_t result = AUD_SUCCESS;

	// prepare sockets and timeouts
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	dante_sockets_clear(select_sockets);
	if (test_socket != AUD_SOCKET_INVALID)
	{
		dante_sockets_add_read(select_sockets, test_socket);
	}
	result = dante_runtime_get_sockets_and_timeout(test->runtime, select_sockets, &timeout);
	if (result != AUD_SUCCESS)
	{
		DB_TEST_ERROR("Failed to get Dante sockets and timeout: %s\n", aud_error_message(result, test->errbuf));
	}
	if (timeout.tv_sec >= 1)
	{
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
	}

	// drive select loop
	select_result = select(select_sockets->n, &select_sockets->read_fds, &select_sockets->write_fds, NULL, &timeout);

	if (select_result < 0)
	{
		result = aud_error_get_last();
		if (result != AUD_ERR_INTERRUPTED)
		{
			DB_TEST_ERROR("Error select()ing: %s\n", aud_error_message(result, test->errbuf));
		}
	}
	else
	{
		result = dante_runtime_process_with_sockets(test->runtime, select_sockets);
		if (result != AUD_SUCCESS)
		{
			DB_TEST_ERROR("Error processing: %s\n", aud_error_message(result, test->errbuf));
		}

	}
	return result;
}
#endif

static char *
drop_whitespace(const char * buf)
{
	while(isspace(buf[0]))
		buf++;
	return (char *) buf;
}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
static size_t
copy_input_string
(
	const char * src,
	char * dst,
	size_t dstlen,
	char ** tail,
	aud_bool_t * ok
)
{
	unsigned i = 0, i_dst = 0;
	aud_bool_t quoted = AUD_FALSE;
	aud_bool_t escaped = AUD_FALSE;
	if (src[0] == '"')
	{
		quoted = AUD_TRUE;
		i++;
	}

	for (; src[i]; i++)
	{
		char ch = src[i];

		// Handle exceptions
		if (escaped)
		{
			escaped = AUD_FALSE;
			// Escaped characters are never exceptional
		}
		else
		{
			if (ch == '\\')
			{
				escaped = AUD_TRUE;
				continue;
			}
			else if (ch == '"')
			{
				i++;
					// Absorb the quote
				quoted = ! quoted;
				goto l__end;
			}
			else if (! quoted && isspace(ch))
			{
				goto l__end;
			}
		}

		if (dst && i_dst + 1 < dstlen)
		{
			dst[i_dst] = ch;
		}
		i_dst++;
	}

l__end:
	if (tail)
	{
		*tail = (char *) src + i;
	}
	if (ok)
	{
		*ok = ! quoted;
			// if quoted on exist, then have format error
	}
	if (dst)
	{
		if (i_dst < dstlen)
		{
			dst[i_dst] = 0;
		}
		else if (dstlen)
		{
			dst[dstlen - 1] = 0;
		}
	}
	return i_dst;
}

static void
set_domain_for_string
(
	db_browse_test_t * test,
	const char * domain_str
)
{
	aud_error_t result;
	if (domain_str[0] == 0)
	{
		printf("Error: empty domain");
	}
	else if (strcmp(domain_str, ".") == 0 ||
		strcmp(domain_str, "ADHOC") == 0 ||
		strcmp(domain_str, "<ADHOC>") == 0
	)
	{
		result = dante_domain_handler_set_current_domain_by_uuid(test->handler, DANTE_DOMAIN_UUID_ADHOC);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting domain to ADHOC");
		}
		else
		{
			printf("Browsing in ADHOC domain");
			strcpy(test->lastDomain, "");
		}
	}
	else
	{
		dante_domain_info_t domain_info =
			dante_domain_handler_available_domain_with_name(test->handler, domain_str);
		if (domain_info.id)
		{
			result = dante_domain_handler_set_current_domain_by_id(test->handler, domain_info.id);
			if (result != AUD_SUCCESS)
			{
				printf("Error setting domain to '%s'", domain_str);
			}
			else
			{
				printf("Browsing in domain '%s'", domain_str);
				strcpy(test->lastDomain, domain_str);
			}
		}
		else
		{
			printf("Unable to find domain '%s'\n", domain_str);
		}
	}
}
#endif

static void
db_browse_test_print_input_help(const db_browse_test_t * test)
{
	(void) test;

	puts("Input options:\n"
		"r d <name>   Reconfirm device\n"
		"r d          Reconfirm all devices\n"
		"r b          Rediscover missing devices\n"
		"r r <name>   Re-resolve device\n"
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		"d <domain>   Set current domain\n"
#endif
		"p            Print discovery count of AES67 descriptors\n"
		"x [0|1|r]    Stop / start / restart current browse\n"
		"ad <seconds> Set adhoc startup delay\n"
		"?, h         Show this help\n"
	);
}

static aud_error_t
db_browse_test_process_line(
	/*[in]*/ db_browse_test_t * test,
	/*[in]*/ char * buf_in,
	/*[out]*/ char*** array,
	/*[out]*/ int* count)
{
	aud_error_t result;
	char in_action, in_type;
	char in_name[64];
	int n_scan;

	// drop leading whitespace
	char * buf = drop_whitespace(buf_in);
	if (! buf[0])
	{
		// empty input
		return AUD_SUCCESS;
	}

	/* NOTE:
		Reconfirm / rediscover / re-resolve create load on the local processor
		and on all Dante devices on the network.  Use sparingly.
	 */

	if (sscanf(buf, "%c %c %s", &in_action, &in_type, in_name) == 3 && in_action == 'r' && in_type == 'd')
	{
		/*
			Reconfirm checks that a previously known device record is still active.

			Use this when a device appears in the browsing API but cannot be communicated with.
		 */
		const db_browse_network_t * network = db_browse_get_network(test->browse);
		db_browse_device_t * device = db_browse_network_device_with_name(network, in_name);
		if (!device)
		{
			printf("Unknown device '%s'\n", in_name);
			return AUD_SUCCESS;
		}
		result = db_browse_device_reconfirm(device, 0, AUD_FALSE);
		if (result != AUD_SUCCESS)
		{
			printf("Error reconfirming device '%s': %s\n", in_name, aud_error_message(result, test->errbuf));
			//return result;
		}
		printf("Reconfirming device '%s'\n", in_name);
	}
	else if (sscanf(buf, "%c %c", &in_action, &in_type) == 2 && in_action == 'r' && in_type == 'd')
	{
		/*
		uint16_t ii;
		set_output_array_length(sizeof(char*), 3, array, count);
		for (ii = 0; ii < 3; ii++)
		{
			copy_string_to_output_array(ii, "test name", array);
		}
		return AUD_SUCCESS;
		*/

		unsigned int i;
		const db_browse_network_t * network = db_browse_get_network(test->browse);

		unsigned int n = db_browse_network_get_num_devices(network);
		set_output_array_length(sizeof(char*), n, array, count);
		for (i = 0; i < n; i++)
		{
			db_browse_device_t * device = db_browse_network_device_at_index(network, i);
			const char * name = db_browse_device_get_name(device);
			result = db_browse_device_reconfirm(device, 0, AUD_FALSE);
			if (result != AUD_SUCCESS)
			{
				printf("Error reconfirming device '%s': %s\n", name, aud_error_message(result, test->errbuf));
				//return result;
			}
			printf("Reconfirming device '%s'\n", name);

			copy_string_to_output_array(i, name, array);
		}
	}
	else if (sscanf(buf, "%c %c", &in_action, &in_type) == 2 && in_action == 'r' && in_type == 'b')
	{
		/*
			Rediscover prompts all devices to re-send their discovery information,
			attempting to discover missing devices.
		 */
		result = db_browse_rediscover(test->browse, 0);
		if (result != AUD_SUCCESS)
		{
			printf("Error rediscovering devices: %s\n", aud_error_message(result, test->errbuf));
			return result;
		}
		printf("Rediscovering missing devices\n");
	}
	else if (sscanf(buf, "%c %c %s", &in_action, &in_type, in_name) == 3 && in_action == 'r' && in_type == 'r')
	{
		/*
			Re-resolve updates addressing and related information for a discovered device
		 */
		const db_browse_network_t * network = db_browse_get_network(test->browse);
		db_browse_device_t * device = db_browse_network_device_with_name(network, in_name);
		if (!device)
		{
			printf("Unknown device '%s'\n", in_name);
			return AUD_SUCCESS;
		}
		printf("Re-resolving device %s\n", in_name);
		result = db_browse_device_reresolve(device, 0);
		if (result != AUD_SUCCESS)
		{
			printf("Error re-resolving devices: %s\n", aud_error_message(result, test->errbuf));
			//return result;
		}
		else{
			printf("Re-resolved device '%s'\n",in_name);
		}
	}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	else if (buf[0] == 'd')
	{
		char * tail = drop_whitespace(buf + 1);
		if (! tail[0])
		{
			unsigned i, n = dante_domain_handler_num_available_domains(test->handler);
			dante_domain_info_t current = dante_domain_handler_get_current_domain(test->handler);
			printf("Current domain: %s\n", aud_str_is_non_empty(current.name) ? current.name : "ADHOC");
			printf("Available domains: %u\n", n);
			for (i = 0; i < n; i++)
			{
				dante_domain_info_t domain_info =
					dante_domain_handler_available_domain_at_index(test->handler, i);
				if (domain_info.name[0])
				{
					printf("\t%s\n", domain_info.name);
				}
				else
				{
					printf("\tADHOC\n");
				}
			}
		}
		else if (! isspace(buf[1]))
		{
			printf("Invalid operation '%s'\n", buf);
		}
		else
		{
			aud_bool_t ok;
			char * tail2;
			size_t len = copy_input_string(tail, in_name, 64, &tail2, &ok);
			if (! ok)
			{
				printf("Bad quoting: '%s'\n", tail);
			}
			else if (len >= 64)
			{
				printf("Input name too long: '%s...'\n", in_name);
			}
			else if (drop_whitespace(tail2)[0])
			{
				printf("Extra arguments: '%s'\n", tail2);
			}
			else
			{
				set_domain_for_string(test, in_name);
			}
		}
	}
#endif
	else if (buf[0] == 'p')
	{
		/*
		uint16_t ii, jj;
		set_output_array_length(sizeof(sdp_descriptor_info_t), 3, array, count);
		for (ii = 0; ii < 3; ii++)
		{
			sdp_descriptor_info_t info;
			memset(&info, 0, sizeof(sdp_descriptor_info_t));
			info.username = "test username";
			info.session_name = "test session name";
			info.session_id = ii;
			info.session_originator_address = "test address";
			info.is_dante = ii;
			info.media_clock_offset = ii;
			info.stream_payload_type = (uint8_t)ii;

			set_output_array_length(sizeof(sdp_descriptor_group_info_t), 3, &info.groups, &info.groups_count);
			for (jj = 0; jj < 3; jj++)
			{
				sdp_descriptor_group_info_t group_info;
				memset(&info, 0, sizeof(sdp_descriptor_group_info_t));

				group_info.address = "test address";
				group_info.port = jj;
				group_info.id = "test id";

				copy_to_output_array(jj, &group_info, sizeof(sdp_descriptor_group_info_t), &info.groups);
			}

			info.gmid = "test gmid";
			info.sub_domain = "test sub_domain";
			info.stream_sample_rate = ii;
			info.stream_encoding = ii;
			info.stream_num_chans = ii;
			info.stream_dir = ii;

			copy_to_output_array(ii, &info, sizeof(sdp_descriptor_info_t), array);
		}
		return AUD_SUCCESS;
		*/

		unsigned n = db_browse_get_num_sdp_descriptors(test->browse);
		set_output_array_length(sizeof(sdp_descriptor_info_t), n, array, count);
		if (n == 0)
		{
			fputs("No AES67 flows discovered\n", stdout);
		}
		else
		{
			unsigned i;
			for (i = 0; i < n; i++)
			{
				const dante_sdp_descriptor_t * sdp =
					db_browse_sdp_descriptor_at_index(test->browse, i);

				sdp_descriptor_info_t info;
				db_test_print_sdp_descriptor(sdp, &info);
				putchar('\n');

				copy_to_output_array(i, &info, sizeof(sdp_descriptor_info_t), array);
			}
		}
	}
	else if (buf[0] == 'x')
	{
		aud_bool_t to_stop = AUD_FALSE, to_start = AUD_FALSE;
		n_scan = sscanf(buf + 1, " %c", &in_type);
		if (n_scan > 0)
		{
			switch(in_type)
			{
			case '0':
				to_stop = AUD_TRUE;
				break;
			case '1':
				to_start = AUD_TRUE;
				break;
			case 'r':
				to_stop = AUD_TRUE;
				to_start = AUD_TRUE;
				break;
			default:
				printf("Unknown stop / start operation: '%s'\n"
					"  Input must be 'x [0|1|r]'\n"
					, buf
				);
				return AUD_SUCCESS;
					// success => keep handling inputs
			}
		}
		if (to_stop)
		{
			fputs("Stopping browse: ", stdout);
			if (test->running)
			{
				db_browse_stop(test->browse);
				puts("stopped");
				test->running = AUD_FALSE;
			}
			else
			{
				puts("not started");
			}
		}
		if (to_start)
		{
			fputs("Starting browse: ", stdout);
			if (test->running)
			{
				puts("already started");
			}
			else
			{
				result = db_browse_start_config(test->browse, &test->browse_config);
				if (result == AUD_SUCCESS)
				{
					puts("started");
					test->running = AUD_TRUE;
				}
				else
				{
					printf("failed: %d\n", result);
				}
			}
		}
		printf("Browse is %s\n", (test->running ? "started" : "stopped"));
	}
	else if (buf[0] == 'a' && buf[1] == 'd')
	{
		/*
			Sets the delay when switching from managed domain to adhoc.

			This is only required when supporting browsing in domains on certain
			embedded platforms (e.g. BK-II or Broadway), and works around
			a possible stall due to processing load when re-entering adhoc mode.
			Only other platforms, it can be safely left at 0.
		 */
		unsigned adhoc_delay;
		n_scan = sscanf(buf, "ad %u", &adhoc_delay);
		if (n_scan != 1)
		{
			puts("To set adhoc startup delay, use: ad <delay>");
			return AUD_SUCCESS;
		}
		printf("Setting adhoc startup delay to %u\n", adhoc_delay);
		db_browse_set_adhoc_startup_delay(test->browse, (uint32_t) adhoc_delay);
	}
#ifdef DANTE_BROWSING_TEST_CUSTOM_PROCESS_LINE
	else if (DANTE_BROWSING_TEST_CUSTOM_PROCESS_LINE(test, buf))
	{}
#endif
	else if (buf[0] == 'h' || buf[0] == '?')
	{
		db_browse_test_print_input_help(test);
	}
	else
	{
		printf("Unknown command '%s'\n", buf);
	}
	return AUD_SUCCESS;
}

static aud_error_t
db_browse_test_main_loop
(
	db_browse_test_t * test,
	/*[out]*/ char*** array,
	/*[out]*/ int* count
) {
	aud_error_t result;
	aud_bool_t print_prompt = AUD_TRUE;

#ifdef  _WIN32
	// set to line buffered mode.
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_INPUT);
#else
	dante_sockets_t select_sockets;
#endif

	DB_TEST_PRINT("Running main loop\n");

	while(g_running)
	{
		char buf[BUFSIZ];

		// print prompt if needed
		if (print_prompt)
		{
			printf("\n> ");
			fflush(stdout);
			print_prompt = AUD_FALSE;
		}

#ifdef _WIN32
		dapi_utils_step(test->runtime, AUD_SOCKET_INVALID, NULL);
#else
		dapi_utils_step(test->runtime, 0, &select_sockets);
#endif
		if (test->network_changed)
		{
			printf("Network changed\n");
			test->network_changed = AUD_FALSE;
		}


		// and check stdin
		buf[0] = '\0';

#ifdef _WIN32
		if (_kbhit())
		{
			DWORD len = 0;
			if (!ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE),buf,BUFSIZ-1,&len, 0))
			{
				DB_TEST_ERROR("Error reading console: %d\n", GetLastError());
			}
			else if (len > 0)
			{
				buf[len] = '\0';
			}
			print_prompt = AUD_TRUE;
		}
#else
		if (FD_ISSET(0, &select_sockets.read_fds)) // 0 is always stdin
		{
			if (fgets(buf, BUFSIZ, stdin) == NULL)
			{
				result = aud_error_get_last();
				if (feof(stdin))
				{
					printf("Exiting...\n");
					return AUD_SUCCESS;
				}
				else if (result == AUD_ERR_INTERRUPTED)
				{
					clearerr(stdin);
				}
				else
				{
					printf("Exiting with %s\n", dr_error_message(result, test->errbuf));
					return result;
				}
			}
			print_prompt = AUD_TRUE;
		}
#endif

		// if we got some input then process the line
		if (buf[0])
		{
		#ifdef _WIN32
			DB_TEST_PRINT("\n");
		#endif
			result = db_browse_test_process_line(test, buf, array, count);
			if (result != AUD_SUCCESS)
			{
				break;
			}
		}
	}
	return AUD_SUCCESS;
}

static void usage(void)
{
	printf("OPTIONS:\n");
	printf("  -i print incremental (node) changes\n");
	printf("  -n print network changes\n");
	printf("  -media browse for media devices\n");
	printf("  -conmon browse for conmon devices\n");
	printf("  -safe browse for safe-mode devices\n");
	printf("  -upgrade browse for upgrade devices\n");
	printf("  -via browse for via devices\n");
	printf("  -aes67 browse for aes67 sap announcements (interface index number must be provided)\n");
	printf("  -sdp browse for sdp descriptors announcements (interface index number must be provided)\n");
	printf("  -in=NAME add browsing network with interface called NAME\n");
	printf("  -ii=INDEX add browsing network with interface at INDEX\n");
	printf("  -localhost=BOOL enable / disable browsing on localhost interface\n");
	printf("  -f=_MFID filter browse by manufacturer ID (syntax _0123abcd...)\n");
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	printf("  -m=PORT_NO set MDNS server port number to PORT_NO\n");
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
#ifdef WIN32
	printf("  -d=PORT_NO set domain handler port number to PORT_NO\n");
#else
	printf("  -d=PATH set domain handler socket path to PATH\n");
#endif
#endif

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	printf("  --domain[=<preferred domain>] browse on a domain\n");
	printf("  --user=<user> browse on a domain with the specified username\n");
	printf("  --pass=<password> browse on a domain with the specified password\n");
	printf("  --ddm=<host>:<port> use the specified ddm rather than discovering (--domain must be specified)\n");
#endif
#ifdef DANTE_BROWSING_TEST_CUSTOM_USAGE
	DANTE_BROWSING_TEST_CUSTOM_USAGE();
#endif
}


static void
db_test_parse_options
(
	db_browse_test_t * test,
	int argc,
	char * argv[]
) {

	int i;
	aud_interface_identifier_t ifaces[DB_BROWSE_MAX_INTERFACE_INDEXES];
	unsigned int num_ifaces = 0;
	aud_error_t result;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-i"))
		{
			test->print_node_changes = AUD_TRUE;
		}
		else if (!strcmp(argv[i], "-n"))
		{
			test->print_network_changes = AUD_TRUE;
		}
		else if (!strcmp(argv[i], "-media"))
		{
			test->types |= DB_BROWSE_TYPE_MEDIA_DEVICE;
		}
		else if (!strcmp(argv[i], "-conmon"))
		{
			test->types |= DB_BROWSE_TYPE_CONMON_DEVICE;
		}
		else if (!strcmp(argv[i], "-safe"))
		{
			test->types |= DB_BROWSE_TYPE_SAFE_MODE_DEVICE;
		}
		else if (!strcmp(argv[i], "-upgrade"))
		{
			test->types |= DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE;
		}
		else if (!strcmp(argv[i], "-via"))
		{
			test->types |= DB_BROWSE_TYPE_VIA_DEVICE;
		}
		else if (!strcmp(argv[i], "-aes67"))
		{
			test->types |= DB_BROWSE_TYPE_AES67_FLOW;
		}
		else if (!strcmp(argv[i], "-sdp"))
		{
			test->types |= DB_BROWSE_TYPE_SDP;
		}
		else if (!strncmp(argv[i], "-in=", 4))
		{
			if (num_ifaces < DB_BROWSE_MAX_INTERFACE_INDEXES)
			{
				ifaces[num_ifaces].flags = AUD_INTERFACE_IDENTIFIER_FLAG_NAME;
#if AUD_INTERFACE_NAME_IS_WCHAR == 1
				mbstowcs(ifaces[num_ifaces].name, argv[i]+4, AUD_INTERFACE_NAME_LENGTH);
#else
				aud_strlcpy(ifaces[num_ifaces].name, argv[i]+4, AUD_INTERFACE_NAME_LENGTH);
#endif
				num_ifaces++;
			}
			else
			{
				printf("Too many interfaces specified (max %d)\n", DB_BROWSE_MAX_INTERFACE_INDEXES);
				exit(0);
			}
		}
		else if (!strncmp(argv[i], "-ii=", 4))
		{
			if (num_ifaces < DB_BROWSE_MAX_INTERFACE_INDEXES)
			{
				ifaces[num_ifaces].flags = AUD_INTERFACE_IDENTIFIER_FLAG_INDEX;
				ifaces[num_ifaces].index = atoi(argv[i] + 4);
				num_ifaces++;
			}
			else
			{
				printf("Too many interfaces specified (max %d)\n", DB_BROWSE_MAX_INTERFACE_INDEXES);
				exit(0);
			}
		}
		else if (!strcmp(argv[i], "-localhost=true"))
		{
			test->browse_config.localhost = AUD_TRUE;
		}
		else if (!strcmp(argv[i], "-localhost=false"))
		{
			test->browse_config.localhost = AUD_FALSE;
		}
		else if (!strncmp(argv[i], "-f=", 3))
		{
			test->browse_filter = argv[i] + 3;
		}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		else if (dapi_utils_ddm_config_parse_one(&test->ddm_config, argv[i], &result))
		{
			if (result != AUD_SUCCESS)
			{
				usage();
				exit(0);
			}
		}
#endif
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
		else if (!strncmp(argv[i], "-m=", 3))
		{
			test->mdns_server_port = (uint16_t) atoi(argv[i] + 3);
		}
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
		else if (!strncmp(argv[i], "-d=", 3))
		{
#ifdef WIN32
			test->domain_handler.port_no = (uint16_t)atoi(argv[i] + 3);
#else
			test->domain_handler.socket_path = argv[i] + 3;
#endif
		}
#endif
#ifdef DANTE_BROWSING_TEST_CUSTOM_PARSE_OPTIONS
		else if (DANTE_BROWSING_TEST_CUSTOM_PARSE_OPTIONS(test, argc, argv, &i))
		{}
#endif
		else
		{
			usage();
			exit(0);
		}
	}

	// Prepare the interface list, including converting name to index if needed
	if (num_ifaces)
	{
		for (i = 0; i < (int)num_ifaces; i++)
		{
			result = aud_interface_get_identifiers(NULL, ifaces + i, 1);
			if (result != AUD_SUCCESS)
			{
				if (ifaces[i].flags & AUD_INTERFACE_IDENTIFIER_FLAG_INDEX)
				{
					printf("Unknown interface index %d\n", ifaces[i].index);
					exit(0);
				}
				else if (ifaces[i].flags & AUD_INTERFACE_IDENTIFIER_FLAG_NAME)
				{
#if AUD_INTERFACE_NAME_IS_WCHAR == 1
					wprintf(L"Unknown interface name '%s'\n", ifaces[i].name);
#else
					printf("Unknown interface name %s\n", ifaces[i].name);

#endif
					exit(0);
				}
			}
			test->browse_config.interface_indexes[i] = ifaces[i].index;
		}
		test->browse_config.num_interface_indexes = num_ifaces;
	}
}

//----------------------------------------------------------
// Entry point
//----------------------------------------------------------

__declspec(dllexport) int RunDll
(
	/*[in]*/ int argc,
	/*[in]*/ char* argv[],
	/*[in]*/ char* input,
	/*[out]*/ char*** array,
	/*[out]*/ int* count
)
{

	signal(SIGINT, sig_handler);

	db_browse_test_t test;
	aud_error_t result = AUD_SUCCESS;

	memset(&test, 0, sizeof(db_browse_test_t));
	db_browse_config_init_defaults(&test.browse_config);

	db_test_parse_options(&test, argc, argv);

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	if (test.types == 0)
	{
		test.types = DB_BROWSE_TYPES_ALL_DEVICES | DB_BROWSE_TYPE_AES67_FLOW;
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	dapi_config_t * dapiConfig = dapi_config_new();
	if (dapiConfig)
	{
		dante_domain_handler_config_t * domainHandlerConfig = dapi_config_get_domain_handler_config(dapiConfig);

		if (domainHandlerConfig)
		{
#ifdef WIN32
			dante_domain_handler_config_set_port(domainHandlerConfig, test.domain_handler.port_no);
#else
			dante_domain_handler_config_set_unix_path(domainHandlerConfig, test.domain_handler.socket_path);
#endif
		}
	}

#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	if (test.mdns_server_port > 0)
	{
		dapi_config_set_mdns_server_port(dapiConfig, test.mdns_server_port);
	}
#endif

	result = dapi_new_config(dapiConfig, &test.dapi);

	dapi_config_delete(dapiConfig);
#else
	result = dapi_new(&test.dapi);
#endif
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
			usage();
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

	//result = db_browse_test_main_loop(&test);
	//DB_TEST_DEBUG("Finished main loop\n");

	for (size_t i = 0; i < 15; i++)
	{
		dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);
	}

	result = db_browse_test_process_line(&test, input, array, count);

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

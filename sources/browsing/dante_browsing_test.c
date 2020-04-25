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
			printf("Error reconfirming device '%s': %s\n", name, aud_error_message(result, test->errbuf));
			return result;
		}

		printf("Reconfirming device '%s'\n", name);
	}

	return AUD_SUCCESS;
}

static aud_error_t
db_browse_test_main_loop
(
	db_browse_test_t * test
) {
	aud_error_t result;
	aud_bool_t print_prompt = AUD_TRUE;

#ifdef  _WIN32
	// set to line buffered mode.
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_INPUT);
#else
	dante_sockets_t select_sockets;
#endif

	{

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

		{
		#ifdef _WIN32
			DB_TEST_PRINT("\n");
		#endif
			result = db_browse_test_process_line(test);
			if (result != AUD_SUCCESS)
			{
				return result;
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

__declspec(dllexport) int GetNames(int argc, char * argv[])
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

	result = db_browse_test_main_loop(&test);
	//argv[0] = "hello";

	STRSAFE_LPSTR temp;
	const size_t alloc_size = sizeof(char) * 101;

	dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);

	unsigned int i;
	const db_browse_network_t* network = db_browse_get_network(test.browse);
	for (i = 0; i < db_browse_network_get_num_devices(network); i++)
	{
		db_browse_device_t* device = db_browse_network_device_at_index(network, i);
		const char* name = db_browse_device_get_name(device);
		result = db_browse_device_reconfirm(device, 0, AUD_FALSE);
		if (result != AUD_SUCCESS)
		{
			printf("Error reconfirming device '%s': %s\n", name, aud_error_message(result, test.errbuf));
			//return result;
		}
		printf("Reconfirming device '%s'\n", name);

		temp = (STRSAFE_LPSTR)CoTaskMemAlloc(alloc_size);
		StringCchCopyA(temp, alloc_size, name);

		CoTaskMemFree(argv[i]);
		argv[i] = (char*)temp;
	}

	/*
	for (int i = 0; i < argc; i++)
	{
		len = 0;
		StringCchLengthA(argv[i], STRSAFE_MAX_CCH, &len);
		result += len;

		temp = (STRSAFE_LPSTR)CoTaskMemAlloc(alloc_size);
		StringCchCopyA(temp, alloc_size, (STRSAFE_LPCSTR)"123456789");

		// CoTaskMemFree must be used instead of delete to free memory.

		CoTaskMemFree(argv[i]);
		argv[i] = (char*)temp;
	}*/

	DB_TEST_DEBUG("Finished main loop\n");

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

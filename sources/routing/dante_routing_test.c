/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : A test harness for the audinate DLL demonstrating its use.
 *
 * This software is copyright (c) 2007-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */
#include "dante_routing_test.h"
#include "dapi_utils_domains.h"
#ifdef _WIN32
#include <conio.h>
#endif

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#define DR_TEST_REQUEST_DESCRIPTION_LENGTH 64
#define DR_TEST_MAX_REQUESTS 128

#define DR_TEST_MAX_BATCH 32
typedef struct
{
	uint16_t n;
	union
	{
		dr_batch_subscription_t subscriptions[DR_TEST_MAX_BATCH];
		dr_batch_rxlabel_t rxlabels[DR_TEST_MAX_BATCH];
	} _;
} dr_test_batch_t;

static dr_test_batch_t g_batch = {0};

static aud_bool_t g_test_running = AUD_TRUE;

#define STRINGIFY(X) #X

#define CHECK_RESULT(FUNC) \
	if (result != AUD_SUCCESS)  \
			{ \
		DR_TEST_ERROR("Barfed on function %s (%s)!\n", STRINGIFY(FUNC), aud_error_get_name(result)); \
		goto cleanup; \
			}

typedef struct dr_test_options
{
	// domain info
	/*
	aud_bool_t           connect_domain;
	char                 ddm_username[100];
	char                 ddm_password[100];
	char                 ddm_domain[100];

	aud_bool_t           manual_ddm;
	char				 ddm_host[100];
	uint32_t			 ddm_port;
	*/
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config;
#endif

	unsigned int num_handles;
	unsigned int request_limit;

	// for name-based connection
	uint16_t local_port;
	dante_name_t device_name;

	// for address-based connection
	unsigned int num_addresses;
	unsigned int addresses[DR_TEST_MAX_INTERFACES];

	//for domain based addr
	unsigned int domain_device_addr;
	
	// for interface-aware / redundant name-based connection
	unsigned int num_local_interfaces;
	aud_interface_identifier_t local_interfaces[DR_TEST_MAX_INTERFACES];

	aud_bool_t automatic_update_on_state_change;
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	uint16_t mdns_server_port;
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	union {
		uint16_t port_no;
		const char * socket_path;
	} domain_handler;
#endif
} dr_test_options_t;

typedef struct dr_test_request
{
	dante_request_id_t id;
	char description[DR_TEST_REQUEST_DESCRIPTION_LENGTH];
} dr_test_request_t;


typedef struct
{
	dr_test_options_t options;

	dapi_t * dapi;
	aud_env_t * env;
	dante_runtime_t * runtime;
	dante_domain_handler_t * handler;

	dr_devices_t * devices;

	dr_device_t * device;

	uint16_t nintf;
	uint16_t ntx;
	uint16_t nrx;
	uint16_t max_txflow_slots;
	uint16_t max_rxflow_slots;

	dr_txchannel_t ** tx;
	dr_rxchannel_t ** rx;

	uint16_t txlabels_buflen;
	dr_txlabel_t * txlabels_buf;

	dr_test_request_t requests[DR_TEST_MAX_REQUESTS];

} dr_test_t;


// Static buffers: save  stack memory by sharing these buffers
aud_errbuf_t g_test_errbuf;
char g_input_buf[BUFSIZ];

// callback functions
static dr_device_changed_fn dr_test_on_device_changed;
static dr_device_response_fn dr_test_on_response;

static ddh_change_event_fn dr_test_event_handle_ddh_changes;

//local definitions
static aud_error_t
dr_test_open(dr_test_t * test);

//----------------------------------------------------------
// Request management
//----------------------------------------------------------

static dr_test_request_t *
dr_test_allocate_request
(
	dr_test_t * test,
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
	DR_TEST_ERROR("error allocating request '%s': no more requests\n", description);
	return NULL;
}

static void
dr_test_request_release
(
	dr_test_request_t * request
) {
	request->id = DANTE_NULL_REQUEST_ID;
	request->description[0] = '\0';
}

void
dr_test_on_response
(
	dr_device_t * device,
	dante_request_id_t request_id,
	aud_error_t result
) {
	unsigned int i;
	dr_test_t * test = (dr_test_t *)  dr_device_get_context(device);

	for (i = 0; i < DR_TEST_MAX_REQUESTS; i++)
	{
		if (test->requests[i].id == request_id)
		{
			DR_TEST_PRINT("\nEVENT: completed request %p (%s) with result %s\n", 
				request_id, test->requests[i].description, dr_error_message(result, g_test_errbuf));
			dr_test_request_release(test->requests+i);
			return;
		}
	}
	DR_TEST_ERROR("\nEVENT: completed unknown request %p\n", request_id);
}


static void
dr_test_cancel
(
	dr_test_t * test
)
{
	// This code example cancels all outstanding requests, returning their
	// resources to the pool.

	unsigned int i;
	for (i = 0; i < DR_TEST_MAX_REQUESTS; i++)
	{
		if (test->requests[i].id != DANTE_NULL_REQUEST_ID)
		{
			if (test->requests[i].description[0])
			{
				DR_TEST_PRINT("Cancelling #%u: %s\n", i, test->requests[i].description);
			}
			else
			{
				DR_TEST_PRINT("Cancelling #%u\n", i);
			}
			dr_device_cancel_request(test->device, test->requests[i].id);
			dr_test_request_release(test->requests + i);
		}
	}
}


//----------------------------------------------------------
// State management and basic functionality
//----------------------------------------------------------

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
static aud_error_t
dr_test_set_current_domain_by_uuid
(
	dr_test_t * test,
	dante_domain_uuid_t uuid
) {
	dante_domain_handler_t * handler = dapi_get_domain_handler(test->dapi);
	
	aud_error_t result = dante_domain_handler_set_current_domain_by_uuid(handler, uuid);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting active_domain: %s\n",
			dr_error_message(result, g_test_errbuf));
		return result;
	}
	dapi_utils_print_domain_handler_info(test->handler);
	return AUD_SUCCESS;
}

static aud_error_t
dr_test_set_current_domain_by_name
(
	dr_test_t * test,
	const char * name
) {
	dante_domain_handler_t * handler = dapi_get_domain_handler(test->dapi);
	
	aud_error_t result = dante_domain_handler_set_current_domain_by_name(handler, name);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting active_domain: %s\n",
			dr_error_message(result, g_test_errbuf));
		return result;
	}
	dapi_utils_print_domain_handler_info(test->handler);
	return AUD_SUCCESS;
}
#endif

void dr_test_event_handle_ddh_changes
(
	const ddh_changes_t * changes
 ) {

	dapi_utils_print_domain_changes(changes);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	aud_error_t result;
	dante_domain_handler_t * handler = ddh_changes_get_domain_handler(changes);
	ddh_change_flags_t change_flags = ddh_changes_get_change_flags(changes);
	dr_test_t * test = dante_domain_handler_get_context(handler);


	if (change_flags & DDH_CHANGE_FLAG_AVAILABLE_DOMAINS && test->options.ddm_config.domain[0])
	{
		dante_domain_info_t info = dante_domain_handler_get_current_domain(handler);
		if (strcmp(info.name, test->options.ddm_config.domain))
		{
			result = dapi_utils_update_ddh_current_domain(handler, test->options.ddm_config.domain);
			if (result != AUD_SUCCESS)
			{
				return;
			}
			info = dante_domain_handler_get_current_domain(handler);
			//open the device if not yet opened.
			if (!test->device && !strcmp(info.name, test->options.ddm_config.domain))
			{
				dr_test_open(test);
			}
		}
	}
#endif
}

static aud_error_t
dr_test_query_capabilities
(
	dr_test_t * test
) {
	aud_error_t result;
	dr_test_request_t * request = dr_test_allocate_request(test, "QueryCapabilities");
	if (!request)
	{
		return AUD_ERR_NOBUFS;
	}

	result = dr_device_query_capabilities(test->device, &dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending query capabilities: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return result;
	}
	return result;
}

static void
dr_test_mark_stale
(
	dr_test_t * test,
	dr_device_component_t component,
	dante_id_t subcomponent
) {
	if (component == DR_DEVICE_COMPONENT_COUNT)
	{
		DR_TEST_DEBUG("Invalidating ALL components\n");
		for (component = 0; component < DR_DEVICE_COMPONENT_COUNT; component++)
		{
			dr_device_mark_component_stale(test->device, component);
		}
	}
	else if (subcomponent == 0)
	{
		DR_TEST_DEBUG("Invalidating component %s\n", dr_device_component_to_string(component));
		dr_device_mark_component_stale(test->device, component);
	}
	else
	{
		if (component == DR_DEVICE_COMPONENT_TXCHANNELS)
		{
			if (subcomponent <= test->ntx)
			{
				DR_TEST_DEBUG("Invalidating txchannel %u\n", subcomponent);
				dr_txchannel_mark_stale(test->tx[subcomponent-1]);
			}
			else
			{
				DR_TEST_ERROR("Invalid txchannel %u\n", subcomponent);
			}
		}
		else if (component == DR_DEVICE_COMPONENT_RXCHANNELS)
		{
			if (subcomponent <= test->nrx)
			{
				DR_TEST_DEBUG("Invalidating rxchannel %u\n", subcomponent);
				dr_rxchannel_mark_stale(test->rx[subcomponent-1]);
			}
			else
			{
				DR_TEST_ERROR("Invalid rxchannel %u\n", subcomponent);
			}
		}
		else if (component == DR_DEVICE_COMPONENT_TXLABELS)
		{
			aud_error_t result = dr_device_mark_txlabel_stale(test->device, subcomponent);
			if (result == AUD_SUCCESS)
			{
				DR_TEST_DEBUG("Invalidating txlabel %u\n", subcomponent);
			}
			else
			{
				DR_TEST_ERROR("Error invalidating txlabel %u: %s\n", subcomponent, aud_error_message(result, g_test_errbuf));
			}
		}
		else
		{
			DR_TEST_DEBUG("Per-subcomponent stale marking not supported for component  %s\n", dr_device_component_to_string(component));
		}
	}
}

static aud_error_t
dr_test_update
(
	dr_test_t * test
) {
	aud_error_t result;
	dr_device_component_t c;
	for (c = 0; c < DR_DEVICE_COMPONENT_COUNT; c++)
	{
		dr_test_request_t * request;
		if (!dr_device_is_component_stale(test->device, c))
		{
			continue;
		}
		DR_TEST_DEBUG("Updating stale component %s\n", dr_device_component_to_string(c));

		request = dr_test_allocate_request(test, NULL);
		if (!request)
		{
			return AUD_ERR_NOBUFS;
		}
		SNPRINTF(request->description, DR_TEST_REQUEST_DESCRIPTION_LENGTH, "Update %s", dr_device_component_to_string(c));

		result = dr_device_update_component(test->device, &dr_test_on_response, &request->id, c);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error sending update %s: %s\n",
				dr_device_component_to_string(c), dr_error_message(result, g_test_errbuf));
			dr_test_request_release(request);
			return result;
		}
	}
	return AUD_SUCCESS;
}

static aud_error_t
dr_test_update_rxflow_errors
(
	dr_test_t * test, 
	dante_rxflow_error_type_t field_type,
	aud_bool_t clear
) {
	aud_error_t result;
	dr_test_request_t * request;
	if (field_type == DANTE_NUM_RXFLOW_ERROR_TYPES)
	{
		DR_TEST_DEBUG("Updating rxflow error flags\n");
		request = dr_test_allocate_request(test, NULL);
		if (!request)
		{
			return AUD_ERR_NOBUFS;
		}
		SNPRINTF(request->description, DR_TEST_REQUEST_DESCRIPTION_LENGTH, "Update rxflow error flags");
		result = dr_device_update_rxflow_error_flags(test->device, &dr_test_on_response, &request->id, clear);
	}
	else
	{
		DR_TEST_DEBUG("Updating rxflow error field %s\n", dante_rxflow_error_type_to_string(field_type));
			request = dr_test_allocate_request(test, NULL);
		if (!request)
		{
			return AUD_ERR_NOBUFS;
		}
		SNPRINTF(request->description, DR_TEST_REQUEST_DESCRIPTION_LENGTH, "Update rx flow %s", dante_rxflow_error_type_to_string(field_type));
		result = dr_device_update_rxflow_error_fields(test->device, &dr_test_on_response, &request->id, field_type, clear);
	}
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error update rxflow errors : %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return result;
	}
	return AUD_SUCCESS;
}

static void
dr_test_on_device_state_changed
(
	dr_test_t * test
) {
	switch (dr_device_get_state(test->device))
	{
	case DR_DEVICE_STATE_RESOLVED:
		// query capabilities
		if (test->options.automatic_update_on_state_change)
		{
			DR_TEST_PRINT("device resolved, querying capabilities\n");
			dr_test_query_capabilities(test);
		}
		return;

	case DR_DEVICE_STATE_ACTIVE:
		{
			// update information
			dr_device_get_txchannels(test->device, &test->ntx, &test->tx);
			dr_device_get_rxchannels(test->device, &test->nrx, &test->rx);

			// update all components when we enter the active state, unless we have a strange status
			if (test->options.automatic_update_on_state_change)
			{
				dr_device_status_flags_t status_flags;
				aud_error_t result = dr_device_get_status_flags(test->device, &status_flags);
				if (result != AUD_SUCCESS)
				{
					DR_TEST_ERROR("Error getting status flags: %s\n", dr_error_message(result, g_test_errbuf));
					return;
				}
				if (status_flags)
				{
					DR_TEST_PRINT("device active, status=0x%08x, not updating\n", status_flags);
					return;
				}
				DR_TEST_PRINT("device active, updating\n");
				dr_test_update(test);
			}
			return;
		}
	case DR_DEVICE_STATE_ERROR:
		{
			aud_error_t error_state_error = dr_device_get_error_state_error(test->device);
			const char * error_state_action = dr_device_get_error_state_action(test->device);
	
			DR_TEST_PRINT("device has entered the ERROR state: error=%s, action='%s'\n",
				dr_error_message(error_state_error, g_test_errbuf), (error_state_action ? error_state_action : ""));
			return;
		}
	default:
		// nothing to do
		DR_TEST_PRINT("device state is now '%s'\n", dr_device_state_to_string(dr_device_get_state(test->device)));
	}
}


static void
dr_test_on_device_addresses_changed
(
	dr_test_t * test
) {
	unsigned int i, na = 2;
	dante_ipv4_address_t addresses[2];
	dr_device_get_addresses(test->device, &na, addresses);

	DR_TEST_PRINT("Addresses are now: ");
	for (i = 0; i < na; i++)
	{
		uint8_t * host = (uint8_t *) &(addresses[i].host);
		DR_TEST_PRINT("%c%u.%u.%u.%u:%u", 
			(i ? ',' : '['),
			host[0], host[1], host[2], host[3], addresses[i].port);
	}
	DR_TEST_PRINT("]\n");
}

//----------------------------------------------------------
// Connect
//----------------------------------------------------------

static aud_error_t
dr_test_open
(
	dr_test_t * test
) {
	dr_test_options_t * options = &test->options;
	aud_error_t result;
	dr_device_open_t * config = NULL;
	unsigned i;

	if (test->device)
	{
		result = AUD_ERR_ALREADY;
		goto l__error;
	}


	if (options->device_name[0])
	{
		config = dr_device_open_config_new(options->device_name);
		if (! config)
		{
			result = AUD_ERR_NOMEMORY;
			goto l__error;
		}
	
		dante_domain_uuid_t uuid = dr_devices_get_domain_uuid(test->devices);
		if (IS_ADHOC_DOMAIN_UUID(uuid)) // We only need interfaces for devices when in the ADHOC domain
		{
			if (options->num_local_interfaces)
			{
				DR_TEST_PRINT("Opening connection to remote device %s using %d interfaces\n", 
					options->device_name, options->num_local_interfaces);

				//result = dr_device_open_remote_on_interfaces(test->devices, 
				//	options->device_name, options->num_local_interface_indexes, options->local_interface_indexes, &test->device);

				for (i = 0; i < options->num_local_interfaces; i++)
				{
					if (options->local_interfaces[i].flags == AUD_INTERFACE_IDENTIFIER_FLAG_NAME)
					{
						dr_device_open_config_enable_interface_by_name(
							config, i, options->local_interfaces[i].name
						);
					}
					else if (options->local_interfaces[i].flags == AUD_INTERFACE_IDENTIFIER_FLAG_INDEX)
					{
						dr_device_open_config_enable_interface_by_index(
							config, i, options->local_interfaces[i].index
						);
					}
				}
			}
			else
			{
				DR_TEST_PRINT("Opening connection to remote device %s\n", options->device_name);
				//result = dr_device_open_remote(test->devices, options->device_name, &test->device);
			}
		}
		result = dr_device_open_with_config(test->devices, config, &test->device);

		dr_device_open_config_free(config);
		config = NULL;
	}
	else if (options->num_addresses)
	{
		config = dr_device_open_config_new(NULL);
		if (! config)
		{
			result = AUD_ERR_NOMEMORY;
			goto l__error;
		}

		DR_TEST_PRINT("Opening connection using %d explicit addresses\n", 
			options->num_addresses);
		//result = dr_device_open_addresses(test->devices, 
		//	options->num_addresses, options->addresses, &test->device);

		for (i = 0; i < options->num_addresses; i++)
		{
			if (options->addresses[i])
			{
				dr_device_open_config_enable_address(
					config, i, options->addresses[i], 0
				);
			}
		}
		result = dr_device_open_with_config(test->devices, config, &test->device);

		dr_device_open_config_free(config);
		config = NULL;
	}
	else if (options->domain_device_addr)
	{
		result = dr_device_open_domain_id(test->devices, options->domain_device_addr, &test->device);
	}
	else
	{
		if (options->local_port)
		{
			DR_TEST_PRINT("Opening connection to local device on port %d\n", options->local_port);
			result = dr_device_open_local_on_port(test->devices, options->local_port, &test->device);
		}
		else
		{
			DR_TEST_PRINT("Opening connection to local device\n");
			result = dr_device_open_local(test->devices, &test->device);
		}
	}
	if (result != AUD_SUCCESS)
	{
	l__error:
		DR_TEST_ERROR("Error creating device: %s\n",
			dr_error_message(result, g_test_errbuf));
		return result;
	}

	dr_device_set_context(test->device, test);
	dr_device_set_changed_callback(test->device, dr_test_on_device_changed);

	// if we are connecting locally we may need to trigger the next state transition,
	// if not this is a no-op
	dr_test_on_device_state_changed(test);

	return AUD_SUCCESS;
}

static aud_error_t
dr_test_close
(
	dr_test_t * test
) {
	if (test->device)
	{
		dr_device_close(test->device);
		test->device = NULL;
		test->nintf = 0;
		test->ntx = 0;
		test->nrx = 0;
		test->max_txflow_slots = 0;
		test->max_rxflow_slots = 0;
		test->tx = NULL;
		test->rx = NULL;
	}
	return AUD_SUCCESS;
}


//----------------------------------------------------------
// Utilities
//----------------------------------------------------------

typedef enum dr_test_read_port_option
{
	DR_TEST_READ_NO_PORT,
	DR_TEST_READ_OPTIONAL_PORT,
	DR_TEST_READ_REQUIRES_PORT,
} dr_test_read_port_option_t;

#if 0
/*
	Prompt for and read an IPv4 address

	If port is optional, then addr->port must be initialised to the default on the way in.
 */
static int
dr_test_read_address
(
	dante_ipv4_address_t * addr,
	dr_test_read_port_option_t has_port,
	const char * desc
)
{
	unsigned int ip1, ip2, ip3, ip4, port;
	int matches;

	if (has_port == DR_TEST_READ_NO_PORT)
	{
		DR_TEST_PRINT("Enter %s address (A.B.C.D):\n", desc);
	}
	else
	{
		DR_TEST_PRINT("Enter %s address. Legal values are:\n", desc);
		DR_TEST_PRINT("  A.B.C.D:PORT to specify an address and port\n");
		DR_TEST_PRINT("  A.B.C.D to specify an address and use the default port\n");
	}
	if (! fgets(g_input_buf, BUFSIZ, stdin))
		return -1;

	matches = sscanf(g_input_buf, "%u.%u.%u.%u:%u", &ip1, &ip2, &ip3, &ip4, &port);
	if (matches < 4)
	{
		return -1;
	}

	if (ip1 > 0xff || ip2 > 0xff || ip3 > 0xff || ip4 > 0xff)
		return -1;

	addr->host = htonl((ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4);
	if (matches == 5)
	{
		if (has_port == DR_TEST_READ_NO_PORT)
		{
			return -1;
		}
		else if (port > 0xFFFF)
		{
			return -1;
		}
		addr->port = (uint16_t) port;
		return 0;
	}
	else
	{
		if (has_port == DR_TEST_READ_REQUIRES_PORT)
		{
			return -1;
		}
		return 0;
	}
}
#endif


//----------------------------------------------------------
// Device-level configuration and settings
//----------------------------------------------------------

static void
dr_test_ping
(
	dr_test_t * test
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Ping device\n");

	request = dr_test_allocate_request(test, "Ping");
	if (!request)
	{
		return;
	}
	result = dr_device_ping(test->device, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending ping: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_store_config
(
	dr_test_t * test
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: store config\n");

	request = dr_test_allocate_request(test, "StoreConfig");
	if (!request)
	{
		return;
	}

	result = dr_device_store_config(test->device, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending store config: %s\n",
			dr_error_message(result, g_test_errbuf));
		return;
	}
}

static void
dr_test_clear_config(dr_test_t * test)
{
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Clear config\n");

	request = dr_test_allocate_request(test, "ClearConfig");
	if (!request)
	{
		return;
	}

	result = dr_device_clear_config(test->device, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending clear config: %s\n",
			dr_error_message(result, g_test_errbuf));
		return;
	}
}

static aud_error_t
dr_test_rename
(
	dr_test_t * test,
	const char * new_name,
	aud_bool_t close
) {
	aud_error_t result;

	DR_TEST_DEBUG("ACTION: rename\n");

	if (new_name && !dante_name_is_valid_device_name(new_name))
	{
		DR_TEST_ERROR("Error: '%s' is not a valid dante device name\n", new_name);
		return AUD_ERR_INVALIDDATA;
	}
	if (close)
	{
		result = dr_device_close_rename(test->device, new_name);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error renaming device to '%s': %s\n",
				(new_name ? new_name : ""), dr_error_message(result, g_test_errbuf));
			return result;
		}
		DR_TEST_PRINT("Device renamed, handle closed, exiting\n");
		test->device = NULL;
	}
	else
	{
		dr_test_request_t * request = dr_test_allocate_request(test, "RenameDevice");
		if (!request)
		{
			return AUD_ERR_NOBUFS;
		}
		result = dr_device_rename(test->device, dr_test_on_response, &request->id, new_name);
	
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error renaming device to '%s': %s\n",
				(new_name ? new_name : ""), dr_error_message(result, g_test_errbuf));
			return result;
		}
	}
	return AUD_SUCCESS;
}

typedef enum
{
	DR_TEST_PERFORMANCE_TX,
	DR_TEST_PERFORMANCE_RX,
	DR_TEST_PERFORMANCE_UNICAST
} dr_test_performance_t;

const char * DR_TEST_PERFORMANCE_NAMES[] =
{
	"tx",
	"rx",
	"unicast"
};

static void
dr_test_set_performance
(
	dr_test_t * test,
	dr_test_performance_t performance,
	dante_latency_us_t latency_us,
	dante_fpp_t fpp
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("Setting %s performance\n", DR_TEST_PERFORMANCE_NAMES[performance]);

	request = dr_test_allocate_request(test, DR_TEST_PERFORMANCE_NAMES[performance]);
	if (!request)
	{
		return;
	}
	
	if (performance == DR_TEST_PERFORMANCE_TX)
	{
		result = dr_device_set_tx_performance_us(test->device,
			latency_us, fpp, dr_test_on_response, & request->id);
	}
	else if (performance == DR_TEST_PERFORMANCE_RX)
	{
		result = dr_device_set_rx_performance_us(test->device, 
			latency_us, fpp, dr_test_on_response, & request->id);
	}
	else
	{
		result = dr_device_set_unicast_performance_us(test->device,
			latency_us, fpp, dr_test_on_response, & request->id);
	}

	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting %s performance properties: %s\n",
			DR_TEST_PERFORMANCE_NAMES[performance], dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_set_lockdown
(
	dr_test_t * test,
	aud_bool_t lockdown
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set lockdown\n");

	request = dr_test_allocate_request(test, "SetLockdown");
	if (!request)
	{
		return;
	}
	
	result = dr_device_set_lockdown(test->device,
		lockdown, dr_test_on_response, & request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting device lockdown: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}


static void
dr_test_set_network_loopback
(
	dr_test_t * test,
	aud_bool_t enabled
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set Network Loopback\n");

	request = dr_test_allocate_request(test, "SetNetworkLoopback");
	if (!request)
	{
		return;
	}
	
	result = dr_device_set_network_loopback(test->device, enabled, dr_test_on_response, & request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error %s network loopback (request): %s\n",
			(enabled ? "enabling" : "disabling"), dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_set_aes67_prefix
(
	dr_test_t * test,
	uint32_t aes67_prefix
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set AES67 prefix\n");

	request = dr_test_allocate_request(test, "SetAES67Prefix");
	if (!request)
	{
		return;
	}

	result = dr_device_set_aes67_mcast_prefix(test->device, dr_test_on_response, &request->id, aes67_prefix);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error %s AES67 prefix (request): %s\n",
			(aes67_prefix ? "enabling" : "disabling"), dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

//----------------------------------------------------------
// Channel actions
//----------------------------------------------------------

static void
dr_test_batch_rxchannel_subscribe
(
	dr_test_t * test,
	const char * filename
) {
	aud_error_t result;
	dr_test_request_t * request;

	FILE * fp;

	DR_TEST_DEBUG("ACTION: Batch Subscribe\n");

	if (!filename || !filename[0])
	{
		DR_TEST_ERROR("Invalid filename\n");
		return;
	}

	fp = fopen(filename, "r");
	if (!fp)
	{
		DR_TEST_ERROR("Error opening filename '%s'\n", filename);
		return;
	}
	g_batch.n = 0;
	while (!feof(fp) && g_batch.n < DR_TEST_MAX_BATCH)
	{
		char line[128], channel[128], device[128], c;
		unsigned int id;
		if (!fgets(line, sizeof(line), fp))
		{
			break;
		}
		if (sscanf(line, "%u \"%[^\"\r\n@]@%[^\"\r\n]%c", &id, channel, device, &c) == 4 && c == '"')
		{
			g_batch._.subscriptions[g_batch.n].rxchannel_id = (dante_id_t) id;
			aud_strlcpy(g_batch._.subscriptions[g_batch.n].channel, channel, DANTE_NAME_LENGTH);
			aud_strlcpy(g_batch._.subscriptions[g_batch.n].device, device, DANTE_NAME_LENGTH);
			g_batch.n++;
		}
		else if (sscanf(line, "%u %s", &id, channel) == 2)
		{
			DR_TEST_ERROR("Invalid line in file \"%s\": \"%s\"\n", filename, line);
			fclose(fp);
			return;
		}
		else if (sscanf(line, "%u", &id) == 1)
		{
			g_batch._.subscriptions[g_batch.n].rxchannel_id = (dante_id_t) id;
			g_batch._.subscriptions[g_batch.n].channel[0] = '\0';
			g_batch._.subscriptions[g_batch.n].device[0] = '\0';
			g_batch.n++;
		}
		else
		{
			DR_TEST_ERROR("Invalid line in file \"%s\": \"%s\"\n", filename, line);
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	if (!g_batch.n)
	{
		DR_TEST_ERROR("No valid configuration in file \"%s\"\n", filename);
		return;
	}

	request = dr_test_allocate_request(test, "BatchSubscribe");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting %d RX subscriptions\n", g_batch.n);
	result = dr_device_batch_subscribe(test->device, dr_test_on_response, &request->id, g_batch.n, g_batch._.subscriptions);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending batch rx channel subscription message: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}

}

static void
dr_test_rxchannel_subscribe
(
	dr_test_t * test,
	unsigned int channel,
	char * name
) {
	aud_error_t result;
	
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Subscribe\n");

	if (channel < 1 || channel > test->nrx)
	{
		DR_TEST_ERROR("Invalid RX channel (must be in range 1..%u)\n", test->nrx);
		return;
	}
	
	request = dr_test_allocate_request(test, "Subscribe");
	if (!request)
	{
		return;
	}

	if (name != NULL)
	{
		char * c = name;
		char * r = strchr(name, '@');
		if (r == NULL)
		{
			DR_TEST_ERROR("Invalid subscription name (NAME must be in the form \"channel@device\")\n");
			dr_test_request_release(request);
			return;
		}
		*r = '\0';
		r++;
		result = dr_rxchannel_subscribe(test->rx[channel-1], dr_test_on_response, &request->id, r, c);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error sending subscribe message: %s\n",
				dr_error_message(result, g_test_errbuf));
			dr_test_request_release(request);
			return;
		}
	}
	else
	{
		result = dr_rxchannel_subscribe(test->rx[channel-1], dr_test_on_response, &request->id, NULL, NULL);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error sending unsubscribe message: %s\n",
				dr_error_message(result, g_test_errbuf));
			dr_test_request_release(request);
			return;
		}
	}
}

static void
dr_test_txchannel_set_enabled
(
	dr_test_t * test,
	unsigned int channel,
	aud_bool_t enabled
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set TxChannel Enabled\n");

	if (channel < 1 || channel > test->ntx)
	{
		DR_TEST_PRINT("Invalid TX channel (must be in range 1..%u)\n", test->ntx);
		return;
	}

	request = dr_test_allocate_request(test, "SetTxChannelEnabled");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting TX channel %u to be %s\n", channel, (enabled ? "enabled" : "disabled"));
	result = dr_txchannel_set_enabled(test->tx[channel-1], dr_test_on_response, &request->id, enabled);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending set enabled message: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_txchannel_set_signal_reflevel
(
	dr_test_t * test,
	unsigned int channel,
	dante_dbu_t dbu
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set TxChannel SignalRefLevel\n");

	if (channel > test->ntx)
	{
		DR_TEST_PRINT("Invalid TX channel (must be in range 1..%u)\n", test->ntx);
		return;
	}

	request = dr_test_allocate_request(test, "SetTxChannelSignalRefLevel");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting TX channel %u signal reference level to ", channel);
	if (dbu == DANTE_DBU_UNSET)
	{
		DR_TEST_PRINT("unset");
	}
	else
	{
		DR_TEST_PRINT("%+d dBu\n", (int) dbu);
	}
	if (channel == 0)
	{
		result = dr_device_set_txchannel_signal_reflevel(test->device, dr_test_on_response, &request->id, dbu);
	}
	else
	{
		result = dr_txchannel_set_signal_reflevel(test->tx[channel-1], dr_test_on_response, &request->id, dbu);
	}
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending set signal reference level message: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_txchannel_clear_signal_reflevel
(
	dr_test_t * test,
	unsigned int channel
)
{
	dr_test_txchannel_set_signal_reflevel(test, channel, DANTE_DBU_UNSET);
}

static void
dr_test_add_txlabel
(
	dr_test_t * test,
	unsigned int channel,
	char * name
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Add Tx Label\n");

	if (channel < 1 || channel > test->ntx)
	{
		DR_TEST_ERROR("Invalid TX channel (must be in range 1..%u)\n", test->ntx);
		return;
	}

	if (!dante_name_is_valid_channel_or_label_name(name))
	{
		DR_TEST_ERROR("Invalid TX label '%s'\n", name);
		return;
	}

	request = dr_test_allocate_request(test, "AddTxLabel");
	if (!request)
	{
		return;
	}

	DR_TEST_DEBUG("Adding label \"%s\" to tx channel %u\n", name, channel);
	result = dr_txchannel_add_txlabel(test->tx[channel-1],
		dr_test_on_response, &request->id, name, DR_MOVEFLAG_MOVE_EXISTING);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending add label \"%s\" message to tx channel %u: %s\n",
			name, channel, dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_remove_txlabel_from_channel
(
	dr_test_t * test,
	dante_id_t channel,
	char * name
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Remove Tx Label From Channel\n");

	if (channel < 1 || channel > test->ntx)
	{
		DR_TEST_ERROR("Invalid TX channel (must be in range 1..%u)\n", test->ntx);
		return;
	}

	request = dr_test_allocate_request(test, "RemoveTxLabelFromChannel");
	if (!request)
	{
		return;
	}

	DR_TEST_DEBUG("Removing label \"%s\" from tx channel %u\n", name, channel);
	result = dr_txchannel_remove_txlabel(test->tx[channel-1],
		dr_test_on_response, &request->id, name);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending remove label \"%s\" message to tx channel %u: %s\n",
			name, channel, dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}


static void
dr_test_remove_txlabel
(
	dr_test_t * test,
	dante_id_t label_id
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Remove Tx Label\n");

	if (dr_device_txlabel_with_id(test->device, label_id, NULL) == AUD_ERR_RANGE)
	{
		DR_TEST_ERROR("Invalid TX label\n");
		return;
	}

	request = dr_test_allocate_request(test, "RemoveTxLabel");
	if (!request)
	{
		return;
	}

	DR_TEST_DEBUG("Removing TX label %u\n", label_id);
	result = dr_device_remove_txlabel_with_id(test->device,
		dr_test_on_response, &request->id, label_id
	);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending remove label %u message: %s\n",
			label_id, dr_error_message(result, g_test_errbuf)
		);
		dr_test_request_release(request);
		return;
	}
}


static void
dr_test_txchannel_set_muted
(
	dr_test_t * test,
	unsigned int channel,
	aud_bool_t muted
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set Tx Channel Muted\n");

	if (channel < 1 || channel > test->ntx)
	{
		DR_TEST_PRINT("Invalid TX channel (must be in range 1..%u)\n", test->ntx);
		return;
	}

	request = dr_test_allocate_request(test, "SetTxChannelMuted");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting TX channel %u to be %s\n", channel, (muted ? "muted" : "unmuted"));
	result = dr_txchannel_set_muted(test->tx[channel-1], dr_test_on_response, &request->id, muted);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending %s message: %s\n",
			(muted ? "mute" : "unmute"), dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_rxchannel_set_muted
(
	dr_test_t * test,
	unsigned int channel,
	aud_bool_t muted
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set Rx Channel Muted\n");

	if (channel < 1 || channel > test->nrx)
	{
		DR_TEST_PRINT("Invalid RX channel (must be in range 1..%u)\n", test->nrx);
		return;
	}

	request = dr_test_allocate_request(test, "SetRxChannelMuted");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting RX channel %u to be %s\n", channel, (muted ? "muted" : "unmuted"));
	result = dr_rxchannel_set_muted(test->rx[channel-1], dr_test_on_response, &request->id, muted);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending %s message: %s\n",
			(muted ? "mute" : "unmute"), dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}


static void
dr_test_rxchannel_set_name
(
	dr_test_t * test,
	unsigned int channel,
	const char * name
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set Rx Channel Name\n");

	if (channel < 1 || channel > test->nrx)
	{
		DR_TEST_ERROR("Invalid RX channel (must be in range 1..%u)\n", test->nrx);
		return;
	}

	if (name && !dante_name_is_valid_channel_or_label_name(name))
	{
		DR_TEST_ERROR("Invalid channel name '%s'\n", name);
		return;
	}

	request = dr_test_allocate_request(test, "SetRxChannelName");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting RX channel %u name to \"%s\"\n", channel, name);
	result = dr_rxchannel_set_name(test->rx[channel-1], dr_test_on_response, &request->id, name);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending rx channel rename message: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_batch_rxchannel_set_name
(
	dr_test_t * test,
	const char * filename
) {
	aud_error_t result;
	dr_test_request_t * request;

	FILE * fp;

	DR_TEST_DEBUG("ACTION: Batch Set Rx Channel Names\n");

	if (!filename || !filename[0])
	{
		DR_TEST_ERROR("Invalid filename\n");
		return;
	}

	fp = fopen(filename, "r");
	if (!fp)
	{
		DR_TEST_ERROR("Error opening filename '%s'\n", filename);
		return;
	}
	g_batch.n = 0;
	while (!feof(fp) && g_batch.n < DR_TEST_MAX_BATCH)
	{
		char line[128], name[128];
		unsigned int c;
		if (!fgets(line, sizeof(line), fp))
		{
			break;
		}
		if (sscanf(line, "%u %s", &c, name) == 2)
		{
			g_batch._.rxlabels[g_batch.n].rxchannel_id = (dante_id_t) c;
			aud_strlcpy(g_batch._.rxlabels[g_batch.n].label, name, DANTE_NAME_LENGTH);
			g_batch.n++;
		}
		else if (sscanf(line, "%u", &c) == 1)
		{
			g_batch._.rxlabels[g_batch.n].rxchannel_id = (dante_id_t) c;
			g_batch._.rxlabels[g_batch.n].label[0] = '\0';
			g_batch.n++;
		}
		else
		{
			DR_TEST_ERROR("Invalid line in file \"%s\": \"%s\"\n", filename, line);
			fclose(fp);
			return;
		}
	}
	fclose(fp);
	if (!g_batch.n)
	{
		DR_TEST_ERROR("No valid configuration in file \"%s\"\n", filename);
		return;
	}

	request = dr_test_allocate_request(test, "BatchSetRxChannelName");
	if (!request)
	{
		return;
	}

	DR_TEST_PRINT("Setting %d RX channel names\n", g_batch.n);
	result = dr_device_batch_rxlabel(test->device, dr_test_on_response, &request->id, g_batch.n, g_batch._.rxlabels);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending batch rx channel rename message: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

//----------------------------------------------------------
// Transmit Flows
//----------------------------------------------------------

static void
dr_test_add_txflow
(
	dr_test_t * test,
	dante_id_t flow_id,
	uint16_t num_slots,
	dante_latency_us_t latency_us,
	dante_fpp_t fpp
) {
	dante_name_t name;
	uint16_t i;
	dr_txflow_config_t * config = NULL;
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Add Tx Flow\n");

	DR_TEST_PRINT("Enter flow name (or simply 'enter' to use default name): ");
	fgets(g_input_buf, BUFSIZ, stdin);
	if (sscanf(g_input_buf, "%s", name) < 1)
	{
		name[0] = '\0';
	}

	result = dr_txflow_config_new(test->device, flow_id, num_slots, &config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating txflow config: %s\n", dr_error_message(result, g_test_errbuf));
		return;
	}

	result = dr_txflow_config_set_name(config, name);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting txflow config name: %s\n", dr_error_message(result, g_test_errbuf));
		dr_txflow_config_discard(config);
		return;
	}

	result = dr_txflow_config_set_latency_us(config, latency_us);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting txflow config latency: %s\n", dr_error_message(result, g_test_errbuf));
		dr_txflow_config_discard(config);
		return;
	}

	result = dr_txflow_config_set_fpp(config, fpp);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting txflow config fpp: %s\n", dr_error_message(result, g_test_errbuf));
		dr_txflow_config_discard(config);
		return;
	}

	
	// Slots
	DR_TEST_PRINT("Enter 1-based tx channel index to add to slots (or simply 'enter' to have an empty slot):\n");
	for (i = 0; i < num_slots; i++)
	{
		unsigned int id;
		DR_TEST_PRINT("slot %d (of %d): ", i, num_slots);
		fgets(g_input_buf, BUFSIZ, stdin);
		if (sscanf(g_input_buf, "%u", &id) == 1)
		{
			dr_txchannel_t * tx = dr_device_txchannel_with_id(test->device, (dante_id_t) id);
			if (tx)
			{
				result = dr_txflow_config_add_channel(config, tx, i);
				if (result != AUD_SUCCESS)
				{
					DR_TEST_ERROR("Error setting txflow config slot: %s\n",
						dr_error_message(result, g_test_errbuf));
					dr_txflow_config_discard(config);
					return;
				}
			}
		}
	}

	request = dr_test_allocate_request(test, "AddTxFlow");
	if (!request)
	{
		return;
	}

	result = dr_txflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending txflow create request: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_replace_txflow_channels
(
	dr_test_t * test,
	dante_id_t id
) {
	uint16_t s, num_slots;
	dr_txflow_t * flow = NULL;
	dr_txflow_config_t * config = NULL;
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Replace Tx Flow Channels\n");

	result = dr_device_txflow_with_id(test->device, id, &flow);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error getting txflow at index %d: %s\n",
			id, dr_error_message(result, g_test_errbuf));
		return;
	}
	result = dr_txflow_replace_channels(flow, &config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating txflow modify config: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_txflow_release(&flow);
		return;
	}
	dr_txflow_release(&flow);
	num_slots = dr_txflow_config_num_slots(config);

	DR_TEST_PRINT("Enter 1-based tx channel id to add to slots (or simply 'enter' to have an empty slot):\n");
	for (s = 0; s < num_slots; s++)
	{
		unsigned int  channel_id;
		DR_TEST_PRINT("Slot %d/%d: ", s, num_slots);
		fgets(g_input_buf, BUFSIZ, stdin);
		if (sscanf(g_input_buf, "%u", &channel_id) == 1)
		{
			dr_txchannel_t * tx = dr_device_txchannel_with_id(test->device, (dante_id_t) channel_id);
			if (tx)
			{
				result = dr_txflow_config_add_channel(config, tx, s);
				if (result != AUD_SUCCESS)
				{
					DR_TEST_ERROR("Error setting txflow config slot: %s\n",
						dr_error_message(result, g_test_errbuf));
					dr_txflow_config_discard(config);
					return;
				}
			}
		}
	}

	request = dr_test_allocate_request(test, "ReplaceTxFlowChannels");
	if (!request)
	{
		return;
	}

	result = dr_txflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending modify txflow request: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_delete_txflow
(
	dr_test_t * test,
	dante_id_t id
) {
	dr_txflow_t * flow = NULL;
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Delete Tx Flow\n");


	result = dr_device_txflow_with_id(test->device, id, &flow);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error getting txflow %d: %s\n",
			id, dr_error_message(result, g_test_errbuf));
		return;
	}

	request = dr_test_allocate_request(test, "DeleteTxFlow");
	if (!request)
	{
		return;
	}
	result = dr_txflow_delete(&flow, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending delete txflow request for flow %d: %s\n",
			id, dr_error_message(result, g_test_errbuf));
		dr_txflow_release(&flow);
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_add_aes67_txflow_multicast
(
	dr_test_t * test,
	dante_id_t flow_id,
	uint16_t num_slots
) {
	dante_name_t name;
	uint16_t i;
	dr_txflow_config_t * config = NULL;
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Add Tx Flow\n");

	DR_TEST_PRINT("Enter flow name (or simply 'enter' to use default name): ");
	fgets(g_input_buf, BUFSIZ, stdin);
	if (sscanf(g_input_buf, "%s", name) < 1)
	{
		name[0] = '\0';
	}

	result = dr_txflow_config_new_aes67_multicast(test->device, flow_id, num_slots, &config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating txflow config: %s\n", dr_error_message(result, g_test_errbuf));
		return;
	}

	result = dr_txflow_config_set_name(config, name);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error setting txflow config name: %s\n", dr_error_message(result, g_test_errbuf));
		dr_txflow_config_discard(config);
		return;
	}

	// Slots
	DR_TEST_PRINT("Enter 1-based tx channel index to add to slots (or simply 'enter' to have an empty slot):\n");
	for (i = 0; i < num_slots; i++)
	{
		unsigned int id;
		DR_TEST_PRINT("slot %d (of %d): ", i, num_slots);
		fgets(g_input_buf, BUFSIZ, stdin);
		if (sscanf(g_input_buf, "%u", &id) == 1)
		{
			dr_txchannel_t * tx = dr_device_txchannel_with_id(test->device, (dante_id_t) id);
			if (tx)
			{
				result = dr_txflow_config_add_channel(config, tx, i);
				if (result != AUD_SUCCESS)
				{
					DR_TEST_ERROR("Error setting txflow config slot: %s\n",
						dr_error_message(result, g_test_errbuf));
					dr_txflow_config_discard(config);
					return;
				}
			}
		}
	}

	request = dr_test_allocate_request(test, "AddTxFlow");
	if (!request)
	{
		return;
	}

	result = dr_txflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending txflow create request: %s\n",
			dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

//----------------------------------------------------------
// Rx Flows 
//----------------------------------------------------------

static void 
dr_test_read_multicast_associations
(
	dr_test_t * test,
	dr_rxflow_config_t * config
) {
	aud_error_t result;
	unsigned int i, n = dr_device_max_rxflow_slots(test->device);

	DR_TEST_PRINT("Multicast template flow associations are configured by choosing transmit channels and\n");
	DR_TEST_PRINT("Selecting one or more receive channels to be subscribed to each of those channels.\n");
	DR_TEST_PRINT("A maximum of of %d subscriptions are allowed for this device.\n", n);

	for (i = 0; i < n; i++)
	{
		dante_name_t subscribed_channel;
		unsigned int rxchannel_id;
		DR_TEST_PRINT("  Enter transmit channel name, slot %u of %u (ENTER to finish):",
			(i+1), n);
		fgets(g_input_buf, BUFSIZ, stdin);

		subscribed_channel[0] = '\0';
		sscanf(g_input_buf, "%[^\r\n]", subscribed_channel);
		if (subscribed_channel[0] == '\0')
		{
			break;
		}

		// parse ids
		do 
		{
			dr_rxchannel_t * rxc = NULL;
			DR_TEST_PRINT("    Receive channel id (ENTER to finish): ");
			fgets(g_input_buf, BUFSIZ, stdin);
			if (sscanf(g_input_buf, "%u", &rxchannel_id) != 1)
			{
				break;
			}
			if (rxchannel_id == 0 || rxchannel_id > test->nrx)
			{
				DR_TEST_PRINT("Invalid rx channel id %u\n", rxchannel_id);
				continue;
			}
			rxc = test->rx[rxchannel_id-1];
			result = dr_rxflow_config_add_associated_channel(config, rxc, subscribed_channel);
			if (result != AUD_SUCCESS)
			{
				DR_TEST_PRINT("Error adding template association channel %d subscribed to %s@%s: %s\n",
					rxchannel_id, subscribed_channel, dr_rxflow_config_get_tx_device_name(config),
					dr_error_message(result, g_test_errbuf));
				continue;
			}
		} while (rxchannel_id);
	}
}

static void 
dr_test_add_rxflow_multicast
(
	dr_test_t * test,
	unsigned int flow_id
) {
	dante_name_t tx_device_name;
	dante_name_t tx_flow_name;
	aud_error_t result;
	dr_test_request_t * request;
	dr_rxflow_config_t * config = NULL;

	DR_TEST_DEBUG("ACTION: Add Multicast Rx Flow Template\n");

	DR_TEST_PRINT("Enter the multicast transmit flow and device name in the form \"flow@device\": ");
	fgets(g_input_buf, BUFSIZ, stdin);
	if (sscanf(g_input_buf, "\"%[^@]@%[^\r\n\"]\"", tx_flow_name, tx_device_name) != 2)
	{
		printf("Invalid flow name.");
		return;
	}

	result = dr_rxflow_config_new_multicast(test->device,
		(dante_id_t) flow_id, tx_device_name, tx_flow_name,
		&config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating multicast config object: %s\n", dr_error_message(result, g_test_errbuf));
		return;
	}

	dr_test_read_multicast_associations(test, config);

	request = dr_test_allocate_request(test, "AddMulticastRxFlowTemplate");
	if (!request)
	{
		return;
	}
	result = dr_rxflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending multicast template create request: %s\n", dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}


static void
dr_test_read_unicast_associations
(
	dr_test_t * test,
	dr_rxflow_config_t * config
) {
	aud_error_t result;
	uint16_t i, num_slots = dr_rxflow_config_num_slots(config);

	DR_TEST_PRINT("Unicast template flow associations are configured by choosing a receive channel and\n");
	DR_TEST_PRINT("setting the transmit channel to which it will subscribe.\n");
	DR_TEST_PRINT("A maximum of of %d subscriptions are allowed for this device.\n", num_slots);
	
	for (i = 0; i < num_slots;)
	{
		dante_name_t subscribed_channel;
		unsigned int rxchannel_id;
		dr_rxchannel_t * rxc = NULL;
		
		DR_TEST_PRINT("Next receive channel id (hit ENTER to finish):");
		fgets(g_input_buf, BUFSIZ, stdin);
		if (sscanf(g_input_buf, "%u", &rxchannel_id) != 1)
		{
			break;
		}
		if (rxchannel_id == 0 || rxchannel_id > test->nrx)
		{
			DR_TEST_PRINT("Invalid rx channel id %u\n", rxchannel_id);
			continue;
		}
		rxc = test->rx[rxchannel_id-1];

		DR_TEST_PRINT("Transmit channel for rxchannel %u:", rxchannel_id);
		fgets(g_input_buf, BUFSIZ, stdin);
		sscanf(g_input_buf, "%[^\"\r\n]", subscribed_channel);
		if (subscribed_channel[0] == '\0')
		{
			DR_TEST_PRINT("Invalid transmit channel name\n");
			continue;
		}

		result = dr_rxflow_config_add_associated_channel(config, rxc, subscribed_channel);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_PRINT("Error adding template association channel %d subscribed to %s@%s: %s\n",
				rxchannel_id, subscribed_channel, dr_rxflow_config_get_tx_device_name(config),
				dr_error_message(result, g_test_errbuf));
			continue;
		}

		// all good, add to counter...
		i++;
	}
}

static void 
dr_test_add_rxflow_unicast
(
	dr_test_t * test,
	dante_id_t flow_id,
	uint16_t num_slots
) {
	char tx_device_name[BUFSIZ];
	aud_error_t result;
	dr_test_request_t * request;
	dr_rxflow_config_t * config = NULL;

	DR_TEST_DEBUG("ACTION: Add Unicast Rx Flow Template\n");

	DR_TEST_PRINT("Enter the transmit device name for the unicast template:");
	fgets(g_input_buf, BUFSIZ, stdin);
	if (sscanf(g_input_buf, "%[^\"\r\n]", tx_device_name) != 1)
	{
		printf("Invalid device name.");
		return;
	}

	result = dr_rxflow_config_new_unicast(test->device,
		(dante_id_t) flow_id, tx_device_name, num_slots,
		&config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating unicast config object: %s\n", dr_error_message(result, g_test_errbuf));
		return;
	}

	dr_test_read_unicast_associations(test, config);

	request = dr_test_allocate_request(test, "AddUnicastRxFlowTemplate");
	if (!request)
	{
		return;
	}

	result = dr_rxflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending multicast template create request: %s\n", dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void 
dr_test_replace_rxflow_associations
(
	dr_test_t * test,
	dante_id_t flow_id
) {
	dr_rxflow_t * flow = NULL;
	dr_rxflow_config_t * config = NULL;
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Replace Rx Flow Associations\n");
	
	result = dr_device_rxflow_with_id(test->device, flow_id, &flow);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error getting flow with id %d: %s\n",
			flow_id, dr_error_message(result, g_test_errbuf));
		return;
	}

	result = dr_rxflow_replace_associations(flow, &config);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error getting config for flow with id %d: %s\n",
			flow_id, dr_error_message(result, g_test_errbuf));
		return;
	}

	if (dr_rxflow_config_is_multicast_template(config))
	{
		dr_test_read_multicast_associations(test, config);
	}
	else if (dr_rxflow_config_is_unicast_template(config))
	{
		dr_test_read_unicast_associations(test, config);
	}

	request = dr_test_allocate_request(test, "ReplaceRxFlowAssociations");
	if (!request)
	{
		return;
	}

	result = dr_rxflow_config_commit(config, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending update template associations request: %s\n", dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

static void
dr_test_delete_rxflow
(
	dr_test_t * test,
	dante_id_t flow_id
) {
	aud_error_t result;
	dr_test_request_t * request;
	dr_rxflow_t * flow;

	DR_TEST_DEBUG("ACTION: Delete Rx Flow\n");

	result = dr_device_rxflow_with_id(test->device, flow_id, &flow);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error getting flow with id %d: %s\n",
			flow_id, dr_error_message(result, g_test_errbuf));
		return;
	}

	request = dr_test_allocate_request(test, "DeleteRxFlow");
	if (!request)
	{
		return;
	}

	result = dr_rxflow_delete(&flow, dr_test_on_response, &request->id);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending flow delete request: %s\n", dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}

/*
static void 
dr_test_set_one_rxflow_interface_mode
(
	dr_test_t * test,
	dr_rxflow_error_field_t field,
	dante_id_t flow_id,
	unsigned int intf,
	unsigned int window_interval_us
) {
	aud_error_t result;
	dr_test_request_t * request;

	DR_TEST_DEBUG("ACTION: Set One RxFlow Interface Mode\n");

	request = dr_test_allocate_request(test, "SetOneRxFlowInterfaceMode");
	if (!request)
	{
		return;
	}

	result = dr_device_set_one_flow_interface_mode(test->device, dr_test_on_response, &request->id, field, flow_id, intf, window_interval_us);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error sending set one rxflow interface mode request: %s\n", dr_error_message(result, g_test_errbuf));
		dr_test_request_release(request);
		return;
	}
}
*/

//----------------------------------------------------------
// Asynchronous event handlers
//----------------------------------------------------------

static void 
dr_test_on_device_changed
(
	dr_device_t * device,
	dr_device_change_flags_t change_flags
) {
	dr_test_t * test = (dr_test_t *) dr_device_get_context(device);
	dr_device_change_index_t i;

	(void) device;
	
	DR_TEST_DEBUG("\nEVENT: device changed:");
	for (i = 0; i < DR_DEVICE_CHANGE_INDEX_COUNT; i++)
	{
		if (change_flags & (1 << i))
		{
			DR_TEST_DEBUG(" %s", dr_device_change_index_to_string(i));
			if (i == DR_DEVICE_CHANGE_INDEX_STATE)
			{
				dr_device_state_t state = dr_device_get_state(device);
				DR_TEST_DEBUG(" (%s)", dr_device_state_to_string(state));
			}
			else if (i == DR_DEVICE_CHANGE_INDEX_STALE)
			{
				unsigned int c;
				DR_TEST_DEBUG("=");
				for (c = 0; c < DR_DEVICE_COMPONENT_COUNT; c++)
				{
					if (dr_device_is_component_stale(test->device, c))
					{
						DR_TEST_DEBUG(" %s", dr_device_component_to_string(c));
					}
				}
			}
		}
	}
	DR_TEST_DEBUG("\n");

	if (change_flags & DR_DEVICE_CHANGE_FLAG_STATE)
	{
		dr_test_on_device_state_changed(test);
	}

	if (change_flags & DR_DEVICE_CHANGE_FLAG_ADDRESSES)
	{
		dr_test_on_device_addresses_changed(test);
	}

	printf("Active Requests: %d/%d\n", 
		dr_devices_num_requests_pending(test->devices),
		dr_devices_get_request_limit(test->devices));

}

//----------------------------------------------------------
// Application-level functionality
//----------------------------------------------------------

static void
dr_test_help(char filter)
{
	DR_TEST_PRINT("Usage:\n\n");
	if (!filter)
	{
		DR_TEST_PRINT("?            prints a help message\n");
		DR_TEST_PRINT("q            Quit\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'b')
	{
		DR_TEST_PRINT("b N B        Set tx signal reference level for channel N to B dbu\n");
		DR_TEST_PRINT("b N -        Clear tx signal reference level for channel N\n");
		DR_TEST_PRINT("b 0 B        Set tx signal reference level for all channels to B dbu\n");
		DR_TEST_PRINT("b 0 -        Clear tx signal reference level for all channels N\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'C')
	{
		DR_TEST_PRINT("C close current connection\n");
	}
	if (!filter || filter == 'c')
	{
		DR_TEST_PRINT("c +          Write the current device configuration\n");
		DR_TEST_PRINT("c -          Clear the current device configuration\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'D')
	{
		DR_TEST_PRINT("D .          Switch to the ADHOC domain\n");
		DR_TEST_PRINT("D N          Switch to domain N\n");
		DR_TEST_PRINT("D 0          Switch to domain NONE\n");
		DR_TEST_PRINT("D            List domains\n");
	}
	if (!filter || filter == 'd')
	{
		DR_TEST_PRINT("d t L F      Set device tx multicast performance properties to L microseconds and F frames per packet\n");
		DR_TEST_PRINT("d r L F      Set device rx multicast performance properties to L microseconds and F frames per packet\n");
		DR_TEST_PRINT("d u L F      Set device unicast performance properties to L microseconds and F frames per packet\n");
		DR_TEST_PRINT("d A P        Set AES67 Multicast Prefix (e.g. d A 0xef450000, must start with 239.x.x.x)\n");
		DR_TEST_PRINT("d p          'Ping' the device (sends a no-op routing message and waits for a response)\n");
		DR_TEST_PRINT("d !          Mark properties component as stale\n");
		DR_TEST_PRINT("d            Display device information (properties,capabilities,status)\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'e')
	{
		DR_TEST_PRINT("e N +        Enable tx channel N\n");
		DR_TEST_PRINT("e N -        Disable tx channel N\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'f')
	{
		DR_TEST_PRINT("f + N S L F  Create new transmit flow with id N, S slots, latency L microseconds and F frames per packet\n");
		DR_TEST_PRINT("f + S        Create new transmit flow with S slots\n");
		DR_TEST_PRINT("f + A N S    Create new AES67 transmit flow with id N, S slots\n");
		DR_TEST_PRINT("f N          Modify transmit flow N\n");
		DR_TEST_PRINT("f - N        Delete transmit flow N\n");
		DR_TEST_PRINT("f !          Mark transmit flow component as being stale\n");
		DR_TEST_PRINT("f            Display all transmit flows\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'g')
	{
		DR_TEST_PRINT("g m N        Create a new multicast receive template on flow id N\n");
		DR_TEST_PRINT("g u N S      Create a new unicast receive template on flow id N with S slots\n");
		DR_TEST_PRINT("g * N        Replace channel associations for template with flow id N\n");
		DR_TEST_PRINT("g - N        Delete template with flow id N\n");
		DR_TEST_PRINT("g !          Mark receive flow component as being stale\n");
		DR_TEST_PRINT("g            Display all receive flows\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'l')
	{
		DR_TEST_PRINT("l N \"NAME\" + Add label NAME to tx channel N\n");
		DR_TEST_PRINT("l N \"NAME\" - Remove label NAME from tx channel N\n");
		DR_TEST_PRINT("l N          List labels for tx channel N\n");
		DR_TEST_PRINT("l !          Mark tx labels component as stale\n");
		DR_TEST_PRINT("l            List labels for all tx channels\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'L')
	{
		DR_TEST_PRINT("L N -        Remove label N\n");
		DR_TEST_PRINT("L N          Display label N\n");
		DR_TEST_PRINT("L !          Mark tx labels component as stale\n");
		DR_TEST_PRINT("L            List all labels\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'n')
	{
		DR_TEST_PRINT("n NAME       Rename the device to NAME (closes the device handle)\n");
		DR_TEST_PRINT("n + NAME     Rename the device to NAME (does not close the device handle)\n");
		DR_TEST_PRINT("n -          Rename the device to its default name (closes the device handle)\n");
		DR_TEST_PRINT("n            Print the device's name\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'O')
	{
		DR_TEST_PRINT("O \"NAME\"     Open connection to device NAME\n");
		DR_TEST_PRINT("O 0xXXXXXXXX Open connection to device with the given domain routing ID\n");
		DR_TEST_PRINT("O A.B.C.D    Open connection to device at the given IP address\n");
		DR_TEST_PRINT("O            Open connection to the local device\n");
	}
	if (!filter || filter == 'o')
	{
		DR_TEST_PRINT("o +          Enable network loopback\n");
		DR_TEST_PRINT("o -          Disable network loopback\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'R')
	{
		DR_TEST_PRINT("R FILE       Batch rename N rx channels from file FILE, where each line of the file is \"N NAME\" or just \"N\"\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'r')
	{
		DR_TEST_PRINT("r !          Mark rx channel component as stale\n");
		DR_TEST_PRINT("r N m +      Mute rx channel N\n");
		DR_TEST_PRINT("r N m -      Unute rx channel N\n");
		DR_TEST_PRINT("r N \"NAME\"   Renamed rx channel N\n");
		DR_TEST_PRINT("r            Display rx channel information\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'S')
	{
		DR_TEST_PRINT("S FILE       Batch subscribe N rx channels from file FILE, where each line of the file is \"N NAME\" or just \"N\"\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 's')
	{
		DR_TEST_PRINT("s N \"NAME\"   Subscribe rx channel N to network channel NAME\n");
		DR_TEST_PRINT("s N          Unsubscribe rx channel N\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 't')
	{
		DR_TEST_PRINT("t !          Mark tx channel component as stale\n");
		DR_TEST_PRINT("t N m +      Mute tx channel N\n");
		DR_TEST_PRINT("t N m -      Unute tx channel N\n");
		DR_TEST_PRINT("t            Display tx channel information\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'u')
	{
		DR_TEST_PRINT("u q          Query (or re-query) capabilities\n");
		DR_TEST_PRINT("u !          Invalidate and update all components\n");
		DR_TEST_PRINT("u            Update information for all stale components\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'U')
	{
		DR_TEST_PRINT("U TYPE +     Get and clear rxflow error field TYPE for all flows\n");
		DR_TEST_PRINT("U TYPE -     Get rxflow error field TYPE for all flows\n"); 
		DR_TEST_PRINT("U +          Get and clear rxflow error flags for all flows\n");
		DR_TEST_PRINT("U -          Get rxflow error flags for all flows\n");
		DR_TEST_PRINT("U N          Print rxflow error information for rx flow N\n");
		DR_TEST_PRINT("U            Print rxflow error information for all rx flows\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'x')
	{
		DR_TEST_PRINT("x +          Put the device into lockdown\n");
		DR_TEST_PRINT("x -          Take the device out of lockdown\n");
		DR_TEST_PRINT("\n");
	}
	if (!filter || filter == 'X')
	{
		DR_TEST_PRINT("X            Cancel outstanding requests\n");
		DR_TEST_PRINT("\n");
	}
}

static void
dr_test_usage
(
	const char * bin
) {
	printf("Usage: %s OPTIONS [DEVICE]\n", bin);
	printf("  OPTIONS are\n");
	printf("    -h=N set num of handles to N\n");
	printf("    -r=N set num of requests to N\n");
	printf("    -ii=INDEX use local interface INDEX (specify once per interface to be used)\n");
	printf("    -i=NAME use local interface NAME (specify once per interface to be used)\n");
	printf("    -a=ADDRESS use address A instead of name (specify once per interface to be used)\n");
	printf("    -u=BOOL enable/disable automatic query / updates on state changes\n");
	printf("    -p=PORT set port for local device connection (for debugging purposes only)\n");
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	printf("    -m=PORT_NO set MDNS server port number to PORT_NO\n");
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
#ifdef WIN32
	printf("    -d=PORT_NO set domain handler port number to PORT_NO\n");
#else
	printf("    -d=PATH set domain handler socket path to PATH\n");
#endif
#endif

#ifdef DANTE_ROUTING_TEST_CUSTOM_USAGE
	DANTE_ROUTING_TEST_CUSTOM_USAGE();
#endif

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	printf("    --domain[=<preferred domain>] browse on a domain\n");
	printf("    --user=<user> browse on a domain with the specified username\n");
	printf("    --pass=<password> browse on a domain with the specified password\n");
	printf("    --ddm=<host>:<port> use the specified ddm rather than discovering (--domain must be specified)\n");
#endif
	printf("  If no name or addresses specified then connect to the local dante device via localhost\n");
}

static void
dr_test_parse_options
(
	dr_test_options_t * options,
	int argc, 
	char * argv[]
) {
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	aud_error_t result;
#endif
	int i;

	// init defaults
	options->automatic_update_on_state_change = AUD_TRUE;

	// and parse options
	for (i = 1; i < argc; i++)
	{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		if (dapi_utils_ddm_config_parse_one(&options->ddm_config, argv[i], &result))
		{
			if (result != AUD_SUCCESS)
			{
				dr_test_usage(argv[0]);
				exit(1);
			}
		}
		else
#endif
#ifdef DANTE_ROUTING_TEST_CUSTOM_PARSE_OPTIONS
		if (DANTE_ROUTING_TEST_CUSTOM_PARSE_OPTIONS(test, argc, argv, &i))
		{}
		else
#endif
		if (!strncmp(argv[i], "-h=", 3))
		{
			options->num_handles = atoi(argv[i]+3);
		}
		else if (!strncmp(argv[i], "-r=", 3))
		{
			options->request_limit = atoi(argv[i]+3);
		}
		else if (!strncmp(argv[i], "-ii=", 4))
		{
			if (options->num_local_interfaces < DR_TEST_MAX_INTERFACES)
			{
				int local_interface_index = atoi(argv[i]+4);
				if (local_interface_index)
				{
					options->local_interfaces[options->num_local_interfaces].index = local_interface_index;
					options->local_interfaces[options->num_local_interfaces].flags = AUD_INTERFACE_IDENTIFIER_FLAG_INDEX;
					options->num_local_interfaces++;
				}
				else
				{
					dr_test_usage(argv[0]);
					exit(1);
				}
			}
		}
		else if (!strncmp(argv[i], "-i=", 3))
		{
			if (options->num_local_interfaces < DR_TEST_MAX_INTERFACES)
			{
				const char * intf_name = argv[i]+3;
				if (intf_name[0])
				{
#ifdef WIN32
					mbstowcs(
						options->local_interfaces[options->num_local_interfaces].name,
						intf_name,
						AUD_INTERFACE_NAME_LENGTH
					);
#else
					aud_strlcpy(
						options->local_interfaces[options->num_local_interfaces].name,
						intf_name,
						AUD_INTERFACE_NAME_LENGTH
					);
#endif
					options->local_interfaces[options->num_local_interfaces].flags = AUD_INTERFACE_IDENTIFIER_FLAG_NAME;
					options->num_local_interfaces++;
				}
				else
				{
					dr_test_usage(argv[0]);
					exit(1);
				}
			}
		}
		else if (!strncmp(argv[i], "-a=", 3))
		{
			if (options->num_addresses < DR_TEST_MAX_INTERFACES)
			{
				unsigned int addr = inet_addr(argv[i]+3);
				options->addresses[options->num_addresses++] = addr;
			}
		}
		else if (!strcmp(argv[i], "-u=true"))
		{
			options->automatic_update_on_state_change = AUD_TRUE;
		}
		else if (!strcmp(argv[i], "-u=false"))
		{
			options->automatic_update_on_state_change = AUD_FALSE;
		}
		else if (!strncmp(argv[i], "-p=", 3) && strlen(argv[i]) > 3)
		{
			options->local_port = (uint16_t) atoi(argv[i]+3);
		}
		else if (!strncmp(argv[i], "-target=", 8) && strlen(argv[i]) > 8)
		{
			options->domain_device_addr = strtoul(argv[i] + 8, NULL, 0);
		}
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
		else if (!strncmp(argv[i], "-m=", 3))
		{
			options->mdns_server_port = (uint16_t) atoi(argv[i] + 3);
		}
#endif
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
		else if (!strncmp(argv[i], "-d=", 3))
		{
#ifdef WIN32
			options->domain_handler.port_no = (uint16_t)atoi(argv[i] + 3);
#else
			options->domain_handler.socket_path = argv[i] + 3;
#endif
		}
#endif
		else if (argv[i][0] == '-')
		{
			dr_test_usage(argv[0]);
			exit(1);
		}
		else
		{
			SNPRINTF(options->device_name, sizeof(options->device_name), "%s", argv[i]);
			break;
		}
	}
}

static aud_error_t
dr_test_process_line(
	dr_test_t * test,
	/*[in]*/ char * buf,
	/*[out]*/ char*** array,
	/*[out]*/ int* count
)
{
	char in_name[BUFSIZ];
	char in_action[BUFSIZ];
	char in_c;
	unsigned int in_channel, in_id, in_count, in_fpp, in_latency, in_type, prefix;
	int in_dbu;
	int match_count;

	// trim beginning and end of string
	//printf("Before=\"%s\"\n", buf);
	while(*buf && isspace(*buf)) buf++;
	{
		char * end = buf;
		while (*end) end++;
		end--;
		while (end > buf && isspace(*end)) {*end = '\0'; end--;}
	}
	//printf("After=\"%s\"\n", buf);

#ifdef DANTE_ROUTING_TEST_CUSTOM_PROCESS_LINE
	if (DANTE_ROUTING_TEST_CUSTOM_PROCESS_LINE(test, buf))
	{
		return AUD_SUCCESS;
	}
#endif

	switch (buf[0])
	{
	case '?':
		{
			dr_test_help(0);
			break;
		}

	case 'b':
		{
			if (sscanf(buf, "b %u %d", &in_channel, &in_dbu) == 2)
			{
				dr_test_txchannel_set_signal_reflevel(test, in_channel, (dante_dbu_t)in_dbu);
			}
			else if (sscanf(buf, "b %u -", &in_channel) == 1)
			{
				dr_test_txchannel_clear_signal_reflevel(test, in_channel);
			}
			else
			{
				dr_test_help('b');
			}
			break;
		}
	case 'C': 
		{
			dr_test_close(test);
			break;
		}
	
	case 'c':
		{
			if (sscanf(buf, "c %s", in_action) == 1 && !strcmp(in_action, "+"))
			{
				dr_test_store_config(test);
			}
			else if (sscanf(buf, "c %s", in_action) == 1 && !strcmp(in_action, "-"))
			{
				dr_test_clear_config(test);
			}
			else
			{
				dr_test_help('c');
			}
			break;
		}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	case 'D':
		{
			if (sscanf(buf, "D %c", &in_c) == 1 && in_c == '.')
			{
				dr_test_set_current_domain_by_uuid(test, DANTE_DOMAIN_UUID_ADHOC);
			}
			else if (sscanf(buf, "D %99s", in_name) == 1)
			{
				dr_test_set_current_domain_by_name(test, in_name);
			}
			else if (sscanf(buf, "D %c", &in_c) == 1)
			{
				dr_test_help('D');
			}
			else
			{
				dapi_utils_print_domain_handler_info(dapi_get_domain_handler(test->dapi));
			}
			break;
		}
#endif
	case 'd':
		{
			if (sscanf(buf, "d t %u %u", &in_latency, &in_fpp) == 2)
			{
				dr_test_set_performance(test, DR_TEST_PERFORMANCE_TX, (dante_latency_us_t) in_latency, (dante_fpp_t) in_fpp);
			}
			else if (sscanf(buf, "d r %u %u", &in_latency, &in_fpp) == 2)
			{
				dr_test_set_performance(test, DR_TEST_PERFORMANCE_RX, (dante_latency_us_t) in_latency, (dante_fpp_t) in_fpp);
			}
			else if (sscanf(buf, "d u %u %u", &in_latency, &in_fpp) == 2)
			{
				dr_test_set_performance(test, DR_TEST_PERFORMANCE_UNICAST, (dante_latency_us_t) in_latency, (dante_fpp_t) in_fpp);
			}
			else if (sscanf(buf, "d A 0x%x", &prefix) == 1)
			{
				dr_test_set_aes67_prefix(test, prefix);
			}
			else if (sscanf(buf, "d %s", in_action) == 1)
			{
				if (!strcmp(in_action, "p"))
				{
					dr_test_ping(test);
				}
				else if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_PROPERTIES, 0);
				}
				else
				{
					dr_test_help('d');
				}
			}
			else
			{
				dr_test_print_device_info(test->device);
			}
			break;
		}

	case 'e':
		{
			if (sscanf(buf, "e %u %s", &in_channel, in_action) == 2 && !strcmp(in_action, "+"))
			{
				dr_test_txchannel_set_enabled(test, in_channel, AUD_TRUE);
			}
			else if (sscanf(buf, "e %u %s", &in_channel, in_action) == 2 && !strcmp(in_action, "-"))
			{
				dr_test_txchannel_set_enabled(test, in_channel, AUD_FALSE);
			}
			else
			{
				dr_test_help('e');
			}
			break;
		}

	// transmit flows
	case 'f':
		{
			if (sscanf(buf, "f + %u %u %u %u", &in_id, &in_count, &in_latency, &in_fpp) == 4)
			{
				dr_test_add_txflow(test, (dante_id_t) in_id, (uint16_t) in_count, (dante_latency_us_t) in_latency, (dante_fpp_t) in_fpp);
			}
			else if (sscanf(buf, "f + %u", &in_count) == 1)
			{
				dr_test_add_txflow(test, 0, (uint16_t) in_count, 0, 0);
			}
			else if (sscanf(buf, "f + A %u %u", &in_id, &in_count) == 2)
			{
				dr_test_add_aes67_txflow_multicast(test, (dante_id_t) in_id, (uint16_t) in_count);
			}
			else if (sscanf(buf, "f %c %u", &in_c, &in_id) == 2 && in_c == '-')
			{
				dr_test_delete_txflow(test, (dante_id_t) in_id);
			}
			else if (sscanf(buf, "f %u", &in_id) == 1)
			{
				dr_test_replace_txflow_channels(test, (dante_id_t) in_id);
			}
			else if (sscanf(buf, "f %s", in_action) == 1)
			{
				if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXFLOWS, 0);
				}
				else
				{
					dr_test_help('f');
				}
			}
			else
			{
				dr_test_print_device_txflows(test->device);
			}
			break;
		}

	// receive flows
	case 'g':
		{
			if (sscanf(buf, "g u %d %d", &in_id, &in_count) == 2)
			{
				dr_test_add_rxflow_unicast(test, (dante_id_t) in_id, (uint16_t) in_count);
			}
			else if (sscanf(buf, "g %c %d", &in_c, &in_id) == 2)
			{
				if (in_c == 'm')
				{
					dr_test_add_rxflow_multicast(test, in_id);
				}
				else if (in_c == '*')
				{
					dr_test_replace_rxflow_associations(test, (dante_id_t) in_id);
				}
				else if (in_c == '-')
				{
					dr_test_delete_rxflow(test, (dante_id_t) in_id);
				}
				else
				{
					dr_test_help('g');
				}
			}
			else if (sscanf(buf, "g %s", in_action) == 1)
			{
				if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_RXFLOWS, 0);
				}
				else
				{
					dr_test_help('g');
				}
			}
			else
			{
				dr_test_print_device_rxflows(test->device);
			}
			break;
		}

	case 'l':
		{
			if (sscanf(buf, "l %u \"%[^\"\r\n]%c %s", &in_channel, in_name, &in_c, in_action) == 4 && in_c == '\"')
			{
				if (!strcmp(in_action, "+"))
				{
					dr_test_add_txlabel(test, in_channel, in_name);
				}
				else if (!strcmp(in_action, "-"))
				{
					dr_test_remove_txlabel_from_channel(test, (dante_id_t) in_channel, in_name);
				}
				else
				{
					dr_test_help('l');
				}
			}
			else if (sscanf(buf, "l %u %s", &in_channel, in_name) == 2)
			{
				if (!strcmp(in_name, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXLABELS, (dante_id_t)in_channel);
				}
				else
				{
					dr_test_help('l');
				}
			}
			else if (sscanf(buf, "l %u", &in_channel) == 1)
			{
				dr_test_print_channel_txlabels(test->device, (dante_id_t) in_channel, array, count);
			}
			else if (sscanf(buf, "l %s", in_name) == 1)
			{
				if (!strcmp(in_name, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXLABELS, 0);
				}
				else
				{
					dr_test_help('l');
				}
			}
			else
			{
				dr_test_print_channel_txlabels(test->device, 0, array, count);
			}
			break;
		}

	case 'L':
		{
			match_count = sscanf(buf, "L %u %s", &in_id, in_action);
			if (match_count == 2)
			{
				if (!strcmp(in_action, "-"))
				{
					dr_test_remove_txlabel(test, (dante_id_t) in_id);
				}
				else
				{
					dr_test_help('L');
				}
			}
			else if (match_count == 1)
			{
				dr_test_print_device_txlabels(test->device, (dante_id_t) in_id, AUD_FALSE);
			}
			else
			{
				match_count = sscanf(buf, "L %s", in_action);
				if (match_count == 1)
				{
					if (!strcmp(in_action, "!"))
					{
						dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXLABELS, 0);
					}
					else
					{
						dr_test_help('L');
					}
				}
				else
				{
					dr_test_print_device_txlabels(test->device, 0, AUD_TRUE);
				}
			}
			break;
		}

	case 'n':
		{
			if (sscanf(buf, "n %c %s", &in_c, in_name) == 2 && in_c == '+')
			{
				dr_test_rename(test, in_name, AUD_FALSE);
			}
			else if (sscanf(buf, "n %s", in_name) == 1)
			{
				aud_error_t result = AUD_ERR_INVALIDSTATE;
				if (!strcmp(in_name, "-"))
				{
					result = dr_test_rename(test, NULL, AUD_TRUE);
				}
				else if (!strcmp(in_name, "?"))
				{
					dr_test_help('n');
				}
				else
				{
					result = dr_test_rename(test, in_name, AUD_TRUE);
				}
				if (result == AUD_SUCCESS)
				{
					return AUD_ERR_DONE;
				}
			}
			else
			{
				dr_test_print_device_names(test->device);
			}
			break;
		}

	case 'O':
		{
			unsigned int a, b, c, d;
			dante_ipv4_addr_t addr = {0};
			if (sscanf(buf, "O \"%[^\"\r\n]%c", in_name, &in_c) == 2 && in_c == '\"')
			{
				SNPRINTF(test->options.device_name, sizeof(test->options.device_name), "%s", in_name);
				test->options.domain_device_addr = 0;
				test->options.num_addresses = 0;
				dr_test_open(test);
			}
			else if (sscanf(buf, "O 0x%u", &in_id) == 1)
			{
				test->options.device_name[0] = '\0';
				test->options.domain_device_addr = in_id;
				test->options.num_addresses = 0;
				dr_test_open(test);
			}
			else if (sscanf(buf, "O %u.%u.%u.%u", &a,&b,&c,&d) == 4)
			{
				addr.addr8[0] = (uint8_t) a;
				addr.addr8[1] = (uint8_t) b;
				addr.addr8[2] = (uint8_t) c;
				addr.addr8[3] = (uint8_t) d;
				test->options.device_name[0] = '\0';
				test->options.domain_device_addr = 0;
				test->options.num_addresses = addr.addr32;
				dr_test_open(test);
			}
			else if (sscanf(buf, "O %c", &in_c) == 1)
			{
				dr_test_help('O');
			}
			else
			{
				test->options.device_name[0] = '\0';
				test->options.domain_device_addr = 0;
				test->options.num_addresses = 0;
				dr_test_open(test);
			}
			break;
		}

	case 'o':
		{
			if (sscanf(buf, "o %s", in_action) == 1 && !strcmp(in_action, "+"))
			{
				dr_test_set_network_loopback(test, AUD_TRUE);
			}
			else if (sscanf(buf, "o %s", in_action) == 1 && !strcmp(in_action, "-"))
			{
				dr_test_set_network_loopback(test, AUD_FALSE);
			}
			else
			{
				dr_test_help('o');
			}
			break;
		}

	case 'q':
		{
			printf("\n");
			g_test_running = AUD_FALSE;
			break;
		}

	case 'R':
		{
			if (sscanf(buf, "R %s", in_name) == 1)
			{
				dr_test_batch_rxchannel_set_name(test, in_name);
			}
			else
			{
				dr_test_help('R');
			}
			break;
		}

	case 'r':
		{
			if (sscanf(buf, "r %u m %c", &in_channel, &in_c) == 2 && in_c == '+')
			{
				dr_test_rxchannel_set_muted(test, in_channel, AUD_TRUE);
			}
			else if (sscanf(buf, "r %u m %c", &in_channel, &in_c) == 2 && in_c == '-')
			{
				dr_test_rxchannel_set_muted(test, in_channel, AUD_FALSE);
			}
			else if (sscanf(buf, "r %u \"%[^\"\r\n]%c", &in_channel, in_name, &in_c) == 3 && in_c == '\"')
			{
				dr_test_rxchannel_set_name(test, in_channel, in_name);
			}
			else if (sscanf(buf, "r %u !", &in_channel) == 1)
			{
				dr_test_mark_stale(test, DR_DEVICE_COMPONENT_RXCHANNELS, (dante_id_t) in_channel);
			}
			else if (sscanf(buf, "r %s", in_action) == 1)
			{
				if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_RXCHANNELS, 0);
				}
				else
				{
					dr_test_help('r');
				}
			}
			else
			{
				dr_test_print_device_rxchannels(test->device, array, count);
			}
			break;
		}

	case 'S':
		{
			if (sscanf(buf, "S %s", in_name) == 1)
			{
				dr_test_batch_rxchannel_subscribe(test, in_name);
			}
			else
			{
				dr_test_help('S');
			}
			break;
		}
	case 's':
		{
			if (sscanf(buf, "s %u \"%[^\"\r\n]%c", &in_channel, in_name, &in_c) == 3 && in_c == '"')
			{
				dr_test_rxchannel_subscribe(test, in_channel, in_name);
			}
			else if (sscanf(buf, "s %u %s", &in_channel, in_name) == 2)
			{
				DR_TEST_PRINT("NAME must be of the form \"channel@device\"\n");
			}
			else if (sscanf(buf, "s %u", &in_channel) == 1)
			{
				dr_test_rxchannel_subscribe(test, in_channel, NULL);
			}
			else
			{
				dr_test_help('s');
			}
			break;
		}

	case 't':
		{
			if (sscanf(buf, "t %u m %c", &in_channel, &in_c) == 2 && in_c == '+')
			{
				dr_test_txchannel_set_muted(test, in_channel, AUD_TRUE);
			}
			else if (sscanf(buf, "t %u m %c", &in_channel, &in_c) == 2 && in_c == '-')
			{
				dr_test_txchannel_set_muted(test, in_channel, AUD_FALSE);
			}
			else if (sscanf(buf, "t %u !", &in_channel) == 1)
			{
				dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXCHANNELS, (dante_id_t) in_channel);
			}
			else if (sscanf(buf, "t %s", in_action) == 1)
			{
				if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_TXCHANNELS, 0);
				}
				else
				{
					dr_test_help('t');
				}
			}
			else
			{
				dr_test_print_device_txchannels(test->device, array, count);
			}
			break;
		}

	case 'u':
		{
			if (sscanf(buf, "u %s", in_action) == 1)
			{
				if (!strcmp(in_action, "q"))
				{
					dr_test_query_capabilities(test);
				}
				else if (!strcmp(in_action, "!"))
				{
					dr_test_mark_stale(test, DR_DEVICE_COMPONENT_COUNT, 0);
					dr_test_update(test);
				}
				else
				{
					dr_test_help('u');
				}
			}
			else
			{
				dr_test_update(test);
			}
			break;
		}

	case 'U':
		{
			if (sscanf(buf, "U %u %c", &in_type, &in_c) == 2 && in_c == '+')
			{
				dr_test_update_rxflow_errors(test, (dante_rxflow_error_type_t) in_type, AUD_TRUE);
			}
			else if (sscanf(buf, "U %u %c", &in_type, &in_c) == 2 && in_c == '-')
			{
				dr_test_update_rxflow_errors(test, (dante_rxflow_error_type_t) in_type, AUD_FALSE);
			}
			else if (sscanf(buf, "U %c", &in_c) == 1 && in_c == '+')
			{
				dr_test_update_rxflow_errors(test, DANTE_NUM_RXFLOW_ERROR_TYPES, AUD_TRUE);
			}
			else if (sscanf(buf, "U %c", &in_c) == 1 && in_c == '-')
			{
				dr_test_update_rxflow_errors(test, DANTE_NUM_RXFLOW_ERROR_TYPES, AUD_FALSE);
			}
			else if (sscanf(buf, "U %u", &in_id) == 1)
			{
				dr_test_print_device_rxflow_errors(test->device, (dante_id_t) in_id);
			}
			else
			{
				dr_test_print_device_rxflow_errors(test->device, 0);
			}
			break;
		}

	case 'x':
		{
			if (sscanf(buf, "x %s", in_action) == 1)
			{
				if (!strcmp(in_action, "+"))
				{
					dr_test_set_lockdown(test, AUD_TRUE);
				}
				else if (!strcmp(in_action, "-"))
				{
					dr_test_set_lockdown(test, AUD_FALSE);
				}
				else
				{
					dr_test_help('x');
				}
			}
			else
			{
				dr_test_update(test);
			}
			break;
		}

	case 'X':
		{
			dr_test_cancel(test);
			break;
		}

	default:
		{
			if (buf[0])
			{
				DR_TEST_PRINT("Unknown command '%c'\n", buf[0]);
			}
			DR_TEST_PRINT("Type '?' for help\n");
			break;
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


static aud_error_t
dr_test_main_loop
(
	dr_test_t * test,
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

	DR_TEST_PRINT("\nDante Routing API test program. Type '?' for help\n\n");
	
	while(g_test_running)
	{		
		char buf[BUFSIZ];
		

		// print prompt if needed
		if (print_prompt)
		{
			const char * name = dr_device_get_name(test->device);
			DR_TEST_PRINT("\n'%s'> ", name ? name : "");
			fflush(stdout);
			print_prompt = AUD_FALSE;
		}

#ifdef _WIN32
		dapi_utils_step(test->runtime, AUD_SOCKET_INVALID, NULL);
#else
		dapi_utils_step(test->runtime, 0, &select_sockets);
#endif

		// and check stdin 
		buf[0] = '\0';
#ifdef _WIN32
		if (_kbhit())
		{
			DWORD len = 0;
			if (!ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE),buf,BUFSIZ-1,&len, 0))
			{
				printf("Error reading console: %d\n", GetLastError());
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
					DR_TEST_PRINT("Exiting...\n");
					return AUD_SUCCESS;
				}
				else if (result == AUD_ERR_INTERRUPTED)
				{
					clearerr(stdin);
				}
				else
				{
					DR_TEST_ERROR("Exiting with %s\n", dr_error_message(result, g_test_errbuf));
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
			DR_TEST_PRINT("\n");
		#endif
			result = dr_test_process_line(test, buf, array, count);
			if (result != AUD_SUCCESS)
			{
				break;
			}
		}
	}
	return AUD_SUCCESS;
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
	dr_test_t test;
	aud_error_t result = AUD_SUCCESS;

	DR_TEST_PRINT("%s: Routing API version %u.%u.%u\n",
		argv[0], DR_VERSION_MAJOR, DR_VERSION_MINOR, DR_VERSION_BUGFIX);
	
	memset(&test, 0, sizeof(dr_test_t));

	dr_test_parse_options(&test.options, argc, argv);

	// create an environment
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	dapi_config_t * dapiConfig = dapi_config_new();
	if (dapiConfig)
	{
		dante_domain_handler_config_t * domainHandlerConfig = dapi_config_get_domain_handler_config(dapiConfig);

		if (domainHandlerConfig)
		{
#ifdef WIN32
			dante_domain_handler_config_set_port(domainHandlerConfig, test.options.domain_handler.port_no);
#else
			dante_domain_handler_config_set_unix_path(domainHandlerConfig, test.options.domain_handler.socket_path);
#endif
		}
	}

#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
	if (test.options.mdns_server_port > 0)
	{
		dapi_config_set_mdns_server_port(dapiConfig, test.options.mdns_server_port);
	}
#endif

	result = dapi_new_config(dapiConfig, &test.dapi);

	dapi_config_delete(dapiConfig);
#else
	result = dapi_new(&test.dapi);
#endif
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error initialising environment: %s\n", dr_error_message(result, g_test_errbuf));
		goto cleanup;
	}

	test.env = dapi_get_env(test.dapi);
	test.handler = dapi_get_domain_handler(test.dapi);
	test.runtime = dapi_get_runtime(test.dapi);

	//aud_log_set_threshold(aud_env_get_log(test.env), AUD_LOGTYPE_STDOUT, AUD_LOG_DEBUG);

	// create a devices structure and set its options
	result = dr_devices_new_dapi(test.dapi, &test.devices);
	if (result != AUD_SUCCESS)
	{
		DR_TEST_ERROR("Error creating device factory: %s\n", dr_error_message(result, g_test_errbuf));
		goto cleanup;
	}
	dr_devices_set_context(test.devices, &test);
	if (test.options.num_handles)
	{
		result = dr_devices_set_num_handles(test.devices, test.options.num_handles);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error setting num_handles to %d: %s\n", test.options.num_handles, dr_error_message(result, g_test_errbuf));
			goto cleanup;
		}
	}
	if (test.options.request_limit)
	{
		result = dr_devices_set_request_limit(test.devices, test.options.request_limit);
		if (result != AUD_SUCCESS)
		{
			DR_TEST_ERROR("Error setting request_limit to %d: %s\n", test.options.request_limit, dr_error_message(result, g_test_errbuf));
			goto cleanup;
		}
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&test.options.ddm_config, test.handler, test.runtime, &g_test_running);
	if (result != AUD_SUCCESS)
	{
			DR_TEST_ERROR("Error connecting to DDM: %s\n", dr_error_message(result, g_test_errbuf));
			goto cleanup;
	}
#endif

#ifdef DANTE_ROUTING_TEST_CUSTOM_CONFIG_OPTIONS
	DANTE_ROUTING_TEST_CUSTOM_CONFIG_OPTIONS(&test);
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(test.handler);
	dante_domain_handler_set_context(test.handler, &test);
	dante_domain_handler_set_event_fn(test.handler, dr_test_event_handle_ddh_changes);

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
	// Some embedded platforms support asynchronous connection to the dante device.
	// Opening a device is not possible while in the NONE domain,
	// so wait till domain uuid is available
	while (IS_NO_DOMAIN_UUID(dr_devices_get_domain_uuid(test.devices)))
	{
		dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);
	}
#endif

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dante_domain_info_t info = dante_domain_handler_get_current_domain(test.handler);
	aud_bool_t skip_device_open = AUD_FALSE;
	//Is current domain the one that we want to be on?
	if (test.options.ddm_config.domain[0] && strcmp(info.name, test.options.ddm_config.domain))
	{
		if (IS_ADHOC_DOMAIN_UUID(info.uuid))
		{
			printf("WARNING: Current domain set to ADHOC\n\n");
			printf("WARNING: Unable to open the device as requested domain is not available\n");
			skip_device_open = AUD_TRUE;
		}
	}
	if (!skip_device_open)
	{
		// Now open a device connection
		result = dr_test_open(&test);
		if (result != AUD_SUCCESS)
		{
			goto cleanup;
		}
	}

#else
	// Now open a device connection
	result = dr_test_open(&test);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif
	// and run the main loop
	//dr_test_main_loop(&test);

	for (size_t i = 0; i < 15; i++)
	{
		dapi_utils_step(test.runtime, AUD_SOCKET_INVALID, NULL);
	}

	result = dr_test_process_line(&test, input, array, count);

cleanup:
	if (test.device)
	{
		dr_device_close(test.device);
	}
	if (test.devices)
	{
		dr_devices_delete(test.devices);
	}
	if (test.dapi)
	{
		dapi_delete(test.dapi);
	}
	return result;
}

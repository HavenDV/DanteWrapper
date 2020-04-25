#include "audinate/dante_api.h"

#include "dapi_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

typedef struct
{
	dapi_t * dapi;
	cmm_client_t * client;
} cmm_client_test_t;

//-------------------
// Printing
//------------------

static const char *
cmm_connection_state_to_string
(
	cmm_connection_state_t connection_state
) {
	switch (connection_state)
	{
	case CMM_CONNECTION_STATE_INITIALISED:  return "INITIALISED";
	case CMM_CONNECTION_STATE_CONNECTING:   return "CONNECTING";
	case CMM_CONNECTION_STATE_CONNECTED:    return "CONNECTED";
	default: return "?";
	}
}

static void
cmm_client_test_print_info
(
	const cmm_info_t * info,
	const char * prefix
) {
	if (info)
	{
		dante_version_t version;
		cmm_info_get_version(info, &version);
		printf("%sVersion: %u.%u.%u\n", prefix, version.major, version.minor, version.bugfix);
		printf("%sMax Dante Networks: %u\n", prefix, cmm_info_max_dante_networks(info));
	}
}

static void
cmm_client_test_print_options
(
	const cmm_options_t * options,
	const char * prefix
) {
	uint16_t i;
	printf("%sInterfaces:\n", prefix);
	if (options)
	{
		for (i = 0; i < cmm_options_num_interfaces(options); i++)
		{
			const cmm_interface_t * iface = 
				cmm_options_interface_at_index(options, i);
			const uint8_t * mac = cmm_interface_get_mac_address(iface);
			if (cmm_interface_is_configured(iface))
			{
				printf("%s  %ls(%02x:%02x:%02x:%02x:%02x:%02x)\n", prefix,
					cmm_interface_get_name(iface),
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			}
			else
			{
				printf("%s  NONE\n", prefix);
			}
		}
	}
}

const char * CMM_CLIENT_TEST_INTERFACE_NAMES[2] =
{
	"Primary",
	"Secondary"
};

static void
cmm_client_test_print_config
(
	const cmm_config_t * config,
	const char * prefix
) {
	uint16_t i; 
	if (config != NULL)
	{
		for (i = 0; i < cmm_config_num_interfaces(config); i++)
		{
			const cmm_interface_t * iface = cmm_config_interface_at_index(config, i);
			const uint8_t * mac = cmm_interface_get_mac_address(iface);
			printf("%s  %ls(%02x:%02x:%02x:%02x:%02x:%02x)\n", prefix,
				cmm_interface_get_name(iface),
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
}

static void
cmm_client_test_print_process
(
	const cmm_process_t * process,
	const char * prefix
) {
	printf("%s  %s: %d %s\n", 
        prefix,
		cmm_process_get_name(process), 
        cmm_process_get_id(process),
		(cmm_process_is_locking_configuration(process) ? " (locked)" : ""));
}


static void
cmm_client_test_print_state
(
	const cmm_state_t * state,
	const char * prefix
) {
	if (state)
	{
		cmm_system_status_t system_status = cmm_state_get_system_status(state);
		uint16_t i;

		printf("%sSystem status: %s (%04x)\n", prefix, 
			cmm_system_status_to_string(system_status), system_status);
		printf("%sDante device name: %s\n", prefix, cmm_state_get_dante_device_name(state));
		printf("%sCurrent config:\n", prefix);
		cmm_client_test_print_config(cmm_state_get_current_config(state), prefix);
		printf("%sPending config:\n", prefix);
		cmm_client_test_print_config(cmm_state_get_pending_config(state), prefix);
		printf("%sProcesses:\n", prefix);
		for (i = 0; i < cmm_state_num_processes(state); i++)
		{
			const cmm_process_t * process = cmm_state_process_at_index(state, i);
			cmm_client_test_print_process(process, prefix);
		}
	}
}

//----------------------------------------------------------
// Callback event handlers
//----------------------------------------------------------


aud_bool_t running = AUD_TRUE;

void signal_handler(int sig);

void signal_handler(int sig)
{
	AUD_UNUSED(sig);
	signal(SIGINT, signal_handler);
	running = AUD_FALSE;
}

cmm_client_event_fn cmm_client_test_event;

//----------------------------------------------------------
// Callback event handlers
//----------------------------------------------------------

static void
cmm_client_test_on_connection_changed
(
	cmm_client_t * client
) {
	cmm_connection_state_t connection = cmm_client_get_connection_state(client);
	printf("  Client connection state is now %s\n",
		cmm_connection_state_to_string(connection));
	if (connection == CMM_CONNECTION_STATE_CONNECTED)
	{
		printf("  Info is:\n");
		cmm_client_test_print_info(cmm_client_get_system_info(client), "  ");
	}
}

static void
cmm_client_test_on_options_changed
(
	cmm_client_t * client
) {
	printf("  Options have changed, now:\n");
	cmm_client_test_print_options(cmm_client_get_system_options(client), "  ");
}

static void
cmm_client_test_on_state_changed
(
	cmm_client_t * client
) {
	printf("  State has changed, now:\n");
	cmm_client_test_print_state(cmm_client_get_system_state(client), "  ");
}

static void
cmm_client_test_on_process_changed
(
	cmm_client_t * client
) {
	printf("  Process has changed, now:\n");
	cmm_client_test_print_process(cmm_client_get_process(client), "  ");
}

static void
cmm_client_test_on_request_completed
(
	cmm_client_t * client,
	aud_error_t result
) {
	(void) client;

	if (result == AUD_SUCCESS)
	{
		printf("  Request completed\n");
	}
	else
	{
		aud_errbuf_t errbuf;
		printf("  Request failed: %s\n", aud_error_message(result, errbuf));
	}
}

void
cmm_client_test_event
(
	cmm_client_t * client,
	const cmm_client_event_info_t * event_info
) {
	//cmm_client_test_t * test = (cmm_client_test_t *) cmm_client_get_context(client);

	printf("  Got an event, flags=0x%08x\n", event_info->flags);
	
	if (event_info->flags & CMM_CLIENT_EVENT_FLAG_CONNECTION_CHANGED)
	{
		cmm_client_test_on_connection_changed(client);
	}

	if (event_info->flags & CMM_CLIENT_EVENT_FLAG_OPTIONS_CHANGED)
	{
		cmm_client_test_on_options_changed(client);
	}

	if (event_info->flags & CMM_CLIENT_EVENT_FLAG_STATE_CHANGED)
	{
		cmm_client_test_on_state_changed(client);
	}

	if (event_info->flags & CMM_CLIENT_EVENT_FLAG_PROCESS_CHANGED)
	{
		cmm_client_test_on_process_changed(client);
	}

	if (event_info->flags & CMM_CLIENT_EVENT_FLAG_REQUEST_COMPLETED)
	{
		cmm_client_test_on_request_completed(client, event_info->result);
	}

}

//----------------------------------------------------------
// Main functions
//----------------------------------------------------------

static aud_error_t
cmm_client_test_run
(
	cmm_client_test_t * test
) {

#if 1
	dante_runtime_t * runtime = dapi_get_runtime(test->dapi);
	
	dante_sockets_t sockets;
	dante_sockets_clear(&sockets);
	aud_utime_t timeout = {1, 0};
	dante_runtime_get_sockets_and_timeout(runtime, &sockets, &timeout);
	if (timeout.tv_sec >= 1)
	{
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
	}
	int select_result = select(sockets.n, &sockets.read_fds, &sockets.write_fds, NULL, &timeout);
	if (select_result >= 0)
	{
		aud_error_t result = dante_runtime_process_with_sockets(runtime, &sockets);
		if (result != AUD_SUCCESS)
		{
			fprintf(stderr, "Error processing: %s\n", aud_error_get_name(result));
			return result;
		}
	}
	else
	{
		aud_error_t result = aud_error_get_last();
		fprintf(stderr, "Error select()ing: %s\n", aud_error_get_name(result));
		return result;
	}
#else
	aud_error_t result;
	aud_socket_t s = cmm_client_get_socket(test->client);
	aud_utime_t timeout = {1, 0};
	fd_set read_fds;
	int select_result;

	FD_ZERO(&read_fds);

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
	FD_SET(s, &read_fds);
#pragma warning(pop)
	select_result = select(1, &read_fds, NULL, NULL, &timeout);
#else
	FD_SET(s, &read_fds);
	select_result = select(s+1, &read_fds, NULL, NULL, &timeout);
#endif
	if (select_result < 0)
	{
		result = aud_system_error_get_last();
		
		//aud_log(test->env->log, AUD_LOG_ERROR, "Select error: %s\n", aud_error_message(result, errbuf)); 
		return result;
	}
	else if (select_result >= 0)
	{
		result = cmm_client_process(test->client, NULL);
		if (result != AUD_SUCCESS)
		{
			//aud_log(test->env->log, AUD_LOG_ERROR, "Error processing dvs client: %s\n", aud_error_message(result, errbuf)); 
			return result;
		}
	}
#endif
	return AUD_SUCCESS;
}

static aud_error_t
cmm_test_set_interfaces(cmm_client_test_t * test, const wchar_t * primaryName, const wchar_t * secondaryName)
{
	aud_error_t result;
	aud_errbuf_t errbuf;

	cmm_config_t * config;
	const cmm_options_t * options = cmm_client_get_system_options(test->client);
	const cmm_interface_t * iface;

	printf("\n");
	printf("Switching to interfaces \"%S\",\"%S\"...\n",
		primaryName ? primaryName : L"-",
		secondaryName ? secondaryName : L"-");

	config = cmm_client_get_temp_config(test->client);
	cmm_config_reset(config);
	if (primaryName)
	{
		iface = cmm_options_interface_with_name(options, primaryName);
		if (iface == NULL)
		{
			printf("Unknown interface \"%S\"!\n", primaryName);
			return AUD_ERR_INVALIDDATA;
		}
		cmm_config_add_interface(config,
			cmm_client_get_system_options(test->client),
			cmm_interface_get_mac_address(iface));
	}
	else
	{
		cmm_config_add_null_interface(config);
	}
	if (secondaryName)
	{
		if (cmm_info_max_dante_networks(cmm_client_get_system_info(test->client)) < 2)
		{
			printf("Eror: cannot configure secondary interface a non-redundant system!\n");
			return AUD_ERR_INVALIDDATA;
		}
		iface = cmm_options_interface_with_name(options, secondaryName);
		if (iface == NULL)
		{
			printf("Unknown interface \"%S\"!\n", secondaryName);
			return AUD_ERR_INVALIDDATA;
		}
		cmm_config_add_interface(config,
			cmm_client_get_system_options(test->client),
			cmm_interface_get_mac_address(iface));
	}
	else
	{
		if (cmm_info_max_dante_networks(cmm_client_get_system_info(test->client)) >= 2)
		{
			cmm_config_add_null_interface(config);
		}
	}

	{
		cmm_system_config_t system_config = {0};
		system_config.flags = CMM_SYSTEM_CONFIG_FLAG_CONFIG;
		system_config.config = config;
		system_config.lock = AUD_FALSE;
		result = cmm_client_set_system_config(test->client, &system_config);
	}
	if (result != AUD_SUCCESS)
	{
		printf("Error sending config: %s\n", aud_error_message(result, errbuf));
		return result;
	}
	while (cmm_client_has_active_request(test->client))
	{
		if (!running)
		{
			return AUD_ERR_INTERRUPTED;
		}
		cmm_client_test_run(test);
	}
	if (cmm_state_get_pending_config(cmm_client_get_system_state(test->client)))
	{
		printf("Action complete, waiting for pending configuration to be applied..\n");
		while (cmm_state_get_pending_config(cmm_client_get_system_state(test->client)))
		{
			if (!running)
			{
				return AUD_ERR_INTERRUPTED;
			}
			cmm_client_test_run(test);
		}
		printf("Action complete, configuration has been applied!\n");
	}
	else
	{
		printf("Action complete, no pending changes\n");
	}
	return AUD_SUCCESS;
}

static void usage(char * bin)
{
	printf("Usage: %s OPTIONS\n", bin);
	printf("  -l         connect to the server and listen for events\n");
	printf("  -i=NAME    switch to the interface with name NAME\n");
}

typedef enum
{
	CMM_CLIENT_TEST_MODE_NONE,
	CMM_CLIENT_TEST_MODE_LISTEN,
	CMM_CLIENT_TEST_MODE_SET_INTERFACES
} cmm_client_test_mode_t;


int main(int argc, char * argv[])
{
	cmm_client_test_mode_t mode = CMM_CLIENT_TEST_MODE_NONE;
	wchar_t names[2][CMM_INTERFACE_NAME_LENGTH];
	int a;
	uint16_t port = 0;
	uint16_t num_names = 0;

	aud_error_t result;
	aud_errbuf_t errbuf;

	cmm_client_test_t test;

	memset(names, 0, sizeof(names));

	for (a = 1; a < argc; a++)
	{
		const char * arg = argv[a];
		if (!strncmp(arg, "-l", 2))
		{
			mode = CMM_CLIENT_TEST_MODE_LISTEN;
		}
		else if (!strncmp(arg, "-p=", 3) && strlen(arg) > 3)
		{
			port = (uint16_t) atoi(arg+3);
		}
		else if (!strncmp(arg, "-i=", 3) && strlen(arg) > 3)
		{
			if (num_names < 2)
			{
				int i;
				const char * in = arg + 3;
				for (i = 0; i < CMM_INTERFACE_NAME_LENGTH-1 && *in; i++)
				{
					names[num_names][i] = in[i];
				}
				names[num_names][i] = '\0';
				mode = CMM_CLIENT_TEST_MODE_SET_INTERFACES;
				num_names++;
			}
		}
		else
		{
			usage(argv[0]);
			exit(0);
		}
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	result = dapi_new(&test.dapi);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Error creating env: %s\n", aud_error_message(result, errbuf));
		return result;
	}
	test.client = cmm_client_new_dapi(test.dapi);
	if (test.client == NULL)
	{
		fprintf(stderr, "Error creating client: NO_MEMORY\n");
		goto cleanup;
	}
	result = cmm_client_set_server_port(test.client, port);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Error setting client's server port: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}

	result = cmm_client_init(test.client, "test");
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Error initialising client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}
	cmm_client_set_context(test.client, &test);
	cmm_client_set_event_cb(test.client, &cmm_client_test_event);
	printf("Connecting...\n");
	result = cmm_client_connect(test.client);
	if (result != AUD_SUCCESS)
	{
		fprintf(stderr, "Error connecting client: %s\n", aud_error_message(result, errbuf)); 
		goto cleanup;
	}

	signal(SIGINT, signal_handler);

	printf("\n");
	printf("Waiting for connection...\n");
	while (cmm_client_get_connection_state(test.client) != CMM_CONNECTION_STATE_CONNECTED)
	{
		if (!running)
		{
			goto cleanup;
		}
		cmm_client_test_run(&test);
	}
	printf("Connected, waiting for options and state...\n");

	while (cmm_client_get_system_options(test.client) == NULL || cmm_client_get_system_state(test.client) == NULL)
	{
		if (!running)
		{
			goto cleanup;
		}
		cmm_client_test_run(&test);
	}
	printf("Acquired options and state\n");
	printf("\n");

	if (mode == CMM_CLIENT_TEST_MODE_SET_INTERFACES)
	{
		cmm_test_set_interfaces(&test, names[0][0] ? names[0] : NULL, names[1][0] ? names[1] : NULL);
	}
	else if (mode == CMM_CLIENT_TEST_MODE_LISTEN)
	{
		while (running)
		{
			cmm_client_test_run(&test);
		}
	}

cleanup:

	if (test.client)
	{
		printf("Disconnecting...\n");
		cmm_client_disconnect(test.client);
		cmm_client_terminate(test.client);
		cmm_client_delete(test.client);
	}
	if (test.dapi)
	{
		dapi_delete(test.dapi);
	}
	return 0;
}

/*
 * File     : $RCSfile$
 * Created  : 25 Nov 2010 15:51:56
 * Updated  : $Date$
 * Author   : Varuni Witana <varuni.witana@audinate.com>
 * Synopsis : Test program to send conmon audinate messages
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#include "conmon_audinate_controller.h"
#include "dapi_utils_domains.h"

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#define CONGESTION_DELAY ONE_SECOND_US

static conmon_audinate_control_handler_fn 
	conmon_audinate_interface_control_handler,
	conmon_audinate_switch_vlan_control_handler,
	conmon_audinate_clocking_control_handler,
	conmon_audinate_unicast_clocking_control_handler,
	conmon_audinate_name_id_control_handler,
	conmon_audinate_igmp_vers_control_handler,
	conmon_audinate_srate_control_handler,
	conmon_audinate_srate_pullup_control_handler,
	conmon_audinate_enc_control_handler,
	conmon_audinate_access_control_handler,
	conmon_audinate_edk_board_control_handler,
	conmon_audinate_ifstats_control_handler,
	conmon_audinate_rx_error_threshold_control_handler,
	conmon_audinate_sys_reset_handler,
	conmon_audinate_metering_control_handler,
	conmon_audinate_serial_port_control_handler,
	conmon_audinate_haremote_control_handler,
	conmon_audinate_upgrade_v3_control_handler,	
	conmon_audinate_clear_config_control_handler,
	conmon_audinate_gpio_control_handler,
	conmon_audinate_ptp_logging_control_handler,
	conmon_audinate_aes67_control_handler;

static conmon_audinate_control_usage_fn
	conmon_audinate_interface_control_usage,
	conmon_audinate_switch_vlan_control_usage,
	conmon_audinate_clocking_control_usage,
	conmon_audinate_unicast_clocking_control_usage,
	conmon_audinate_master_control_usage,
	conmon_audinate_name_id_control_usage,
	conmon_audinate_igmp_vers_control_usage,
	conmon_audinate_vers_control_usage,
	conmon_audinate_srate_control_usage,
	conmon_audinate_srate_pullup_control_usage,
	conmon_audinate_enc_control_usage,
	conmon_audinate_access_control_usage,
	conmon_audinate_manf_vers_control_usage,
	conmon_audinate_identify_control_usage,
	conmon_audinate_edk_board_control_usage,
	conmon_audinate_ifstats_control_usage,
	conmon_audinate_rx_error_threshold_control_usage,
	conmon_audinate_rxerrq_control_usage,
	conmon_audinate_sys_reset_usage,
	conmon_audinate_metering_control_usage,
	conmon_audinate_audio_interface_usage,
	conmon_audinate_serial_port_control_usage,
	conmon_audinate_haremote_control_usage,
	conmon_audinate_audio_interface_usage,
	conmon_audinate_upgrade_v3_control_usage,
	conmon_audinate_clear_config_control_usage,
	conmon_audinate_gpio_control_usage,
	conmon_audinate_ptp_logging_control_usage,
	conmon_audinate_dante_ready_usage,	
	conmon_audinate_routing_ready_usage,	
	conmon_audinate_haremote_stats_usage,	
	conmon_audinate_aes67_control_usage,
	conmon_audinate_lock_status_usage;

typedef struct conmon_audinate_control_handler_map
{
	conmon_audinate_message_type_t type;
	char cmd[16];
	conmon_audinate_control_handler_fn * handler;
	conmon_audinate_control_usage_fn * usage;
} conmon_audinate_control_handler_map;

static const conmon_audinate_control_handler_map
audinate_control_map[] =
{
	{CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_CONTROL, "interface", conmon_audinate_interface_control_handler, conmon_audinate_interface_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_SWITCH_VLAN_CONTROL, "switch_vlan", conmon_audinate_switch_vlan_control_handler, conmon_audinate_switch_vlan_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_CONTROL, "clocking",  conmon_audinate_clocking_control_handler, conmon_audinate_clocking_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_UNICAST_CLOCKING_CONTROL, "uclocking",  conmon_audinate_unicast_clocking_control_handler, conmon_audinate_unicast_clocking_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_MASTER_QUERY, "master", NULL, conmon_audinate_master_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_NAME_ID_CONTROL, "name_id", conmon_audinate_name_id_control_handler, conmon_audinate_name_id_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_IFSTATS_QUERY, "ifstats", conmon_audinate_ifstats_control_handler, conmon_audinate_ifstats_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_IGMP_VERS_CONTROL, "igmp", conmon_audinate_igmp_vers_control_handler, conmon_audinate_igmp_vers_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_VERSIONS_QUERY, "versions", NULL, conmon_audinate_vers_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_SRATE_CONTROL, "srate", conmon_audinate_srate_control_handler, conmon_audinate_srate_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_SRATE_PULLUP_CONTROL, "pullup", conmon_audinate_srate_pullup_control_handler, conmon_audinate_srate_pullup_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_ENC_CONTROL, "enc", conmon_audinate_enc_control_handler, conmon_audinate_enc_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_AUDIO_INTERFACE_QUERY, "audio_interface", NULL, conmon_audinate_audio_interface_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_SYS_RESET, "sysreset", conmon_audinate_sys_reset_handler, conmon_audinate_sys_reset_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_EDK_BOARD_CONTROL, "edk", conmon_audinate_edk_board_control_handler, conmon_audinate_edk_board_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_ACCESS_CONTROL, "access", conmon_audinate_access_control_handler, conmon_audinate_access_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_MANF_VERSIONS_QUERY, "manf_versions", NULL, conmon_audinate_manf_vers_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_IDENTIFY_QUERY, "identify",  NULL, conmon_audinate_identify_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_RX_ERROR_THRES_CONTROL, "errthres", conmon_audinate_rx_error_threshold_control_handler, conmon_audinate_rx_error_threshold_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_RX_CHANNEL_RX_ERROR_QUERY, "rxerrq", NULL, conmon_audinate_rxerrq_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_METERING_CONTROL, "metering", conmon_audinate_metering_control_handler, conmon_audinate_metering_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_SERIAL_PORT_CONTROL, "serial", conmon_audinate_serial_port_control_handler, conmon_audinate_serial_port_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_V3_CONTROL, "upgrade3", conmon_audinate_upgrade_v3_control_handler, conmon_audinate_upgrade_v3_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_CLEAR_CONFIG_CONTROL, "clear_config", conmon_audinate_clear_config_control_handler, conmon_audinate_clear_config_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_QUERY, "routing_ready", NULL, conmon_audinate_routing_ready_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_CONTROL, "haremote", conmon_audinate_haremote_control_handler, conmon_audinate_haremote_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_STATS_QUERY, "haremote_stats", NULL, conmon_audinate_haremote_stats_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_DANTE_READY_QUERY, "dante_ready", NULL, conmon_audinate_dante_ready_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_PTP_LOGGING_CONTROL, "ptp_logging", conmon_audinate_ptp_logging_control_handler, conmon_audinate_ptp_logging_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_GPIO_QUERY, "gpio", conmon_audinate_gpio_control_handler, conmon_audinate_gpio_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_AES67_CONTROL, "aes67", conmon_audinate_aes67_control_handler, conmon_audinate_aes67_control_usage},
	{CONMON_AUDINATE_MESSAGE_TYPE_LOCK_QUERY, "lock", NULL, conmon_audinate_lock_status_usage},

	{0, "", 0, 0}
		// Terminator
};



// how long to wait for a response from the server
const aud_utime_t g_comms_timeout = {2, 0};

//const aud_utime_t g_control_timeout = {1, 500000};

aud_bool_t g_communicating = AUD_FALSE;

aud_error_t g_last_result = AUD_SUCCESS;

static conmon_client_response_fn handle_response;

static void
handle_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
) {
	// TODO: should check request id codes to ensure correct matching
	aud_errbuf_t errbuf;

	(void) client;

	printf ("Got response for request 0x%p: %s\n",
		request_id, aud_error_message(result, errbuf));
	g_communicating = AUD_FALSE;
	g_last_result = result;
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
	while(g_communicating && aud_utime_compare(&now, &then) < 0)
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

static conmon_client_handle_networks_changed_fn handle_networks_changed;

static void
handle_networks_changed
(
	conmon_client_t * client
) {
	char buf[1024];
	const conmon_networks_t * networks = conmon_client_get_networks(client);

	conmon_example_networks_to_string(networks, buf, 1024);
	printf("NETWORKS CHANGED: %s\n", buf);
}


static void usage(char * cmd)
{
	fputc ('\n', stderr);
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	fprintf(stderr,"%s %s", cmd, "[domain options] device [command] -h for command details\n\n");
	fprintf(stderr, "domain options:\n");
	fprintf(stderr, "  --ddm  Enable domains, using discovery to find the DDM\n");
	fprintf(stderr, "  --ddm=HOST:PORT  Enable domains, using the specified DDM address (bypasses DDM discovery)\n");
	fprintf(stderr, "  --user=USER  Specify the username for DDM authentication\n");
	fprintf(stderr, "  --pass=PASS  Specify a password for DDM authentication\n");
	fprintf(stderr, "  --domain=DOMAIN  Specify a domain to use once connected to the DDM\n");
	fprintf(stderr, "\n");
#else
 	fprintf(stderr,"%s %s", cmd, "[options] [device] [command] -h for command details\n\n");
#endif
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "-p PORT: specify the local conmon server port\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Control channel messages:\n");
	fprintf(stderr, "  * Use device name 'localhost' to indicate the local device\n");
	fprintf(stderr,"%s", "  interface       query or configure dante interfaces\n");
	fprintf(stderr,"%s", "  switch_vlan     query or configure switch vlans\n");
	fprintf(stderr,"%s", "  clocking        query or configure clocking\n");
	fprintf(stderr,"%s", "  uclocking       query or configure unicast clocking\n");
	fprintf(stderr,"%s", "  ifstats         query interface status or clear error counters\n");
	fprintf(stderr,"%s", "  igmp            query or configure igmp version\n");
	fprintf(stderr,"%s", "  versions        query dante versions and capabilities\n");
	fprintf(stderr,"%s", "  srate           query or configure sample rate\n");
	fprintf(stderr,"%s", "  pullup          query or configure sample rate pullup & subdomain name\n");
	fprintf(stderr,"%s", "  enc             query or configure encoding [pcm16|pcm24|pcm32|raw32]\n");
	fprintf(stderr,"%s", "  sysreset        soft or factory reset a device\n");
	fprintf(stderr,"%s", "  edk             query or configure pdk baseboard codec [spdif|aes|tos]\n");
	fprintf(stderr,"%s", "  manf_versions   query manufacturer message information\n");
	fprintf(stderr,"%s", "  identify        trigger device identify\n");
	fprintf(stderr,"%s", "  errthres        query or configure receive audio error threshold values\n");
	fprintf(stderr,"%s", "  rxerrq          query for receive audio errors\n");
	fprintf(stderr,"%s", "  metering        query or configure audio metering rate\n");
	fprintf(stderr,"%s", "  serial          query or configure serial interface\n");
	fprintf(stderr,"%s", "  upgrade3        trigger a firmware upgrade\n");
	fprintf(stderr,"%s", "  clear_config    query or command for clearing device config\n");
	fprintf(stderr,"%s", "  routing_ready   query for routing ready status\n");
	fprintf(stderr,"%s", "  haremote        query or configure haremote options\n");
	fprintf(stderr,"%s", "  haremote_stats  query haremote statistics\n");
	fprintf(stderr,"%s", "  ptp_logging     query or configure ptp logging (enable/disable)\n");
	fprintf(stderr,"%s", "  gpio            query or configure gpio mode\n");
	fprintf(stderr,"%s", "  aes67           query or configure aes67 state (enable/disable)\n");
	fprintf(stderr,"%s", "  lock            query device lock status (enabled/disabled)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Broadcast channel messages:\n");
	fprintf(stderr, "  * device must be 'broadcast', message are always sent to all devices in domain\n");
	fprintf(stderr,"%s", "  master          query for PTP masters\n");
	fprintf(stderr,"%s", "  name_id         query for device id or process id or uuid\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Local Channel Messages:\n");
	fprintf(stderr, "  * device must be 'localhost', message is only sent to local device)\n");
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	fprintf(stderr, "  * Local channel not supported for Standalone controllers in managed domains\n");	
#endif
	fprintf(stderr,"%s", "  access          query or configure inetd access enabled/disabled\n");
	fprintf(stderr,"%s", "  dante_ready     query dante ready status\n");
}

static aud_error_t parse_msg_args
(
	int argc, 
	char **argv, 
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	int i;
	aud_bool_t help_cmd = AUD_FALSE;

	// help with this command ?
	if(argc >= 4)
	{
		if(strcmp(argv[3], "-h") == 0)
			help_cmd = AUD_TRUE;
	}

	for (i = 0; audinate_control_map [i].cmd [0]; i++)
	{
		if(strcmp(argv[2], audinate_control_map[i].cmd) == 0)
		{
			conmon_audinate_control_handler_fn * handler;
			handler = audinate_control_map[i].handler;
			//conmon_audinate_message_head_initialise((conmon_audinate_message_head_t*)body,
			//	audinate_control_map[i].type, ONE_SECOND_US);

			// This is only port help
			if(help_cmd == AUD_TRUE)
			{
				conmon_audinate_control_usage_fn * usage;
				usage = audinate_control_map[i].usage;

				if(usage)
				{
					// Run usage for this command
					(*audinate_control_map[i].usage)(argv[0]);
				}
				else
				{
					fprintf(stderr,"%s - %s\n", "no additional help available for this command", argv[2]);
				}
				return AUD_ERR_DONE;
			}

			if(handler) 
			{
				aud_error_t result;

				if((result = (* handler)(argc, argv, body, body_size)) == AUD_ERR_INVALIDPARAMETER)
				{
					if(audinate_control_map[i].usage)
					{
						(*audinate_control_map[i].usage)(argv[0]);
					}
					else
					{
						usage(argv[0]);
					}
				}
				return result;
			} 
			else 
			{
				conmon_audinate_init_query_message(body, audinate_control_map[i].type, CONGESTION_DELAY);
				*body_size = conmon_audinate_query_message_get_size(body);
				return AUD_SUCCESS;
			}
		}
	}

	// unknown help for this command
	if(help_cmd == AUD_TRUE)
	{
		fprintf(stderr,"%s - %s", "no help available for this command\n", argv[2]);
		usage(argv[0]);
		return AUD_ERR_DONE;
	}
	else
	{
		usage(argv[0]);
		return AUD_ERR_INVALIDPARAMETER;
	}
}

int main(int argc, char **argv)
{
	aud_error_t result;
	aud_errbuf_t errbuf;
	conmon_client_request_id_t req_id;
	
	dapi_t * dapi = NULL;
	dante_runtime_t * runtime = NULL;
	dante_domain_handler_t * handler = NULL;
	conmon_client_t * client = NULL;
	uint16_t server_port = 0;
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	dapi_utils_ddm_config_t ddm_config = {0};
#endif

	conmon_name_t controlled_name;
	aud_utime_t control_timeout = {5, 0};

	conmon_message_body_t body;
	uint16_t body_size = 0; // = sizeof(conmon_audinate_message_head_t); // message with no payload

	// with only help option
	if(argc == 2)
	{
		if(strcmp(argv[1], "-h") == 0)
			usage(argv[0]);
	}

	if(argc < 3)
	{
		usage(argv[0]);
		result = AUD_ERR_INVALIDPARAMETER;
		goto cleanup;
	}

	// Parse options
	if (!strcmp(argv[1], "-p"))
	{
		server_port = atoi(argv[2]);
		memcpy(argv+1, argv+3, (argc-3) * sizeof(char *));
		argc -= 2;
	}
	if (argc < 3)
	{
		usage(argv[0]);
		result = AUD_ERR_INVALIDPARAMETER;
		goto cleanup;
	}



#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_config_parse(&ddm_config, &argc, argv, 1, AUD_TRUE); 
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif

	result = parse_msg_args(argc, argv, &body, &body_size);
	if (result == AUD_ERR_INVALIDPARAMETER || result == AUD_ERR_DONE)
	{
		goto cleanup;
	}
	
	// get the name of the device to be controlled
	if (!strcmp(argv[1], "localhost"))
	{
		controlled_name[0] = '\0';
	}
	else if (!strcmp(argv[1], "broadcast"))
	{
		controlled_name[0] = '\0';
	}
	else
	{
		SNPRINTF(controlled_name, CONMON_NAME_LENGTH, "%s", argv[1]);
	}

		
	result = dapi_new(&dapi);
	if (result != AUD_SUCCESS)
	{
		printf("Error initialising conmon client library: %s\n",
			aud_error_message(result, errbuf));
		goto cleanup;
	}

	runtime = dapi_get_runtime(dapi);
	handler = dapi_get_domain_handler(dapi);
	
	conmon_client_config_t * config = conmon_client_config_new("conmon_audinate_controller");
	conmon_client_config_set_server_port(config, server_port);
	result = conmon_client_new_dapi(dapi, config, & client);
	conmon_client_config_delete(config);

	if (result != AUD_SUCCESS || client == NULL)
	{
		printf("Error creating client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}
	
	// set before connecting to avoid possible race conditions / missed notifications
	conmon_client_set_networks_changed_callback(client, handle_networks_changed);

#if 0
	conmon_examples_event_loop_init(client);
#endif	
	result = conmon_client_connect (client, & handle_response, & req_id); // store client at pos 0 of array
	if (result == AUD_SUCCESS)
	{
		printf("Connecting, request id is 0x%p\n", req_id);
	}
	else
	{
		printf("Error connecting client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}
	result = wait_for_response(runtime, &g_comms_timeout);
	if (result != AUD_SUCCESS)
	{
		printf("Error connecting client: %s\n", aud_error_message(result, errbuf));
		goto cleanup;
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&ddm_config, handler, runtime, NULL);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}

	dante_domain_info_t current = dante_domain_handler_get_current_domain(handler);

	//Is current domain the one that we want to be on?
	if (ddm_config.domain[0] && strcmp(current.name, ddm_config.domain))
	{
		printf("WARNING: Unable to send Query/Config as requested domain is not available\n");
		goto cleanup;
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(handler);
	dante_domain_handler_set_event_fn(handler, dapi_utils_ddh_event_print_changes);


	// Make sure the client is in a position to send control messages to the right destinations
	if (controlled_name[0])
	{
		printf("Waiting for client to be ready for remote control\n");
		while (!conmon_client_is_remote_ready(client))
		{
			dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		}
	}
	else
	{
		printf("Waiting for client to be ready for local control\n");
		while (!conmon_client_is_local_ready(client))
		{
			dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		}
	}
	
	if((!strncmp(argv[2],"access", 6))||(!strncmp(argv[2],"dante_ready",11)))
	{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		if (IS_MANAGED_DOMAIN_UUID(dante_domain_handler_get_current_domain_uuid(handler)))
		{
			printf("Local channel messages are not supported for standalone controllers in managed domains");
			return AUD_ERR_INVALIDSTATE;
		}
#endif
		if (strcmp(argv[1], "localhost"))
		{
			printf("Invalid argument - target device must be 'localhost' for local channel messages\n");
			return AUD_ERR_INVALIDPARAMETER;
		}
		result = conmon_client_send_monitoring_message(client,
			handle_response, &req_id,
			CONMON_CHANNEL_TYPE_LOCAL,
			CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, CONMON_VENDOR_ID_AUDINATE,
			&body, body_size);
		if (result != AUD_SUCCESS)
		{
			printf ("Error sending local control message (request): %s\n", aud_error_message(result, errbuf));
		}
		else
		{
			printf("sending local control message with request_id %p\n", req_id);
			result = wait_for_response(runtime, &g_comms_timeout);
			if (result != AUD_SUCCESS)
			{
				printf ("Error sending local control message (response): %s\n", aud_error_message(result, errbuf));
			}
		}
	}
	else if (!strncmp(argv[2], "master",6) || !strncmp(argv[2], "name_id", 7))
	{
		if (strcmp(argv[1], "broadcast"))
		{
			printf("Invalid argument - target device must be 'broadcast' for broadcast channel messages\n");
			return AUD_ERR_INVALIDPARAMETER;
		}
		result = conmon_client_send_monitoring_message(client,
			handle_response, &req_id,
			CONMON_CHANNEL_TYPE_BROADCAST,
			CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, CONMON_VENDOR_ID_AUDINATE,
			&body, body_size);
		if (result != AUD_SUCCESS)
		{
			printf ("Error sending broadcast message (request): %s\n", aud_error_message(result, errbuf));
		}
		else
		{
			printf("sent broadcast message with request id %p\n", req_id);
			result = wait_for_response(runtime, &g_comms_timeout);
			if (result != AUD_SUCCESS)
			{
				printf ("Error sending broadcast message (response): %s\n", aud_error_message(result, errbuf));
			}
		}	
	}
	else
	{
		// we're connected, send control message
		result = conmon_client_send_control_message(client,
				handle_response, &req_id,
				controlled_name, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, CONMON_VENDOR_ID_AUDINATE,
			&body, body_size,
			&control_timeout); // the server will give up trying after this long, so that we don't wait forever for a response
		if (result != AUD_SUCCESS)
		{
			printf ("Error sending control message (request): %s\n", aud_error_message(result, errbuf));
		}
		else
		{
			printf("sent control message with request id %p\n", req_id);
			result = wait_for_response(runtime, &g_comms_timeout);
			if (result != AUD_SUCCESS)
			{
				printf ("Error sending control message (response): %s\n", aud_error_message(result, errbuf));
			}
		}
	}

cleanup:
	if (client)
	{
		conmon_client_delete(client);
			// Note: this calls conmon_client_disconnect as part of clean-up
	}
	if (dapi)
	{
		dapi_delete(dapi);
	}
	return result;
}

static uint32_t parse_address(const char * msg)
{
	uint32_t addr;
	int v1, v2, v3, v4;

	if (sscanf(msg, "%d.%d.%d.%d", &v1, &v2, &v3, &v4) < 4)
	{
		return 0;
	}
	if (v1 < 0 || v1 > 255 || v2 < 0 || v2 > 255 || v3 < 0 || v3 > 255 || v4 < 0 || v4 > 255)
	{
		return 0;
	}
	addr = v1 << 24 | v2 << 16 | v3 << 8 | v4;
	return addr;
}

aud_error_t conmon_audinate_interface_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	char * str;
	const char * domain_name = NULL;
	conmon_audinate_init_interface_control(body, 0);
	
	if(argc == 3)
	{
		// no params send null message
		printf("sending query message\n");
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 5 && !strcmp(argv[3], "redundancy") && !strcmp(argv[4], "true"))
	{
		conmon_audinate_interface_control_set_switch_redundancy(body, AUD_TRUE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 5 && !strcmp(argv[3], "redundancy") && !strcmp(argv[4], "false"))
	{
		conmon_audinate_interface_control_set_switch_redundancy(body, AUD_FALSE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 5 && !strcmp(argv[4], "dynamic"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		conmon_audinate_interface_control_set_interface_address_dynamic(body, index);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 6 && !strcmp(argv[4], "ll") && !strcmp(argv[5], "true"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		conmon_audinate_interface_control_set_link_local_enabled(body, index, AUD_TRUE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 6 && !strcmp(argv[4], "ll") && !strcmp(argv[5], "false"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		conmon_audinate_interface_control_set_link_local_enabled(body, index, AUD_FALSE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 6 && !strcmp(argv[4], "ll_delay") && !strcmp(argv[5], "true"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		conmon_audinate_interface_control_set_link_local_delayed(body, index, AUD_TRUE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 6 && !strcmp(argv[4], "ll_delay") && !strcmp(argv[5], "false"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		conmon_audinate_interface_control_set_link_local_delayed(body, index, AUD_FALSE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc >= 7 && !strcmp(argv[4], "static"))
	{
		uint16_t index = (uint16_t) atoi(argv[3]);
		uint32_t ip_address = htonl(parse_address(argv[5]));
		uint32_t netmask = htonl(parse_address(argv[6]));
		uint32_t dns_server  = 0;
		uint32_t gateway  = 0;
		if(argc >= 8)
		{
			dns_server = htonl(parse_address(argv[7]));
		}
		if(argc >= 9)
		{
			gateway = htonl(parse_address(argv[8]));
		}
		if(argc >= 0x0a)
		{
			str = argv[9];
			if (str[0])
			{
				domain_name = str;
			}
		}
		if (ip_address && netmask)
		{
			conmon_audinate_interface_control_set_interface_address_static(body, index,
				ip_address, netmask, dns_server, gateway);
			*body_size = conmon_audinate_interface_control_get_size(body);
			if (domain_name)
			{
				conmon_message_size_info_t size = { CONMON_MESSAGE_MAX_BODY_SIZE };	
				size.curr = *body_size;
				conmon_audinate_interface_control_set_interface_domain_name(body,&size ,index,domain_name);
				*body_size = (uint16_t)size.curr;
			}
			return AUD_SUCCESS;
		}
	}
	else if (argc == 6 && !strcmp(argv[4], "dhcp") && !strcmp(argv[5], "true"))
	{
		uint16_t index = (uint16_t)atoi(argv[3]);
		conmon_audinate_interface_control_set_dhcp_enabled(body, index, AUD_TRUE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 6 && !strcmp(argv[4], "dhcp") && !strcmp(argv[5], "false"))
	{
		uint16_t index = (uint16_t)atoi(argv[3]);
		conmon_audinate_interface_control_set_dhcp_enabled(body, index, AUD_FALSE);
		*body_size = conmon_audinate_interface_control_get_size(body);
		return AUD_SUCCESS;
	}

	return AUD_ERR_INVALIDPARAMETER;
}


aud_error_t conmon_audinate_switch_vlan_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	conmon_audinate_init_switch_vlan_control(body, 0);
	if(argc == 3)
	{
		printf("sending query message\n");
		*body_size = conmon_audinate_switch_vlan_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 4 && !strncmp(argv[3], "id=", 3) && strlen(argv[3]) > 3)
	{
		uint16_t id = (uint16_t) atoi(argv[3]+3);
		conmon_audinate_switch_vlan_control_set_config_id(body, id);
		*body_size = conmon_audinate_switch_vlan_control_get_size(body);
		return AUD_SUCCESS;
	}

	return AUD_ERR_INVALIDPARAMETER;

}



static char *
find_char
(
	char * src,
	char ch
)
{
	unsigned i;
	for (i = 0; src[i]; i++)
	{
		if (src[i] == ch)
		{
			return &src[i];
		}
	}

	return NULL;
}


static dante_id64_t *
parse_id64
(
	const char * str_in,
	dante_id64_t * dst_id
)
{
	// If it's quoted, then treat as string form
	const char * str = str_in;
	char ch = str[0];
	if (ch == '"' || ch == '\'')
	{
		const char * end;
		str++;
		end = find_char((char *) str, ch);
		if (end && (end - str) >= 8)
		{
			if (dante_id64_from_ascii_string(dst_id, str) == AUD_SUCCESS)
				return dst_id;
		}
	}
	else
	{
		if (dante_id64_from_hex_string(dst_id, str) == AUD_SUCCESS)
			return dst_id;
	}
	fprintf(stderr, "Invalid identifier '%s'\n", str_in);
	return NULL;
}


aud_error_t
conmon_audinate_upgrade_v3_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	aud_error_t result;
	conmon_audinate_upgrade_source_file_t src_file = { 0 };
	conmon_audinate_upgrade_xmodem_params_t xm_params = { 0 };
	char * arg;
	char * str;
	dante_id64_t mf, model, *mfp = NULL, *modelp = NULL;
	conmon_audinate_upgrade_additional_target_t additional_target = { 0 };

	conmon_message_size_info_t size = { 0 };

	conmon_audinate_init_upgrade_control_v3(body, &size, 0);

	argc -= 3;
	argv += 3;

	if (!argc)
		goto l__error;

	arg = argv[0];
	if (strncmp(arg, "tftp=", 5) == 0)
	{
		arg += 5;
		str = find_char(arg, ':');
		if (str)
		{
			str[0] = 0;
			str++;
			src_file.port = (uint16_t) atoi(str);
			if (! src_file.port)
			{
				fprintf(stderr, "Invalid port '%s'\n", str);
				goto l__error;
			}
		}
		else
		{
			src_file.port = 69;
		}
		src_file.protocol = CONMON_AUDINATE_UPGRADE_PROTOCOL_TFTP_GET;
		src_file.addr_inet.s_addr = inet_addr(arg);
		if (! src_file.addr_inet.s_addr)
		{
			fprintf(stderr, "Invalid TFTP server address '%s'\n", arg);
			goto l__error;
		}

		argc--;
		argv++;
	}
	else if (strcmp(arg, "local") == 0)
	{
		argc--;
		argv++;
	}
	else if (strcmp(arg, "query") == 0)
	{
		argc--;
		argv++;
		if (argc > 0)
		{
			goto l__error;
		}
		* body_size = (uint16_t) size.curr;
		return AUD_SUCCESS;
	}
	else if (strncmp(arg, "xmodem=", 7) == 0)
	{
		arg += 7;
		if (strcmp(arg, "uarta") == 0)
		{
			xm_params.port = XMODEM_PORT_UARTA;
		}
		else if (strcmp(arg, "uartb") == 0)
		{
			xm_params.port = XMODEM_PORT_UARTB;
		}
		else if (strcmp(arg, "spi0") == 0)
		{
			xm_params.port = XMODEM_PORT_SPI0;
		}
		else
		{
			goto l__error;
		}

		src_file.protocol = CONMON_AUDINATE_UPGRADE_PROTOCOL_XMODEM;

		argc--;
		argv++;

		if (argc)
		{
			arg = argv[0];
			if (strncmp(arg, "len=", 4) == 0)
			{
				arg += 4;
				src_file.file_len = atoi(arg);

				argc--;
				argv++;
			}
			else
			{
				goto l__error;
			}
		}
		else
		{
			goto l__error;
		}

		if (argc)
		{
			arg = argv[0];
			if (strncmp(arg, "baud=", 5) == 0)
			{
				arg += 5;
				xm_params.baud_rate = htonl(atoi(arg));

				argv++;
				argc--;
			}
			else
			{
				goto l__error;
			}			
		}
		else
		{
			goto l__error;
		}

		if (argc)
		{
			if (xm_params.port == XMODEM_PORT_SPI0)
			{
				arg = argv[0];
				if (strncmp(arg, "mstrpol=", 8) == 0)
				{
					arg += 8;
					xm_params.s_params.spi[XMODEM_SPI_DEV_MASTER].polarity = htons((uint16_t)atoi(arg));

					argv++;
					argc--;
				}
				else
				{
					goto l__error;
				}				

				if (argc)
				{
					arg = argv[0];
					if (strncmp(arg, "mstrpha=", 8) == 0)
					{
						arg += 8;
						xm_params.s_params.spi[XMODEM_SPI_DEV_MASTER].phase = htons((uint16_t)atoi(arg));

						argv++;
						argc--;
					}
					else
					{
						goto l__error;
					}					
				}
				if (argc)
				{
					arg = argv[0];
					if (strncmp(arg, "slvpol=", 7) == 0)
					{
						arg += 7;
						xm_params.s_params.spi[XMODEM_SPI_DEV_SLAVE].polarity = htons((uint16_t)atoi(arg));

						argv++;
						argc--;
					}
					else
					{
						goto l__error;
					}					
				}
				if (argc)
				{
					arg = argv[0];
					if (strncmp(arg, "slvpha=", 7) == 0)
					{
						arg += 7;
						xm_params.s_params.spi[XMODEM_SPI_DEV_SLAVE].phase = htons((uint16_t)atoi(arg));

						argv++;
						argc--;
					}
				}
				else
				{
					goto l__error;
				}
			}
			else
			{
				arg = argv[0];
				if (strncmp(arg, "parity=", 7) == 0)
				{
					arg += 7;
					xm_params.s_params.uart.parity = htons((uint16_t)atoi(arg));

					argv++;
					argc--;
				}
				else
				{
					goto l__error;
				}				

				if (argc)
				{
					arg = argv[0];
					if (strncmp(arg, "flwctrl=", 8) == 0)
					{
						arg += 8;
						xm_params.s_params.uart.flow_control = htons((uint16_t)atoi(arg));

						argv++;
						argc--;
					}
					else
					{
						goto l__error;
					}
				}
				else
				{
					goto l__error;
				}				
			}
		}
		else
		{
			goto l__error;
		}

		xm_params.port = htonl(xm_params.port);
	}
	else
	{
		goto l__error;
	}

	if (src_file.protocol != CONMON_AUDINATE_UPGRADE_PROTOCOL_XMODEM)
	{
		if (argc)
		{
			arg = argv[0];
			if (strncmp(arg, "path=", 5) == 0)
			{
				arg += 5;
				src_file.filename = arg;

				argc--;
				argv++;
			}
			else
			{
				goto l__error;
			}
		}
		else
		{
			goto l__error;
		}
	}

	if (argc)
	{
		arg = argv[0];
		if (strncmp(arg, "len=", 4) == 0)
		{
			arg += 4;
			src_file.file_len = atoi(arg);

			argc--;
			argv++;
		}
	}

	result =
		conmon_audinate_upgrade_control_set_source_file(
			body, &size, &src_file
		);
	if (result != AUD_SUCCESS)
	{
		return result;
	}

	if (src_file.protocol == CONMON_AUDINATE_UPGRADE_PROTOCOL_XMODEM)
	{
		result =
			conmon_audinate_upgrade_control_set_xmodem_params(
				body, &size, &xm_params
			);

		if (result != AUD_SUCCESS)
		{
			return result;
		}
	}

	while (argc)
	{
		arg = argv[0];
		if (strncmp(arg, "mf=", 3) == 0)
		{
			mfp = parse_id64(arg+3, &mf);
			if (! mfp)
				goto l__error;
		}
		else if (strncmp(arg, "model=", 6) == 0)
		{
			modelp = parse_id64(arg+6, &model);
			if (! modelp)
				goto l__error;
		}
		else if (strncmp(arg, "addip=", 6) == 0)
		{
			additional_target.addr_inet.s_addr = inet_addr(arg+6);
			if (additional_target.addr_inet.s_addr == 0) {
				goto l__error;
			}
		}
		else if (strncmp(arg, "addport=", 8) == 0)
		{
			additional_target.port = atoi(arg+8);
			if (additional_target.port == 0) {
				goto l__error;
			}
		}
		else
		{
			goto l__error;
		}

		argc--;
		argv++;
	}

	conmon_audinate_upgrade_control_set_override(body, mfp, modelp);

	if (additional_target.addr_inet.s_addr != 0 && additional_target.port != 0)
	{
		conmon_audinate_upgrade_control_set_additional_target(body, &size, &additional_target);
	}

	* body_size = (uint16_t) size.curr;
	return AUD_SUCCESS;

l__error:
	return AUD_ERR_INVALIDPARAMETER;
}

aud_error_t
conmon_audinate_clear_config_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	conmon_message_size_info_t size = { 0 };
	conmon_audinate_clear_config_action_t action = CONMON_AUDINATE_CLEAR_CONFIG_QUERY;

	conmon_audinate_init_clear_config_control(body, &size, 0);
	
	if ((argc == 4) && !strcmp(argv[3], "keep_ip"))
		action = CONMON_AUDINATE_CLEAR_CONFIG_KEEP_IP;
	else if ((argc == 4) && !strcmp(argv[3], "all"))
		action = CONMON_AUDINATE_CLEAR_CONFIG_CLEAR_ALL;
	else if ((argc == 4) && !strcmp(argv[3], "query"))
		action = CONMON_AUDINATE_CLEAR_CONFIG_QUERY;
	else if (argc >= 3)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	
	conmon_audinate_clear_config_control_set_action(body, &size, action);

	* body_size = (uint16_t) size.curr;
	return AUD_SUCCESS;
}

aud_error_t
conmon_audinate_gpio_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	uint16_t action = CONMON_AUDINATE_GPIO_CONTROL_FIELD_QUERY_STATE;
	uint32_t output_state_valid_mask;
	uint32_t output_state_values;
	char * str;

	output_state_valid_mask = output_state_values = 0;

	if ((argc == 4) && !strcmp(argv[3], "query"))
		action = CONMON_AUDINATE_GPIO_CONTROL_FIELD_QUERY_STATE;
	else if ((argc == 6) && !strcmp(argv[3], "output"))
	{
		action = CONMON_AUDINATE_GPIO_CONTROL_FIELD_OUTPUT_STATE;

		if(!strncmp(argv[4], "mask=0x", 7) && strlen(argv[4]) > 7)
		{
			str = argv[4] + 7;
			sscanf(str, "%x", &output_state_valid_mask);
		}

		if(!strncmp(argv[5], "val=0x", 6) && strlen(argv[5]) > 6)
		{
			str = argv[5] + 6;
			sscanf(str, "%x", &output_state_values);
		}
	}
	else if (argc >= 3)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	conmon_audinate_init_gpio_control(body, 0, action, 1);
	conmon_audinate_gpio_control_state_set_at_index(body, 0, output_state_valid_mask, output_state_values);

	* body_size = (uint16_t) conmon_audinate_gpio_control_get_size(body);
	return AUD_SUCCESS;
}

aud_error_t conmon_audinate_clocking_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	//conmon_audinate_clocking_control_t * clock_control = (conmon_audinate_clocking_control_t *)body;
	uint16_t param;
	char * str;

	conmon_audinate_init_clocking_control(body,0);
	// parse args
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else if(!strncmp(argv[3], "src", 3) && strlen(argv[3]) > 3)
	{
		str = argv[3] + 4;
		if(!strncmp(str,"bnc",3)) 
		{
			param = CONMON_AUDINATE_CLOCK_SOURCE_BNC;
		} 
		else if (!strncmp(str,"aes",3))
		{
			param = CONMON_AUDINATE_CLOCK_SOURCE_AES;
		}
		else if (!strncmp(str,"int",3))
		{
			param = CONMON_AUDINATE_CLOCK_SOURCE_INTERNAL;
		} 
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_clocking_control_set_source(body, param);
	}	
	else if(!strncmp(argv[3], "pref", 4) && strlen(argv[3]) > 4)
	{
		uint8_t pref;
		str = argv[3] + 5;
		if(!strncmp(str,"true",4)) 
		{
			pref = (uint8_t) AUD_TRUE;
		} 
		else if (!strncmp(str,"false",5))
		{
			pref = (uint8_t) AUD_FALSE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_clocking_control_set_preferred(body, pref);
	}
	else if(!strncmp(argv[3], "udelay", 6) && strlen(argv[3]) > 6)
	{
		aud_bool_t udelay;
		str = argv[3] + 7;
		if(!strncmp(str,"true",4)) 
		{
			udelay = AUD_TRUE;
		} 
		else if (!strncmp(str,"false",5))
		{
			udelay = AUD_FALSE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_clocking_control_set_unicast_delay_requests(body, udelay);
	}
	else if(!strncmp(argv[3], "subdomain", 9) && strlen(argv[3]) > 9)
	{
		const char * subdomain_name = NULL;
		str = argv[3] + 10;
		if (str[0])
		{
			//subdomain = (conmon_audinate_clock_subdomain_t) atoi(str);
			subdomain_name = str;
		}
		conmon_audinate_clocking_control_set_subdomain_name(body, subdomain_name);

	}
	else if(!strncmp(argv[3], "enabled", 7) && strlen(argv[3]) > 7)
	{
		aud_bool_t enabled;
		str = argv[3] + 8;
		if(!strncmp(str,"true",4)) 
		{
			enabled = AUD_TRUE;
		} 
		else if (!strncmp(str,"false",5))
		{
			enabled = AUD_FALSE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_clocking_control_set_multicast_ports_enabled(body, enabled);
	}
	else if(!strncmp(argv[3], "slave_only", 10) && strlen(argv[3]) > 10)
    {
        aud_bool_t enabled;
        str = argv[3] + 11;
        if(!strncmp(str,"true",4))
        {
                enabled = AUD_TRUE;
        }
        else if (!strncmp(str,"false",5))
        {
                enabled = AUD_FALSE;
        }
        else
        {
                return AUD_ERR_INVALIDPARAMETER;
        }
        conmon_audinate_clocking_control_set_slave_only_enabled(body, enabled);
    }
	else if (!strncmp(argv[3], "layer3", 6) && strlen(argv[3]) == 6)
	{
		aud_bool_t enabled;
		if (!strncmp(argv[4], "enabled", 7) && strlen(argv[4]) > 7)
		{
			str = argv[4] + 8;
			if (!strncmp(str, "true", 4))
			{
				enabled = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				enabled = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_clocking_control_set_transport_layer_ports_enabled(body, CONMON_AUDINATE_TRANSPORT_LAYER_3, enabled);
	}
	else if (!strncmp(argv[3], "version1", 8) && strlen(argv[3]) == 8)
	{
		if (!strncmp(argv[4], "enabled", 7) && strlen(argv[4]) > 7)
		{
			aud_bool_t enabled;
			str = argv[4] + 8;
			if (!strncmp(str, "true", 4))
			{
				enabled = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				enabled = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
			conmon_audinate_clocking_control_set_version_specific_ports_enabled(body, CONMON_AUDINATE_PORT_VERSION_V1, enabled);
		}
		else if (!strncmp(argv[4], "udelay", 6) && strlen(argv[4]) > 6)
		{
			aud_bool_t udelay;
			str = argv[4] + 7;
			if (!strncmp(str, "true", 4))
			{
				udelay = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				udelay = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
			conmon_audinate_clocking_control_set_version_specific_unicast_delay_requests(body, CONMON_AUDINATE_PORT_VERSION_V1, udelay);
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
	}
	else if (!strncmp(argv[3], "version2", 8) && strlen(argv[3]) == 8)
	{
		if (!strncmp(argv[4], "enabled", 7) && strlen(argv[4]) > 7)
		{
			aud_bool_t enabled;
			str = argv[4] + 8;
			if (!strncmp(str, "true", 4))
			{
				enabled = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				enabled = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
			conmon_audinate_clocking_control_set_version_specific_ports_enabled(body, CONMON_AUDINATE_PORT_VERSION_V2, enabled);
		}
		else if (!strncmp(argv[4], "udelay", 6) && strlen(argv[4]) > 6)
		{
			aud_bool_t udelay;
			str = argv[4] + 7;
			if (!strncmp(str, "true", 4))
			{
				udelay = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				udelay = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
			conmon_audinate_clocking_control_set_version_specific_unicast_delay_requests(body, CONMON_AUDINATE_PORT_VERSION_V2, udelay);
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
	}
	else if (!strncmp(argv[3], "port_id", 7) && strlen(argv[3]) > 7)
	{
		uint16_t port_id = (uint16_t)atoi(argv[3] + 8);
		if (port_id < 1 || port_id > 4)	//currently max of 4 ports
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		if (!strncmp(argv[4], "enabled", 7) && strlen(argv[4]) > 7)
		{
			aud_bool_t enabled;
			str = argv[4] + 8;
			if (!strncmp(str, "true", 4))
			{
				enabled = AUD_TRUE;
			}
			else if (!strncmp(str, "false", 5))
			{
				enabled = AUD_FALSE;
			}
			else
			{
				return AUD_ERR_INVALIDPARAMETER;
			}
			conmon_audinate_clocking_control_set_port_id_enabled(body, port_id, enabled);
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
	}
	// TODO set sync mode ?
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//*body_size = sizeof(conmon_audinate_clocking_control_t);
	*body_size = conmon_audinate_clocking_control_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t conmon_audinate_unicast_clocking_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	conmon_audinate_init_unicast_clocking_control(body, 0);

	// parse args
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else if(!strcmp(argv[3], "enabled=true"))
	{
		conmon_audinate_unicast_clocking_control_set_enabled(body, AUD_TRUE);
	}
	else if(!strcmp(argv[3], "enabled=false"))
	{
		conmon_audinate_unicast_clocking_control_set_enabled(body, AUD_FALSE);
	}
	else if(!strcmp(argv[3], "reload"))
	{
		conmon_audinate_unicast_clocking_control_set_reload_devices(body, AUD_TRUE);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	*body_size = conmon_audinate_unicast_clocking_control_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t conmon_audinate_name_id_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	uint16_t d, nd;
	if (argc == 3)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	nd = (uint16_t) (argc-3);
	conmon_audinate_init_name_id_control(body, 0, nd);
	for (d = 0; d < nd; d++)
	{
		const char * str = argv[d+3];
		int i, x[8], p;
		if (!strncmp(str, "name=", 5) && strlen(str) > 5)
		{
			conmon_audinate_name_id_set_device_name_id_at_index(body, d, str+5, NULL, NULL, NULL);
		}
		else if (!strncmp(str, "id=", 3) && sscanf(str+3, "%02x%02x%02x%02x%02x%02x%02x%02x:%04x",
			x+0, x+1, x+2, x+3, x+4, x+5, x+6, x+7, &p) == 9)
		{
			conmon_instance_id_t instance_id;
			for (i = 0; i < CONMON_DEVICE_ID_LENGTH; i++)
			{
				instance_id.device_id.data[i] = (uint8_t) x[i];
			}
			instance_id.process_id = (uint16_t) p;
			conmon_audinate_name_id_set_device_name_id_at_index(body, d, NULL, NULL, &instance_id, NULL);
		}
		else if (!strncmp(str, "uuid=", 5) && sscanf(str+5, "%02x:%02x:%02x:%02x:%02x:%02x", 
			x+0, x+1, x+2, x+3, x+4, x+5) == 6)
		{
			conmon_audinate_clock_uuid_t uuid;
			for (i = 0; i < CONMON_AUDINATE_CLOCK_UUID_LENGTH; i++)
			{
				uuid.data[i] = (uint8_t) x[i];
			}
			conmon_audinate_name_id_set_device_name_id_at_index(body, d, NULL, NULL, NULL, &uuid);
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
	}
	*body_size = conmon_audinate_name_id_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t conmon_audinate_igmp_vers_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	uint16_t version;
	//conmon_audinate_igmp_version_control_t * igmp_control = (conmon_audinate_igmp_version_control_t *)body;
	conmon_audinate_init_igmp_version_control(body, CONGESTION_DELAY);
	// parse args
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else if(!strncmp(argv[3], "vers", 4) && strlen(argv[3]) > 4)
	{
		version = (uint16_t) atoi(argv[3] + 5);
		if(version < 1 || version > 3) 
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		else
		{
			conmon_audinate_igmp_version_control_set_version(body, version);
		}
	}	
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	//*body_size = sizeof(conmon_audinate_igmp_version_control_t);
	*body_size = conmon_audinate_igmp_version_control_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t
conmon_audinate_ifstats_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	conmon_audinate_init_ifstats_control (body, 0);
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else
	{
		char * arg = argv [3];
		if (strcmp (arg, "clear") == 0)
		{
			conmon_audinate_ifstats_control_set_clear_errors(body);
		}
		else
		{
			goto l__usage;
		}
	}

	*body_size = (uint16_t) conmon_audinate_ifstats_control_get_size(body);
	return AUD_SUCCESS;

l__usage:
	return AUD_ERR_INVALIDPARAMETER;
}


#define CHECK_SRATE(x) ( x == CONMON_AUDINATE_SRATE_44K || x == CONMON_AUDINATE_SRATE_48K  || x == CONMON_AUDINATE_SRATE_96K || x == CONMON_AUDINATE_SRATE_192K ||  x == CONMON_AUDINATE_SRATE_88K ||  x == CONMON_AUDINATE_SRATE_176K) 	 

aud_error_t
conmon_audinate_srate_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	conmon_audinate_init_srate_control (body, 0);
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message

		*body_size = conmon_audinate_srate_control_get_size(body);
		return AUD_SUCCESS;
	}
	else
	{
		char * arg = argv [3];

		char * value = strchr (arg, '=');
		if (! value)
		{
			goto l__usage;
		}

		* value = 0;
		value ++;
		if (! * value)
		{
			goto l__usage;
		}
		
		if (strcmp (arg, "rate") == 0)
		{
			uint32_t rate = atol (value);

			if (CHECK_SRATE(rate))
			{
				conmon_audinate_srate_control_set_rate(body, rate);
				*body_size = conmon_audinate_srate_control_get_size(body);
				return AUD_SUCCESS;
			}
			else
			{
				goto l__usage;
			}
		}
		else
		{
			goto l__usage;
		}
	}

l__usage:
	return AUD_ERR_INVALIDPARAMETER;
}


aud_error_t
conmon_audinate_enc_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	conmon_audinate_init_enc_control (body, 0);
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message

		*body_size = conmon_audinate_enc_control_get_size(body);
		return AUD_SUCCESS;
	}
	else
	{
		char * arg = argv [3];

		char * value = strchr (arg, '=');
		if (! value)
		{
			goto l__usage;
		}

		* value = 0;
		value ++;
		if (! * value)
		{
			goto l__usage;
		}
		
		if (strcmp (arg, "enc") == 0)
		{
			uint16_t enc = (uint16_t) atoi (value);
			if (! enc)
			{
				if (strcmp(value, "pcm16") == 0)
				{
					enc = 16;
				}
				else if (strcmp(value, "pcm24") == 0)
				{
					enc = 24;
				}
				else if (strcmp(value, "pcm32") == 0)
				{
					enc = 32;
				}
				else if (strcmp (value, "raw32") == 0)
				{
					enc = 0x1300;
				}
				else
				{
					goto l__usage;
				}
			}

			conmon_audinate_enc_control_set_encoding (body, enc);
			*body_size = conmon_audinate_enc_control_get_size (body);
			return AUD_SUCCESS;
		}
	}

l__usage:
	return AUD_ERR_INVALIDPARAMETER;
}

#define PULLUP_STRING_NONE "0"
#define PULLUP_STRING_PLUS4 "+4.1667"
#define PULLUP_STRING_PLUS01 "+0.1"
#define PULLUP_STRING_MINUS01 "-0.1"
#define PULLUP_STRING_MINUS4 "-4.0"

aud_error_t
conmon_audinate_srate_pullup_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	uint32_t value;
	char * str;

	conmon_audinate_init_srate_pullup_control (body, 0);

	// parse args
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
		*body_size = conmon_audinate_srate_pullup_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 5)
	{
		if(!strncmp(argv[3], "val", 3) && strlen(argv[3]) > 3)
		{
			str = argv[3] + 4;
			if(!strcmp(str,PULLUP_STRING_NONE))
			{
				value = CONMON_AUDINATE_SRATE_PULLUP_NONE;
			} 
			else if (!strcmp(str,PULLUP_STRING_PLUS4))
			{
				value =  CONMON_AUDINATE_SRATE_PULLUP_PLUSFOURPOINTONESIXSIXSEVEN;
			}
			else if (!strcmp(str,PULLUP_STRING_PLUS01))
			{
				value =  CONMON_AUDINATE_SRATE_PULLUP_PLUSPOINTONE;
			} 
			else if (!strcmp(str,PULLUP_STRING_MINUS01))
			{
				value =  CONMON_AUDINATE_SRATE_PULLUP_MINUSPOINTONE;
			} 
			else if (!strcmp(str,PULLUP_STRING_MINUS4))
			{
				value =  CONMON_AUDINATE_SRATE_PULLUP_MINUSFOUR;
			} 
			else
			{
				goto l__usage;
			}
			conmon_audinate_srate_pullup_control_set_rate(body, value);	
		}
		if(!strncmp(argv[4], "subdomain", 9) && strlen(argv[4]) > 9)
		{
			const char * subdomain_name = NULL;
			str = argv[4] + 10;
			if (str[0])
			{
				subdomain_name = str;
			}
			conmon_audinate_srate_pullup_control_set_subdomain(body, subdomain_name);
		}	
		*body_size = conmon_audinate_srate_pullup_control_get_size(body);
		return AUD_SUCCESS;
	}
l__usage:
		return AUD_ERR_INVALIDPARAMETER;
}


aud_error_t conmon_audinate_edk_board_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	//conmon_audinate_edk_board_control_t * edk_control = (conmon_audinate_edk_board_control_t *)body;
	uint16_t param;
	char * str;

	conmon_audinate_init_edk_board_control(body, CONGESTION_DELAY);
	// parse args
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else if(!strncmp(argv[3], "rev", 3) && strlen(argv[3]) > 3)
	{
		str = argv[3] + 4;
		if(!strncmp(str,"other",5)) 
		{
			param = CONMON_AUDINATE_EDK_BOARD_REV_OTHER;
		} 
		else if (!strncmp(str,"red",3))
		{
			param = CONMON_AUDINATE_EDK_BOARD_REV_RED;
		}
		else if (!strncmp(str,"green",5))
		{
			param = CONMON_AUDINATE_EDK_BOARD_REV_GREEN;
		} 
		else
		{
			fprintf(stderr,"%s controlled_device edk rev=other|red|green\n", argv[0]);
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_edk_board_set_rev(body, param);
	}	
	else if(!strncmp(argv[3], "pad", 3) && strlen(argv[3]) > 3)
	{
		int db;
		str = argv[3] + 4;
		db=atoi(str);
		if(db == 0)
		{
			param = CONMON_AUDINATE_EDK_BOARD_PAD_0DB;
		}
		else if (db == -6)
		{
			param = CONMON_AUDINATE_EDK_BOARD_PAD_MINUS6DB;
		}
		else if (db == -12)
		{
			param = CONMON_AUDINATE_EDK_BOARD_PAD_MINUS12DB;
		}
		else if (db == -24)
		{
			param = CONMON_AUDINATE_EDK_BOARD_PAD_MINUS24DB;
		}
		else if (db == -48)
		{
			param = CONMON_AUDINATE_EDK_BOARD_PAD_MINUS48DB;
		}
		else
		{
			fprintf(stderr,"%s controlled_device edk pad=0|-6|-12|-24|-48\n", argv[0]);
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_edk_board_set_pad(body, param);
	}	
	else if(!strncmp(argv[3], "dig", 3) && strlen(argv[3]) > 3)
	{
		str = argv[3] + 4;
		if(!strncmp(str,"spdif",5)) 
		{
			param = CONMON_AUDINATE_EDK_BOARD_DIG_SPDIF;
		}
		else if(!strncmp(str,"aes",3))
		{
			param = CONMON_AUDINATE_EDK_BOARD_DIG_AES;
		}
		else if(!strncmp(str,"tos",3))
		{
			param = CONMON_AUDINATE_EDK_BOARD_DIG_TOSLINK;
		}
		else
		{
			fprintf(stderr,"%s controlled_device edk dig=spdif|aes|tos\n", argv[0]);
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_edk_board_set_dig(body, param);
	}
	else if(!strncmp(argv[3], "src", 3) && strlen(argv[3]) > 3)
	{
		str = argv[3] + 4;
		if(!strncmp(str,"sync",4)) 
		{
			param = CONMON_AUDINATE_EDK_BOARD_SRC_SYNC;
		}
		else if(!strncmp(str,"async",5)) 
		{
			param = CONMON_AUDINATE_EDK_BOARD_SRC_ASYNC;
		}
		else
		{
			fprintf(stderr,"%s controlled_device edk src=sync|async\n", argv[0]);
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_edk_board_set_src(body, param);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}

	//*body_size = sizeof(conmon_audinate_edk_board_control_t);
	*body_size = conmon_audinate_edk_board_control_get_size(body);
	return AUD_SUCCESS;
}

aud_error_t conmon_audinate_sys_reset_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	//conmon_audinate_sys_reset_control_t * rst_control = (conmon_audinate_sys_reset_control_t *)body;
	uint16_t param;
	char * str;
	// parse args
	conmon_audinate_init_sys_reset_control(body, CONGESTION_DELAY);

	if(argc == 4 && argv[3] && !strncmp(argv[3], "mode", 4) && strlen(argv[3]) > 4)
	{
		str = argv[3] + 5;
		if(!strncmp(str,"soft",4)) 
		{
			param = CONMON_AUDINATE_SYS_RESET_SOFT;
		}
		else if(!strncmp(str,"factory",7)) 
		{
			param = CONMON_AUDINATE_SYS_RESET_FACTORY;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_sys_reset_control_set_mode(body, param);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	//*body_size = sizeof(conmon_audinate_sys_reset_control_t);
	*body_size = conmon_audinate_sys_reset_control_get_size(body);
	return AUD_SUCCESS;
}

aud_error_t conmon_audinate_access_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	uint16_t param;
	char * str;
	// parse args
	conmon_audinate_init_access_control(body, CONGESTION_DELAY);

	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
	}
	else if(!strncmp(argv[3], "mode", 4) && strlen(argv[3]) > 4)
	{
		str = argv[3] + 5;
		if(!strncmp(str,"enable",6)) 
		{
			param = CONMON_AUDINATE_ACCESS_ENABLE;
		}
		else if(!strncmp(str,"disable",7)) 
		{
			param = CONMON_AUDINATE_ACCESS_DISABLE;
		}
		else if(!strncmp(str,"inetd_enable",7))
		{
			param = CONMON_AUDINATE_INETD_ENABLE;
		}
		else if(!strncmp(str,"inetd_disable",7)) 
		{
			param = CONMON_AUDINATE_INETD_DISABLE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_access_control_set_mode(body, param);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	//*body_size = sizeof(conmon_audinate_access_control_t);
	*body_size = conmon_audinate_access_control_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t conmon_audinate_rx_error_threshold_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	uint16_t win;
	uint16_t thres;
	uint16_t reset;
	//char * str;
	// parse args
	conmon_audinate_init_rx_error_threshold_control(body, CONGESTION_DELAY);
	if(argc == 3)
	{
		printf("sending query message\n");
		// no params send null message
		goto send_msg;
	}
	else if(!strncmp(argv[3], "thres", 5) && strlen(argv[3]) > 5)
	{
		thres = (uint16_t) atoi(argv[3] + 6);
		conmon_audinate_rx_error_threshold_set_threshold(body, thres);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	if(argv[4] && !strncmp(argv[4], "win", 3) && strlen(argv[4]) > 3)
	{
		win = (uint16_t) atoi(argv[4] + 4);
		conmon_audinate_rx_error_threshold_set_window(body, win);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	if(argv[5] && !strncmp(argv[5], "reset", 5) && strlen(argv[5]) > 5)
	{
		reset = (uint16_t) atoi(argv[5] + 6);
		conmon_audinate_rx_error_threshold_set_reset_time(body, reset);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	send_msg:
	*body_size = conmon_audinate_rx_error_threshold_control_get_size(body);
	return AUD_SUCCESS;
}


aud_error_t conmon_audinate_metering_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	conmon_audinate_init_metering_control(body, CONGESTION_DELAY);
	if(argc == 3)
	{
		*body_size = conmon_audinate_metering_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 4)
	{
		uint32_t rate;

		if (!strncmp(argv[3], "rate=", 5) && strlen(argv[3]) > 5)
		{
			rate = (uint16_t) atoi(argv[3]+5);
			if (!(rate == 10 || rate == 30))
			{
				goto __error;
			}
		}
		else
		{
			goto __error;
		}
		
		conmon_audinate_metering_control_set_update_rate(body, rate);
		*body_size = conmon_audinate_metering_control_get_size(body);
		return AUD_SUCCESS;
	}

__error:
	return AUD_ERR_INVALIDPARAMETER;
}


aud_error_t conmon_audinate_serial_port_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	conmon_audinate_init_serial_port_control(body, CONGESTION_DELAY);
	if(argc == 3)
	{
		printf("sending query message\n");
		*body_size = conmon_audinate_serial_port_control_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 8)
	{
		uint16_t index;
		conmon_audinate_serial_port_baud_rate_t baud_rate;
		conmon_audinate_serial_port_bits_t bits;
		conmon_audinate_serial_port_parity_t parity;
		conmon_audinate_serial_port_stop_bits_t stop_bits;

		if (!strncmp(argv[3], "index=", 6) && strlen(argv[3]) > 6)
		{
			index = (uint16_t) atoi(argv[3]+6);
		}
		else
		{
			goto __error;
		}
		if (!strncmp(argv[4], "speed=", 6) && strlen(argv[4]) > 6)
		{
			baud_rate = (conmon_audinate_serial_port_baud_rate_t) atoi(argv[4]+6);
		}
		else
		{
			goto __error;
		}
		if (!strncmp(argv[5], "bits=", 5) && strlen(argv[5]) > 5)
		{
			bits = (conmon_audinate_serial_port_bits_t) atoi(argv[5]+5);
		}
		else
		{
			goto __error;
		}
		if (!strcmp(argv[6], "parity=none"))
		{
			parity = CONMON_AUDINATE_SERIAL_PORT_PARITY_NONE;
		}
		else if (!strcmp(argv[6], "parity=odd"))
		{
			parity = CONMON_AUDINATE_SERIAL_PORT_PARITY_ODD;
		}
		else if (!strcmp(argv[6], "parity=even"))
		{
			parity = CONMON_AUDINATE_SERIAL_PORT_PARITY_EVEN;
		}
		else
		{
			goto __error;
		}
		if (!strncmp(argv[7], "stop=", 5) && strlen(argv[7]) > 5)
		{
			stop_bits = (conmon_audinate_serial_port_stop_bits_t) atoi(argv[7]+5);
		}
		else
		{
			goto __error;
		}
		
		conmon_audinate_serial_port_control_set_port(body, index,
			baud_rate, bits, parity, stop_bits);
		*body_size = conmon_audinate_serial_port_control_size(body);
		return AUD_SUCCESS;
	}

__error:
	return AUD_ERR_INVALIDPARAMETER;
}

aud_error_t conmon_audinate_haremote_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	conmon_audinate_init_haremote_control(body, CONGESTION_DELAY);
	if(argc == 3)
	{
		printf("sending query message\n");
		*body_size = conmon_audinate_haremote_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (argc == 4)
	{
		conmon_audinate_haremote_bridge_mode_t mode;
		if (!strcmp(argv[3], "mode=all"))
		{
			mode = CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_ALL;
		}
		else if (!strcmp(argv[3], "mode=none"))
		{
			mode = CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NONE;
		}
		else if (!strcmp(argv[3], "mode=slot_db9"))
		{
			mode = CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_SLOT_DB9;
		}
		else if (!strcmp(argv[3], "mode=network_slot"))
		{
			mode = CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_SLOT;
		}
		else if (!strcmp(argv[3], "mode=network_db9"))
		{
			mode = CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_DB9;
		}
		else
		{
			goto __error;
		}
		conmon_audinate_haremote_control_set_bridge_mode(body, mode);
		*body_size = conmon_audinate_haremote_control_get_size(body);
		return AUD_SUCCESS;
	}

__error:
	return AUD_ERR_INVALIDPARAMETER;
}

aud_error_t conmon_audinate_ptp_logging_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
) {
	char * str;

	conmon_audinate_init_ptp_logging_control(body, CONGESTION_DELAY);
	if(argc == 3)
	{
		printf("sending query message\n");
		*body_size = conmon_audinate_ptp_logging_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if(!strncmp(argv[3], "enabled", 7) && strlen(argv[3]) > 7)
	{
		aud_bool_t enabled;
		str = argv[3] + 8;
		if(!strncmp(str,"true",4)) 
		{
			enabled = AUD_TRUE;
		} 
		else if (!strncmp(str,"false",5))
		{
			enabled = AUD_FALSE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_ptp_logging_network_set_enabled(body,enabled);
		*body_size = conmon_audinate_ptp_logging_control_get_size(body);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	return AUD_SUCCESS;
}

aud_error_t conmon_audinate_aes67_control_handler
(
	int argc,
	char ** argv,
	conmon_message_body_t *body,
	uint16_t *body_size
)
{
	char * str;

	conmon_audinate_init_aes67_control(body, CONGESTION_DELAY);
	if (argc == 3)
	{
		printf("sending query message\n");
		*body_size = conmon_audinate_aes67_control_get_size(body);
		return AUD_SUCCESS;
	}
	else if (!strncmp(argv[3], "enabled", 7) && strlen(argv[3]) > 7)
	{
		aud_bool_t enabled;
		str = argv[3] + 8;
		if (!strncmp(str, "true", 4))
		{
			enabled = AUD_TRUE;
		}
		else if (!strncmp(str, "false", 5))
		{
			enabled = AUD_FALSE;
		}
		else
		{
			return AUD_ERR_INVALIDPARAMETER;
		}
		conmon_audinate_aes67_control_set_enable(body, enabled);
		*body_size = conmon_audinate_aes67_control_get_size(body);
	}
	else
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	return AUD_SUCCESS;
}

/******************************************************
 **********  Conmon controller usage functions  *******
 ******************************************************/
void conmon_audinate_interface_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device interface redundancy BOOL - enable/disable redundancy on this device\n", cmd);
	fprintf(stderr, "%s controlled_device interface index dynamic - enable DHCP or Link local addressing on this interface\n", cmd);
	fprintf(stderr, "%s controlled_device interface index ll BOOL - enable/disable Link local addressing only on this interface\n", cmd);
	fprintf(stderr, "%s controlled_device interface index ll_delay BOOL - enable/disable delayed Link local addressing on this interface\n",cmd);
	fprintf(stderr,"\t(Delay switching to LL if DHCP server is slow to respond)\n");
	fprintf(stderr, "%s controlled_device interface index static address netmask [dns_server] [gateway] [domain] - configure a static ip address on this interface\n", cmd);
	fprintf(stderr, "%s controlled_device interface index dhcp BOOL - enable/disable DHCP addressing only on this interface\n", cmd);
	fprintf(stderr,"\t(Warning! - must ensure a DHCP server is available)\n");
}

void conmon_audinate_switch_vlan_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device switch_vlan [id=ID] - switch device to use a specific vlan configuration\n", cmd);
}

void conmon_audinate_clocking_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device clocking src=int|bnc - set device clock source to internal or external (bnc)\n", cmd);
	fprintf(stderr,"%s controlled_device clocking pref=true|false - set/clear this device as preferred PTP master\n", cmd);
	fprintf(stderr,"%s controlled_device clocking subdomain=subdomain_name - configure the PTP subdomain\n", cmd);
	fprintf(stderr,"%s controlled_device clocking enabled=true|false - enable/disable  multicast PTP\n", cmd);
	fprintf(stderr,"%s controlled_device clocking slave_only=true|false - enable/disable PTP slave only mode\n", cmd);
	fprintf(stderr,"%s controlled_device clocking layer3 enabled=true|false - enabling/disabling ip layer ports\n", cmd);
	fprintf(stderr,"%s controlled_device clocking version1 enabled=true|false - enable/disable PTP version 1 ports\n", cmd);
	fprintf(stderr,"%s controlled_device clocking version2 enabled=true|false - enable/disable PTP version 2 ports\n", cmd);
	fprintf(stderr, "%s controlled_device clocking port_id=val enabled=true|false - enable/disable a specific PTP port\n", cmd);
	fprintf(stderr,"%s controlled_device clocking version1 udelay=true|false - enable/disable unicast delay requests for PTP version 1 ports\n", cmd);
	fprintf(stderr,"%s controlled_device clocking version2 udelay=true|false - enable/disable unicast delay requests for PTP version 2 ports\n", cmd);
}

void conmon_audinate_unicast_clocking_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device uclocking enabled=true|false - enable/disable unicast clocking\n", cmd);
	fprintf(stderr,"%s controlled_device uclocking reload - reload unicast peer list from file\n", cmd);
}

void conmon_audinate_name_id_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s broadcast name_id [name=NAME | id=DEVICE_ID:PROCESS_ID | uuid=UUID]* - find a device's name,id,uuid information given one of these parameters\n", cmd);
	fprintf(stderr,"\t(Warning! - uses the broadcast channel)\n");
}

void conmon_audinate_master_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s broadcast master - query for all devices that have a master PTP port\n",cmd);
	fprintf(stderr,"\t(Warning! - uses the broadcast channel)\n");
}

void conmon_audinate_igmp_vers_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device igmp vers=1|2|3 - query/configure the IGMP version on a device\n",cmd);
	fprintf(stderr,"\t(Note: Ultimo only implements v2, other embedded devices are able to automatically adapt to the IGMP version used by the querier)\n");
}

void conmon_audinate_vers_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device versions - query device software/firmware/uboot/dapi versions and capabilities\n",cmd);
}

void conmon_audinate_srate_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device srate rate=44100|48000|88200|96000|176400|192000 - query/set sample rate\n", cmd);
}

void conmon_audinate_srate_pullup_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device pullup val="
			PULLUP_STRING_NONE "|" PULLUP_STRING_PLUS4 "|"
			PULLUP_STRING_PLUS01 "|" PULLUP_STRING_MINUS01 "|" PULLUP_STRING_MINUS4 " pullup value and associated subdomain\n"
			, cmd
	);
	fprintf(stderr,"\tNote: pullup:subdomain association is %s%%:_DFLT %s%%:_ALT1 %s%%:_ALT2 %s%%:_ALT3 %s%%:_ALT4\n", PULLUP_STRING_NONE, PULLUP_STRING_PLUS4, PULLUP_STRING_PLUS01,  
	PULLUP_STRING_MINUS01, PULLUP_STRING_MINUS4);
}

void conmon_audinate_enc_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device enc enc=pcm16|pcm24|pcm32|raw32 - set the device preferred encoding\n", cmd);
	fprintf(stderr,"\tNote: devices can translate between pcm bitdepths if so configured via module configuration\n");
}

void conmon_audinate_edk_board_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device edk rev=other|red|green - configure BKII pdk board version\n", cmd);
	fprintf(stderr,"%s controlled_device edk pad=0|-6|-12|-24|-48 - configure BKII pdk board pad DB\n", cmd);
	fprintf(stderr,"%s controlled_device edk dig=spdif|aes|tos - configure BKII pdk board digital input\n", cmd);
	fprintf(stderr,"%s controlled_device edk src=sync|async	- configure pdk sample rate convertor\n", cmd);
}

void conmon_audinate_access_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s localhost access mode=enable|disable - enable/disable all externally accessible  control protocols\n", cmd);
	fprintf(stderr,"%s localhost access mode=inetd_enable|inetd_disable - enable/disable telnetd and ftpd\n", cmd);
	fprintf(stderr,"\tNote: sent on local channel\n");
}

void conmon_audinate_manf_vers_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device manf_versions - query manufacturer software/firmware versions and capabilities\n",cmd);
}

void conmon_audinate_identify_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device identify - trigger identify on a device\n",cmd);
	fprintf(stderr,"\tNote: identify functionality must be implemented by OEM on a Brooklyn II\n");
}

void conmon_audinate_ifstats_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device ifstats - query device interface statistics\n", cmd);
	fprintf(stderr,"%s controlled_device ifstats clear - clear device interface error counters\n", cmd);
}

void conmon_audinate_rx_error_threshold_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device errthres thres=val win=val reset=val - query or set audio rx error threshold values\n", cmd);
	fprintf(stderr,"\t(Note: not implemented on all devices)\n");
}

void conmon_audinate_rxerrq_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s rxerrq - query for receive audio errors\n", cmd);
	fprintf(stderr,"\t(Note: not implemented on all devices)\n");
}

void conmon_audinate_sys_reset_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device sysreset mode=soft|factory - soft reset or factory (clear all config) reset device: device is rebooted\n", cmd);
}

void conmon_audinate_metering_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device metering [rate=10|30] - query or set metering rate to 10Hz/30Hz\n", cmd);
}

void conmon_audinate_serial_port_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device serial index=val speed=val bits=7|8 parity=none|odd|even stop=0|1|2 - query/configure a uart\n", cmd);
}

void conmon_audinate_haremote_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device haremote mode=all|none|slot_db9|network_slot|network_db9 - query/configure the haremote transaction mode\n", cmd);
}

void conmon_audinate_haremote_stats_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device haremote_stats - query remote statistics\n", cmd);
}

void conmon_audinate_upgrade_v3_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device upgrade3 query - return the status of the last upgrade\n", cmd);
	fprintf(stderr, "%s controlled_device upgrade3 tftp=server_ip[:port]|local path=path [len=len] [mf=manufacturer_id] [model=model_id][addip=additional_target_ip][addport=additional_target_port]\n", cmd);
	fprintf(stderr, "\t Upgrade device via tftp\n");
	fprintf(stderr, "%s controlled_device upgrade3 xmodem=port len=len baud=baud {parity=1|2|3 flwctrl=1|2} | {mstrpol=1|2 mstrpha=1|2 slvpol=1|2 slvpha=1|2} [mf=manufacturer_id] [model=model_id][addip=additional_target_ip][addport=additional_target_port]\n", cmd);
	fprintf(stderr, "\t Upgrade device via xmodem - Ultimo only\n");
	fprintf(stderr, "XModem Parameters:\n");
	fprintf(stderr, "port: uarta, uartb or spi0\n");
	fprintf(stderr, "baud: when port=spi0 baud referrs to the master clock rate\n");
	fprintf(stderr, "parity=1: UART parity none, parity=2: UART parity even, parity=3: UART parity odd\n");
	fprintf(stderr, "flwctrl=1: UART flow control none, flwctrl=2: UART flow control RTS/CTS\n");
	fprintf(stderr, "mstrpol=1: SPI master polarity idle low, mstrpol=2: SPI master polarity idle high\n");
	fprintf(stderr, "mstrpha=1: SPI master phase sample leading edge, mstrpol=2: SPI master phase sample trailing edge\n");
	fprintf(stderr, "slvpol=1: SPI slave polarity idle low, mstrpol=2: SPI slave polarity idle high\n");
	fprintf(stderr, "slvpha=1: SPI slave phase sample leading edge, mstrpol=2: SPI slave phase sample trailing edge\n");
	fprintf(stderr, "Xmodem configuration examples:\n");
	fprintf(stderr, "%s controlled_device upgrade3 xmodem=uartb len=4500 baud=115200 parity=1 flwctrl=2\n", cmd);
	fprintf(stderr, "%s controlled_device upgrade3 xmodem=spi0 len=4500 baud=3125000 mstrpol=1 mstrpha=2 slvpol=1 slvpha=2\n", cmd);
}

void conmon_audinate_clear_config_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device clear_config [keep_ip|all|query] - clear device config: clear all except ip address information, clear all, query\n", cmd);
}

void conmon_audinate_gpio_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device gpio [query] - query current gpio input value\n", cmd); 
	fprintf(stderr, "%s controlled_device gpio [output] - mask=0xXX val=0xXX - configure gpio output\n", cmd); 
	fprintf(stderr, "\tNote: Ultimo only\n");
}

void conmon_audinate_ptp_logging_control_usage
(
	char *cmd
)
{
	fprintf(stderr,"%s controlled_device ptp_logging enabled=true|false - enable/disable network ptp logging\n", cmd);
	fprintf(stderr,"\tNote: logging address is 239.254.3.3\n");
}

void conmon_audinate_aes67_control_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device aes67 enabled=true|false - enable/disable aes67 functionality\n", cmd);
}

void conmon_audinate_audio_interface_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device audio_interface - query audio_interface configuration (chans_per_line, framing, alignment, channel mapping)\n", cmd);
}

void conmon_audinate_dante_ready_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device dante_ready - query dante ready status, indicates all dante apps are ready to accept commands \n", cmd);
	fprintf(stderr, "\tNote: sent on the local channel\n");
}

void conmon_audinate_routing_ready_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device routing_ready - query routing ready status, indicates routing component is ready to accept commands\n", cmd);
}

void conmon_audinate_lock_status_usage
(
	char *cmd
)
{
	fprintf(stderr, "%s controlled_device lock - query device lock/unlock status\n", cmd);
}


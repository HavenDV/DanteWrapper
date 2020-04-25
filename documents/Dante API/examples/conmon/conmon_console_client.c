#include "dapi_utils_domains.h"
#include "conmon_examples.h"
#include "conmon_aud_print_msg.h"

#include <assert.h>
#include <stdio.h>

#ifdef WIN32
#include "conio.h"
#else
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#endif


aud_bool_t g_running = AUD_TRUE;
conmon_client_request_id_t g_req_id = CONMON_CLIENT_NULL_REQ_ID;
aud_errbuf_t g_errbuf;

dapi_t * g_dapi = NULL;
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
dapi_utils_ddm_config_t g_ddm_config = {0};
#endif

// Vendor id for the example clients, can be overwritten
conmon_vendor_id_t g_tx_vendor_id = {{'e', 'x', 'a', 'm', 'p', 'l', 'e', '\0'}};
conmon_vendor_id_t * g_rx_vendor_id = NULL;
aud_bool_t g_print_audinate_payloads = AUD_TRUE;
aud_bool_t g_print_payloads = AUD_FALSE;

typedef struct
{
	uint16_t id;
	const char * name;
} name_map_t;


const name_map_t CHANNEL_NAMES[] = 
{
	{CONMON_CHANNEL_TYPE_CONTROL,   "control"},
	{CONMON_CHANNEL_TYPE_METERING,  "metering"},
	{CONMON_CHANNEL_TYPE_STATUS,    "status"},
	{CONMON_CHANNEL_TYPE_BROADCAST, "broadcast"},
	{CONMON_CHANNEL_TYPE_LOCAL,     "local"},
	{CONMON_CHANNEL_TYPE_SERIAL,    "serial"},
	{CONMON_CHANNEL_TYPE_KEEPALIVE, "keepalive"},
	{CONMON_CHANNEL_TYPE_VENDOR_BROADCAST, "vbroadcast"},
	{CONMON_CHANNEL_TYPE_MONITORING, "monitoring"}
};

AUD_INLINE const char *
channel_type_to_string(conmon_channel_type_t type)
{
	return CHANNEL_NAMES[type].name;
}

AUD_INLINE conmon_channel_type_t
channel_type_from_string(const char * name)
{
	conmon_channel_type_t i; 
	for (i = 0; i < CONMON_NUM_CHANNELS; i++)
	{
		if (!STRCASECMP(name, CHANNEL_NAMES[i].name))
		{
			return CHANNEL_NAMES[i].id;
		}
	}
	return CONMON_CHANNEL_TYPE_NONE;
}

AUD_INLINE const char *
channel_direction_to_string(conmon_channel_direction_t dir)
{
	return dir == CONMON_CHANNEL_DIRECTION_TX ? "TX" : "RX";
}

static conmon_client_connection_state_changed_fn handle_connection_state_changed;
static conmon_client_handle_networks_changed_fn handle_networks_changed;
static conmon_client_handle_dante_device_name_changed_fn handle_dante_device_name_changed;
static conmon_client_handle_dns_domain_name_changed_fn handle_dns_domain_name_changed;
static conmon_client_handle_subscriptions_changed_fn handle_subscriptions_changed;

static conmon_client_handle_control_message_fn handle_control_message;
static conmon_client_handle_monitoring_message_fn handle_monitoring_message;

static conmon_client_response_fn 
	handle_connect_response,
	handle_response;

static void 
handle_message
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
) {
	conmon_instance_id_t instance_id;
	uint16_t body_size = conmon_message_head_get_body_size(head);
	const conmon_vendor_id_t * vid = conmon_message_head_get_vendor_id(head);
	char buf[BUFSIZ];

	AUD_UNUSED(body);

	if (g_rx_vendor_id && !conmon_vendor_id_equals(g_rx_vendor_id, vid))
	{
		return;
	}

	if (channel_type == CONMON_CHANNEL_TYPE_CONTROL)
	{
		printf("%s:\n", channel_type_to_string(channel_type));
	}
	else
	{
		printf("%s %s:\n", channel_direction_to_string(channel_direction), channel_type_to_string(channel_type));
	}

	conmon_message_head_get_instance_id(head, &instance_id);
	printf("    body_size=0x%04x seq=0x%04x class=0x%04x\n",
		conmon_message_head_get_body_size(head),
		conmon_message_head_get_seqnum(head), 
		conmon_message_head_get_message_class(head));

	printf("    source=%s (%s)\n",
		conmon_example_instance_id_to_string(&instance_id, buf, BUFSIZ),
		conmon_client_device_name_for_instance_id(client, &instance_id));
	printf("    vendor=%s\n", conmon_example_vendor_id_to_string(vid, buf, BUFSIZ));
	
	if (g_print_audinate_payloads)
	{
		if (conmon_vendor_id_equals(vid, CONMON_VENDOR_ID_AUDINATE))
		{
			conmon_aud_print_msg(body, body_size);
		}
	}
	if (g_print_payloads)
	{
		uint16_t i, len = body_size;
		for (i = 0; i < len; i++)
		{
			if (!body->data[i] || !isprint(body->data[i]))
			{
				break;
			}
		}
		if (i == len)
		{
			printf("    body=\"");
			for (i = 0; i < len; i++)
			{
				printf("%c", body->data[i]);
			}
			printf("\"\n");
		}
		else
		{
			printf("    body=0x");
			for (i = 0; i < len; i++)
			{
				printf("%02x", body->data[i]);
			}
			printf("\n");
		}
	}
}

static void
print_subscription
(
	const conmon_client_subscription_t * sub
) {
	unsigned int n;
	char buf[64];
	uint16_t active = conmon_client_subscription_get_connections_active(sub);
	uint16_t available = conmon_client_subscription_get_connections_available(sub);
	printf("%s@%s: status=%s id=%s connections=",
		channel_type_to_string(conmon_client_subscription_get_channel_type(sub)),
		conmon_client_subscription_get_device_name(sub),
		conmon_example_rxstatus_to_string(conmon_client_subscription_get_rxstatus(sub)),
		conmon_example_instance_id_to_string(conmon_client_subscription_get_instance_id(sub), buf, 64)
		);
	for (n = 0; n < CONMON_MAX_NETWORKS; n++)
	{
		if (available & (1 << n))
		{
			printf("%c", (active & (1 << n)) ? '+' : '-');
		}
	}
}


static void
print_connection_state(conmon_client_t * client)
{
	printf("Conmon connection state is now %s\n", conmon_example_client_state_to_string(conmon_client_state(client)));
	if (conmon_client_state(client) == CONMON_CLIENT_CONNECTED)
	{
		char buf[1024];
		printf("  Networks are %s\n", conmon_example_networks_to_string(conmon_client_get_networks(client), buf, sizeof(buf)));
		printf("  Dante device name is \"%s\"\n", conmon_client_get_dante_device_name(client));
		printf("  DNS domain name is \"%s\"\n", conmon_client_get_dns_domain_name(client));
	}
	printf("\n");
}

static void
handle_connection_state_changed
(
	conmon_client_t * client
) {
	printf("Connection state changed\n");
	print_connection_state(client);
	if (g_req_id != CONMON_CLIENT_NULL_REQ_ID)
	{
		printf("Discarding request 0x%p\n", g_req_id);
		g_req_id = CONMON_CLIENT_NULL_REQ_ID;
			// clear any existing request
	}
}

static void
handle_networks_changed
(
	conmon_client_t * client
) {
	char buf[BUFSIZ];
	printf("  Networks changed, now %s\n", conmon_example_networks_to_string(conmon_client_get_networks(client), buf, sizeof(buf)));
}


static void
handle_dante_device_name_changed
(
	conmon_client_t * client
) {
	printf("  Dante device name changed, now \"%s\"\n", conmon_client_get_dante_device_name(client));
}


static void handle_dns_domain_name_changed
(
	conmon_client_t * client
) {
	printf("  DNS domain name changed, now \"%s\"\n", conmon_client_get_dns_domain_name(client));
}


static void
handle_subscriptions_changed
(
	conmon_client_t * client,
	unsigned int num_changes,
	const conmon_client_subscription_t * const * changes
) {
	aud_utime_t now;
	unsigned int i;

	AUD_UNUSED(client);

	aud_utime_get(&now);
	printf("  %u.%u: Subscriptions changed:\n", (unsigned int) now.tv_sec, (unsigned int) now.tv_usec);
	for (i = 0; i < num_changes; i++)
	{
		printf("    ");
		print_subscription(changes[i]);
		printf("\n");
	}
}

static void
handle_event
(
	const conmon_client_event_t * ev
) {
	char buf[1024];
	size_t len = sizeof(buf);
	conmon_example_event_to_string(ev, buf, len);
	printf("%s\n", buf);
}

static void
handle_control_message
(
	conmon_client_t * client,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
) {
	handle_message(client, CONMON_CHANNEL_TYPE_CONTROL, CONMON_CHANNEL_DIRECTION_RX, head, body);
}

static void 
handle_monitoring_message
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
) {
	handle_message(client, channel_type, channel_direction, head, body);
}

static void
list_subscriptions(conmon_client_t * client)
{
	uint16_t i, max = conmon_client_max_subscriptions(client);
	printf ("Max subscriptions: %d\n", max);
	for (i = 0; i < max; i++)
	{
		const conmon_client_subscription_t * sub = conmon_client_subscription_at_index(client, i);
		if (sub)
		{
			printf("  %d: ", i);
			print_subscription(sub);
			printf("\n");
		}
	}
}

static void
print_commands(char c)
{
	printf("Usage:\n");
	if (c == '\0' || c == 'r' || c == 'd')
	{
		printf(" [r|d] control           register/deregister for control messages\n");
		printf(" [r|d] [t|r]x metering   register/deregister for tx/rx metering address information\n");
		printf(" [r|d] [t|r]x status     register/deregister for tx/rx messages on status channel\n");
		printf(" [r|d] [t|r]x broadcast  register/deregister for tx/rx messages on broadcast channel\n");
		printf(" [r|d] local             register/deregister for messages on the local channel\n");
		printf(" [r|d] serial            register/deregister for messages on the serial channel\n");
		printf(" [r|d] [t|r]x vbroadcast register/deregister for tx/rx messages on vendor broadcast channel\n");
	}
	if (c == '\0' || c == 's' || c == 'u')
	{
		printf(" [s|u] CHANNEL DEVICE  subscribe to / unsubscribe from channel CHANNEL on device DEVICE\n");
		printf(" [s|u] CHANNEL         subscribe to / unsubscribe from channel CHANNEL for ALL devices\n");
		printf(" s                     list subscriptions\n");
	}
	if (c == '\0' || c == 'e')
	{
		printf(" e +                   enable Tx Metering\n");
		printf(" e -                   disable Tx Metering\n");
	}
	if (c == '\0' || c == '#')
	{
		printf(" # control DEVICE N    send a control message to device DEVICE with length N\n");
		printf(" # CHANNEL N           send a message on channel CHANNEL with length N\n");
	}
	if (c == '\0' || c == 'm')
	{
		printf(" m control DEVICE MSG  send a control message to device DEVICE with payload MSG\n");
		printf(" m CHANNEL MSG         send a message on channel CHANNEL with payload MSG\n");
	}
	if (c == '\0' || c == 'M')
	{
		printf(" M CHANNEL MSG         send a message on channel CHANNEL with payload MSG, don't wait for a response\n");
	}
	if (c == '\0' || c == 'l')
	{
		printf(" a [+|-]               enabled / disable remote control access\n");
	}
	if (c == '\0' || c == 'i')
	{
		printf(" i DEVICE PROCESS      get name for instance id DEVICE/PROCESS\n");
		printf(" i NAME                get instance id for NAME\n");
	}
	if (c == '\0' || c == 'z')
	{
		printf(" z N                   sleep for N seconds\n");
	}
	if (c == '\0' || c == 'c')
	{
		printf(" c +                   connect using auto-connect\n");
		printf(" c -                   disconnect\n");
		printf(" c PORT                set server port (must be disconnected)\n");
	}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	if (c == '\0' || c == 'D')
	{
		printf(" D NAME                switch to managed domain NAME\n");
		printf(" D *                   switch to adhoc domain\n");
		printf(" D +                   connect to DDM\n");
		printf(" D -                   disconnect from DDM\n");
	}
#endif
	if (c == '\0')
	{
		printf(" ?                     help\n");
		printf(" q                     quit\n");
	}
}

static conmon_client_request_id_t
process_line(char * buf, conmon_client_t * client)
{
	char c, d, sign;
	char channel[BUFSIZ];
	char device[BUFSIZ];
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	char domain[BUFSIZ];
#endif
	conmon_channel_type_t channel_type;
	conmon_channel_direction_t channel_direction;
	conmon_name_t device_name;
	conmon_message_body_t body;
	unsigned int process_id;
	unsigned int n;
	
	aud_error_t result = AUD_SUCCESS;
	conmon_client_request_id_t req_id = CONMON_CLIENT_NULL_REQ_ID;

	if (!buf)
	{
		return req_id;
	}
	while (isspace(*buf))
	{
		buf++;
	}
	if (*buf == '\0')
	{
		return req_id;
	}

	if (sscanf(buf, "%c %cx %s", &c, &d, channel) == 3 && (c == 'r' || c == 'd') && (d == 'r' || d == 't'))
	{
		channel_type = channel_type_from_string(channel);
		channel_direction = (d == 't' ? CONMON_CHANNEL_DIRECTION_TX : CONMON_CHANNEL_DIRECTION_RX);
		if (c == 'r')
		{
			printf("Registering for %cx messages on channel %s\n", d, channel);
			result =
				conmon_client_register_monitoring_messages (
					client, & handle_response, & req_id,
					channel_type, channel_direction, handle_monitoring_message
				);
		}
		else
		{
			printf("Deregistering for %cx messages on channel %s\n", d, channel);
			result =
				conmon_client_register_monitoring_messages (
					client, & handle_response, & req_id,
					channel_type, channel_direction, NULL
				);
		}
	}
	else if (sscanf(buf, "%c %s", &c, channel) == 2 && (c == 'r' || c == 'd') && !strcmp(channel, "serial"))
	{
		if (c == 'r')
		{
			printf("Registering for serial messages\n");
			result =
					conmon_client_register_monitoring_messages (
						client, & handle_response, & req_id,
						CONMON_CHANNEL_TYPE_SERIAL, CONMON_CHANNEL_DIRECTION_RX, handle_monitoring_message
					);
		}
		else
		{
			printf("Deregistering for serial messages\n");
			result =
					conmon_client_register_monitoring_messages (
						client, & handle_response, & req_id,
						CONMON_CHANNEL_TYPE_SERIAL, CONMON_CHANNEL_DIRECTION_RX, NULL
					);
		}
	}
	else if (sscanf(buf, "%c %s", &c, channel) == 2 && (c == 'r' || c == 'd') && !strcmp(channel, "local"))
	{
		if (c == 'r')
		{
			printf("Registering for local messages\n");
			result =
					conmon_client_register_monitoring_messages (
						client, & handle_response, & req_id,
						CONMON_CHANNEL_TYPE_LOCAL, CONMON_CHANNEL_DIRECTION_TX, handle_monitoring_message
					);
		}
		else
		{
			printf("Deregistering for local messages\n");
			result =
					conmon_client_register_monitoring_messages (
						client, & handle_response, & req_id,
						CONMON_CHANNEL_TYPE_LOCAL, CONMON_CHANNEL_DIRECTION_TX, NULL
					);
		}
	}
	else if (sscanf(buf, "%c %s", &c, channel) == 2 && (c == 'r' || c == 'd') && !strcmp(channel, "control"))
	{
		if (c == 'r')
		{
			printf("Registering for control messages\n");
			result =
				conmon_client_register_control_messages (
					client, & handle_response, & req_id,
					handle_control_message
				);
		}
		else
		{
			printf("Deregistering for control messages\n");
			result =
				conmon_client_register_control_messages (
					client, & handle_response, & req_id,
					NULL
				);
		}
	}
	else if (sscanf(buf, "m control %s %[^\r\n]*", device_name, body.data) == 2)
	{
		// create a control message and send it...
		uint16_t body_size = (uint16_t) strlen((char *) body.data);
		
		printf("Sending a control message of size %d to device %s\n", body_size, device_name);
		result =
			conmon_client_send_control_message (
				client, & handle_response, & req_id,
				device_name, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
				&body, body_size, NULL
			);
	}
	else if (sscanf(buf, "%c %s %[^\r\n]*", &c, channel, body.data) == 3 && (c == 'M' || c == 'm'))
	{
		conmon_client_response_fn * p_response_fn = (c == 'm') ? &handle_response : NULL;
		conmon_client_request_id_t * p_req_id = (c == 'm') ? &req_id : NULL;
		channel_type = channel_type_from_string(channel);
		if (channel_type == CONMON_CHANNEL_TYPE_CONTROL)
		{
			// create a message and send it...
			uint16_t body_size = (uint16_t) strlen((char *) body.data);
			
			printf("Sending a control message of size %d to local device\n", body_size);
			result =
				conmon_client_send_control_message (
					client, p_response_fn, p_req_id,
					NULL, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
					&body, body_size, NULL
				);
		}
		else if (channel_type >= CONMON_CHANNEL_TYPE_METERING && channel_type < CONMON_NUM_CHANNELS)
		{
			// create a message and send it...
			uint16_t body_size = (uint16_t) (strlen((char *) body.data) + 1);

			printf("Sending a message of size %d to channel %s\n", body_size, channel);
			result =
				conmon_client_send_monitoring_message (
					client, p_response_fn, p_req_id,
					channel_type, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
					&body, body_size
				);
		}
		else
		{
			print_commands('c');
			return req_id;
		}
	}
	else if (sscanf(buf, "# control %s %d", device_name, &n) == 2)
	{
		// create a control message and send it...
		if (n > CONMON_MESSAGE_MAX_BODY_SIZE)
		{
			printf("Can't attempt to send more than %d bytes\n", CONMON_MESSAGE_MAX_BODY_SIZE);
			result = AUD_ERR_INVALIDPARAMETER;
		}
		memset(body.data, 0, n);
		uint16_t body_size = (uint16_t) n;
		
		printf("Sending a control message of size %d to device %s\n", body_size, device_name);
		result =
			conmon_client_send_control_message (
				client, & handle_response, & req_id,
				device_name, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
				&body, body_size, NULL
			);
	}
	else if (sscanf(buf, "# %s %d", channel, &n) == 2)
	{
		// create a control message and send it...
		if (n > CONMON_MESSAGE_MAX_BODY_SIZE)
		{
			printf("Can't attempt to send more than %d bytes\n", CONMON_MESSAGE_MAX_BODY_SIZE);
			result = AUD_ERR_INVALIDPARAMETER;
		}
		memset(body.data, 0, n);
		uint16_t body_size = (uint16_t) n;

		channel_type = channel_type_from_string(channel);
		if (channel_type == CONMON_CHANNEL_TYPE_CONTROL)
		{
			printf("Sending a control message of size %d to local device\n", body_size);
			result =
				conmon_client_send_control_message (
					client, handle_response, &req_id,
					NULL, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
					&body, body_size, NULL
				);
		}
		else if (channel_type >= CONMON_CHANNEL_TYPE_METERING && channel_type < CONMON_NUM_CHANNELS)
		{
			printf("Sending a message of size %d to channel %s\n", body_size, channel);
			result =
				conmon_client_send_monitoring_message (
					client, handle_response, &req_id,
					channel_type, CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC, &g_tx_vendor_id,
					&body, body_size
				);
		}
		else
		{
			print_commands('#');
			return req_id;
		}
	}
	else if (sscanf(buf, "%c %s %s", &c, channel, device_name) == 3 && (c == 's' || c == 'u'))
	{
		channel_type = channel_type_from_string(channel);
		if (channel_type != CONMON_CHANNEL_TYPE_NONE && channel_type != CONMON_CHANNEL_TYPE_CONTROL)
		{
			if (c == 's')
			{
				printf("Subscribing to %s@%s\n", channel, device_name);
				result =
					conmon_client_subscribe (
						client, & handle_response, & req_id,
						channel_type, device_name
					);
			}
			else
			{
				printf("Unsubscribing from %s@%s\n", channel, device_name);
				result =
					conmon_client_unsubscribe (
						client, & handle_response, & req_id,
						channel_type, device_name
					);
			}
		}
		else
		{
			print_commands(c);
			return req_id;
		}
	}
	else if (sscanf(buf, "%c %s", &c, channel) == 2 && (c == 's' || c == 'u'))
	{
		channel_type = channel_type_from_string(channel);
		if (channel_type != CONMON_CHANNEL_TYPE_NONE && channel_type != CONMON_CHANNEL_TYPE_CONTROL)
		{
			if (c == 's')
			{
				printf("Subscribing to all for %s\n", channel);
				result =
					conmon_client_subscribe_global (
						client, & handle_response, & req_id,
						channel_type
					);
			}
			else
			{
				printf("Unsubscribing from all on %s\n", channel);
				result =
					conmon_client_unsubscribe_global (
						client, & handle_response, & req_id,
						channel_type
					);
			}
		}
		else
		{
			print_commands(c);
			return req_id;
		}
	}
	else if (sscanf(buf, "%c %c", &c, &d) == 2 && (c == 'a') && (d == '+' || d == '-'))
	{
		if (d == '+')
		{
			result = conmon_client_set_remote_control_allowed(client, &handle_response, &req_id, AUD_TRUE);
		}
		else if (d == '-')
		{
			result = conmon_client_set_remote_control_allowed(client, &handle_response, &req_id, AUD_FALSE);
		}
	}
	else if (sscanf(buf, "%c %s %u", &c, device, &process_id) == 3 && (c == 'i'))
	{
		// id -> name
		const char * name;
		conmon_instance_id_t instance_id;
		if (!conmon_device_id_from_str(&instance_id.device_id, device))
		{
			// invalid format
			printf("Invalid device id format\n");
		}
		instance_id.process_id = (conmon_process_id_t) process_id;
		name = conmon_client_device_name_for_instance_id(client, &instance_id);

		if (name)
		{
			printf("Device '%s' has instance id %s/%04d\n", name, device, process_id);
		}
		else
		{
			printf("No known device with instance id %s/%04d\n", device, process_id);
		}
		return req_id;
	}
	else if (sscanf(buf, "%c %u", &c, &n) == 2 && (c == 'z'))
	{
		// disable tx metering
		aud_utime_t at = {0, 0};
		at.tv_sec = n;
		conmon_example_sleep(&at);
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'e') && (sign == '+'))
	{
		// enable tx metering
		result = conmon_client_set_tx_metering_enabled(client, &handle_response, &req_id, AUD_TRUE);
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'e') && (sign == '-'))
	{
		// disable tx metering
		result = conmon_client_set_tx_metering_enabled(client, &handle_response, &req_id, AUD_FALSE);
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'c') && (sign == '+'))
	{
		result = conmon_client_auto_connect(client);
		print_connection_state(client);
		//conmon_examples_event_loop_set_sockets_changed();
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'c') && (sign == '-'))
	{
		// disconnect
		result = conmon_client_disconnect(client);
		print_connection_state(client);
		//conmon_examples_event_loop_set_sockets_changed();
	}
	else if (sscanf(buf, "%c %d", &c, &n) == 2 && (c == 'c'))
	{
		// set server port 
		result = conmon_client_set_server_port(client, (uint16_t) n);
	}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'D') && (sign == '*'))
	{
		result = dante_domain_handler_set_current_domain_by_uuid(dapi_get_domain_handler(g_dapi), DANTE_DOMAIN_UUID_ADHOC);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting domain to ADHOC: %s\n", aud_error_get_name(result));
		}
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'D') && (sign == '+'))
	{
		dapi_utils_ddm_connect_blocking(&g_ddm_config, dapi_get_domain_handler(g_dapi), dapi_get_runtime(g_dapi), &g_running);
	}
	else if (sscanf(buf, "%c %c", &c, &sign) == 2 && (c == 'D') && (sign == '-'))
	{
		// disconnect from DDM
		dante_domain_handler_disconnect(dapi_get_domain_handler(g_dapi));
	}
	else if (sscanf(buf, "%c %s", &c, domain) == 2 && (c == 'D'))
	{
		result = dante_domain_handler_set_current_domain_by_name(dapi_get_domain_handler(g_dapi), domain);
		if (result != AUD_SUCCESS)
		{
			printf("Error setting domain to '%s': %s\n", domain, aud_error_get_name(result));
		}
	}
#endif
	else if (sscanf(buf, "%c", &c) == 1)
	{
		if (c == 's')
		{
			list_subscriptions(client);
			return req_id;
		}
		else if (c == 'D')
		{
			dapi_utils_print_domain_handler_info(dapi_get_domain_handler(g_dapi));
		}
		else if (c == '?')
		{
			print_commands('\0');
			return req_id;
		}
		else if (c == 'q')
		{
			g_running = AUD_FALSE;
			return req_id;
		}
		else
		{
			print_commands('\0');
			return req_id;
		}
	}
	else
	{
		print_commands('\0');
		return req_id;
	}

	if (result == AUD_SUCCESS)
	{
		if (req_id != CONMON_CLIENT_NULL_REQ_ID)
		{
			printf("Action has request id %p\n", req_id);
		}
		else
		{
			printf("Action has no request id, completed immediately\n");
		}
	}
	else
	{
		printf("Error performing action: %s\n" , aud_error_message(result, g_errbuf));
	}
	return req_id;
}

static aud_error_t
run(dante_runtime_t * runtime, conmon_client_t * client, FILE * fp)
{
	aud_error_t result = AUD_SUCCESS;
	//dante_sockets_t * my_sockets = conmon_examples_event_loop_get_user_sockets();
	aud_bool_t print_prompt = AUD_TRUE;
	aud_bool_t enable_prompt = AUD_FALSE;
	int in_sock = AUD_SOCKET_INVALID;

	if (! fp)
	{
		enable_prompt = AUD_TRUE;
#ifdef  _WIN32
		// set to line buffered mode.
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_PROCESSED_INPUT);
#else
		// read input from STDIN
		fp = stdin;
#endif
	}

	if (fp)
	{
#ifdef _WIN32
		in_sock = _fileno(fp);
#else
		in_sock = fileno(fp);
#endif
	}

	while(g_running)
	{
		char buf[BUFSIZ];

		// print prompt if needed
		if (enable_prompt && print_prompt)
		{
			printf("\n>>> ");
			fflush(stdout);
			print_prompt = AUD_FALSE;
		}

		dante_sockets_t my_sockets;
		result = dapi_utils_step(runtime, in_sock, &my_sockets);
		if (result != AUD_SUCCESS)
		{
			return result;
		}

		// and check input 
		buf[0] = '\0';
#ifdef _WIN32
		if ((! fp) && _kbhit())
		{
			DWORD len = 0;
			if (!ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE),buf,BUFSIZ-1, &len, 0))
			{
				printf("Error reading console: %d\n", GetLastError());
			}
			else if (len > 0)
			{
				buf[len] = '\0';
			}
			print_prompt = AUD_TRUE;
		}
		else
#endif
		{
			if (FD_ISSET(in_sock, &my_sockets.read_fds))
			{
				if (fgets(buf, BUFSIZ, fp) == NULL)
				{
					result = aud_error_get_last();
					if (feof(fp))
					{
						if (enable_prompt)
							printf("Exiting...\n");
						return AUD_SUCCESS;
					}
					else if (result == AUD_ERR_INTERRUPTED)
					{
						clearerr(stdin);
					}
					else
					{
						printf("Exiting with %s\n", aud_error_message(result, g_errbuf));
						return result;
					}
				}
				print_prompt = AUD_TRUE;
			}
		}

		// if we got some input then process the line
		if (buf[0])
		{
		#ifdef _WIN32
			if (enable_prompt)
				printf("\n");
		#endif
			g_req_id = process_line(buf, client);

			// run in this event loop until we get a response
			while (g_running && g_req_id != CONMON_CLIENT_NULL_REQ_ID)
			{
				result = dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
				if (result != AUD_SUCCESS)
				{
					return result;
				}
			}
		}
	}

	return result;
}

static void usage(char * bin)
{
	printf("Usage: %s OPTIONS\n", bin);
	printf("  -a auto-connect (default) \n");
	printf("  -noa disable auto-connect\n");
	printf("  -m=port enable metering channel on PORT\n");
	printf("  -n=NAME set client name\n");
	printf("  -i=FILE read FILE as input\n");
	printf("  -p=PORT set server port\n");
	printf("  -vid=ID set vendor id. Used for transmitted packets\n");
	printf("  -vba=A.B.C.D Set vendor broadcast channel address\n");
	printf("  -vrx Filter rx packets by transmit vendor id\n");
	printf("  -serial enable serial channel\n");
	printf("  -pp print payloads\n");

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	printf("Domain Options\n");
	printf("  --ddm Enable domains, using discovery to find the DDM\n");
	printf("  --ddm=HOST:PORT Enable domains, using the specified DDM address (bypasses DDM discovery)\n");
	printf("  --user=USER Specify the username for DDM authentication\n");
	printf("  --pass=PASS Specify a password for DDM authentication\n");
	printf("  --domain=DOMAIN Specify a domain to use once connected to the DDM\n");
#endif
}

#define STRINGIFY(X) #X

#define CHECK_RESULT(FUNC) \
	if (result != AUD_SUCCESS)  \
	{ \
		printf("Barfed on function %s (%s)!\n", STRINGIFY(FUNC), aud_error_get_name(result)); \
		goto cleanup; \
	}

#define PRINT_EVENT(EV,FUNC) \
	assert(EV); \
	if (ddh_event_get_flags(EV)) \
	{ \
		char buf[256]; \
		printf("%s triggered events %s\n", STRINGIFY(FUNC), ddh_event_to_string(EV, buf, sizeof(buf))); \
	}

int main(int argc, char * argv[])
{
	aud_error_t result;
	const char * client_name = "console_client";
	char * input_filename = NULL;
	int a;
	uint16_t server_port = 0;
	aud_bool_t auto_connect = AUD_TRUE;
	uint16_t metering_port = 0;
	uint32_t vendor_broadcast_address = 0;
	aud_bool_t serial = AUD_FALSE;
#ifdef WIN32
	aud_bool_t disable_quick_edit = AUD_FALSE;
#endif

	conmon_client_config_t * config = NULL;
	conmon_client_t * client = NULL;

	printf("console client, build timestamp %s %s\n", __DATE__, __TIME__);

	result = dapi_new(&g_dapi);
	if (result != AUD_SUCCESS)
	{
		printf("Error initialising conmon client library: %s\n",
			aud_error_message(result, g_errbuf));
		goto cleanup;
	}
	//aud_log_set_threshold(env->log, AUD_LOGTYPE_STDERR, AUD_LOG_DEBUG);

	for (a = 1; a < argc; a++)
	{
		char * arg = argv[a];
		char * value = argv[a];
		while (*value && *value != '=') value++;
		if (*value == '=') value++;
		if (*value == '\0') value = NULL;

		if (!strcmp(arg, "-a"))
		{
			auto_connect = AUD_TRUE;
		}
		else if (!strcmp(arg, "-noa"))
		{
			auto_connect = AUD_FALSE;
		}
		else if (!strncmp(arg, "-m=", 3) && value)
		{
			metering_port = (uint16_t) atoi(value);
#if defined(__linux__) || defined(__APPLE__)
			if(metering_port <= 1024)
			{
				printf("ERROR: port should be > 1024 on this platform\n");
				usage(argv[0]);
				exit(1);
			}
#endif
		}
		else if (!strncmp(arg, "-n=", 3) && value)
		{
			client_name = value;
		}
		else if (!strncmp(arg, "-i=", 3) && value)
		{
			input_filename = value;
		}
		else if (!strncmp(arg, "-p=",3) && value)
		{
			server_port = (uint16_t) atoi(value);
		}
#ifdef WIN32
		else if (!strcmp(arg, "-qe"))
		{
			disable_quick_edit = AUD_TRUE;
		}
#endif
		else if (!strncmp(arg, "-vid=",5) && value)
		{
			unsigned int hex[8];
			int i;
			if (sscanf(value, "%02x%02x%02x%02x%02x%02x%02x%02x",
				 hex + 0, hex + 1, hex + 2, hex + 3,
				 hex + 4, hex + 5, hex + 6, hex + 7) < 8)
			{
				usage(argv[0]);
				exit(1);
			}
			for (i = 0; i < 8; i++)
			{
				g_tx_vendor_id.data[i] = (uint8_t) hex[i];
			}
		}
		else if (!strncmp(arg, "-vba=", 5) && value)
		{
			unsigned int ip[4];
			uint8_t * ip8 = (uint8_t *) &vendor_broadcast_address;
			if (sscanf(value, "%u.%u.%u.%u", ip+0, ip+1, ip+2, ip+3) < 4)
			{
				usage(argv[0]);
				exit(1);
			}
			if (ip[0] != 239 || ip[1] != 254)
			{
				printf("ERROR: Broadcast address should be 239.254.X.Y\n");
				usage(argv[0]);
				exit(1);
			}
			ip8[0] = (uint8_t) ip[0];
			ip8[1] = (uint8_t) ip[1];
			ip8[2] = (uint8_t) ip[2];
			ip8[3] = (uint8_t) ip[3];
		}
		else if (!strcmp(arg, "-vrx"))
		{
			g_rx_vendor_id = &g_tx_vendor_id;
		}
		else if (!strcmp(arg, "-serial"))
		{
			serial = AUD_TRUE;
		}
		else if (!strcmp(arg, "-pp"))
		{
			g_print_payloads = AUD_TRUE;
		}
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		else if (dapi_utils_ddm_config_parse_one(&g_ddm_config, arg, &result))
		{
			if (result != AUD_SUCCESS)
			{
				usage(argv[0]);
				exit(1);
			}
		}
#endif
		else
		{
			usage(argv[0]);
			exit(1);
		}
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(disable_quick_edit);
#endif

	dante_domain_handler_t * handler = g_dapi ? dapi_get_domain_handler(g_dapi) : NULL;
	dante_runtime_t * runtime = g_dapi ? dapi_get_runtime(g_dapi) : NULL;

	assert(runtime);
	assert(handler);

	config = conmon_client_config_new(client_name);
	if (!config)
	{
		printf("Error creating client config: NO_MEMORY\n");
		goto cleanup;
	}
	conmon_client_config_set_server_port(config, server_port);
	if (vendor_broadcast_address)
	{
		conmon_client_config_set_vendor_broadcast_channel_enabled(config, AUD_TRUE);
		conmon_client_config_set_vendor_broadcast_channel_address(config, vendor_broadcast_address);
	}
	if (metering_port)
	{
		conmon_client_config_set_metering_channel_enabled(config, AUD_TRUE);
		conmon_client_config_set_metering_channel_port(config, metering_port);
	}
	if (serial)
	{
		conmon_client_config_set_serial_channel_enabled(config, AUD_TRUE);
	}
	result = conmon_client_new_dapi(g_dapi, config, &client);
	conmon_client_config_delete(config);
	config = NULL;

	if (result != AUD_SUCCESS)
	{
		printf("Error creating client: %s\n", aud_error_message(result, g_errbuf));
		goto cleanup;
	}

	conmon_client_set_connection_state_changed_callback(client, handle_connection_state_changed);
	conmon_client_set_networks_changed_callback(client, handle_networks_changed);
	conmon_client_set_dante_device_name_changed_callback(client, handle_dante_device_name_changed);
	conmon_client_set_dns_domain_name_changed_callback(client, handle_dns_domain_name_changed);
	conmon_client_set_subscriptions_changed_callback(client,handle_subscriptions_changed);
	conmon_client_set_event_callback(client, handle_event);

	if (auto_connect)
	{
		result = conmon_client_auto_connect(client);
		if (result != AUD_SUCCESS)
		{
			printf("Error enabling auto-connect: %s\n", aud_error_message(result, g_errbuf));
			goto cleanup;
		}

		printf("Auto-connecting, blocking & waiting for local connection to complete\n");
		while (g_running && conmon_client_state(client) != CONMON_CLIENT_CONNECTED)
		{
			dapi_utils_step (runtime, AUD_SOCKET_INVALID, NULL);
		}
	}
	else
	{
		result = conmon_client_connect (client, & handle_connect_response, &g_req_id);
		if (result != AUD_SUCCESS)
		{
			printf("Error connecting client: %s\n", aud_error_message(result, g_errbuf));
			goto cleanup;
		}

		printf("Connecting, request id is %p, blocking & waiting for connection to complete\n", g_req_id);
		while (g_running && g_req_id != CONMON_CLIENT_NULL_REQ_ID)
		{
			dapi_utils_step(runtime, AUD_SOCKET_INVALID, NULL);
		}
	}

	if (conmon_client_state(client) == CONMON_CLIENT_CONNECTED)
	{
		dante_instance_id_str_t id_str;
		const dante_version_t * version = conmon_client_get_server_version(client);
		const dante_version_build_t * version_build = conmon_client_get_server_version_build(client);
		printf("Connected, server_version=%u.%u.%u.%u instance_id=%s\n",
			version->major, version->minor, version->bugfix, version_build->build_number,
			dante_instance_id_to_string(conmon_client_get_instance_id(client), id_str));
	}
	else
	{
		printf("Error connecting, aborting\n");
		goto cleanup;
	}

#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	result = dapi_utils_ddm_connect_blocking(&g_ddm_config, handler, runtime, &g_running);
	if (result != AUD_SUCCESS)
	{
		goto cleanup;
	}
#endif

	// Print current domain info before doing anything else
	printf("Current domain configuration:\n");
	dapi_utils_print_domain_handler_info(handler);
	dante_domain_handler_set_event_fn(handler, dapi_utils_ddh_event_print_changes);

	if (input_filename)
	{
		// run script before handing control to STDIN
		FILE * fp = fopen(input_filename, "r");
		if (!fp)
		{
			int errcode = (int) aud_system_error_get_last();
			printf("Error opening file '%s': %s (%d)\n"
				, input_filename
				, strerror(errcode)
				, errcode
			);
			return 1;
		}

		run(runtime, client, fp);

		fclose(fp);
	}
	run(runtime, client, NULL);

cleanup:
	if (client)
	{
		conmon_client_delete(client);
	}
	if (g_dapi)
	{
		dapi_delete(g_dapi);
		g_dapi = NULL;
		runtime = NULL;
		handler = NULL;
	}
	return result;
}


static void
handle_connect_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
) {
	aud_errbuf_t errbuf;
	printf ("  Got response for connection request %p: %s\n",
		request_id, aud_error_message(result, errbuf));
	if (g_req_id == request_id)
	{
		g_req_id = CONMON_CLIENT_NULL_REQ_ID;
	}
	print_connection_state(client);
}


static void
handle_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
) {
	aud_errbuf_t errbuf;

	AUD_UNUSED(client);

	printf ("  Got response for request %p: %s\n", request_id, aud_error_message(result, errbuf));
	if (g_req_id == request_id)
	{
		g_req_id = CONMON_CLIENT_NULL_REQ_ID;
	}
}

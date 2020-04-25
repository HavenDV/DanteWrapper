/*
 * File     : conmon_examples.c
 * Created  : July 2008
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Common information for the conmon example clients
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#include "conmon_examples.h"
#include <stdio.h>


//----------------------------------------------------------
// Printing functions for the example clients
//----------------------------------------------------------

const char *
conmon_example_client_state_to_string
(
	conmon_client_state_t state
) {
	switch (state)
	{
	case CONMON_CLIENT_NO_CONNECTION:     return "NO_CONNECTION";
	case CONMON_CLIENT_CONNECTING:        return "CONNECTING";
	case CONMON_CLIENT_CONNECTED:         return "CONNECTED";
	case CONMON_CLIENT_RECONNECT_PENDING: return "RECONNECT_PENDING";
	default:                              return "???";
	}
}

const char *
conmon_example_channel_type_to_string
(
	conmon_channel_type_t channel_type
) {
	switch(channel_type)
	{
	case CONMON_CHANNEL_TYPE_CONTROL:    return "control";
	case CONMON_CHANNEL_TYPE_METERING:   return "metering";
	case CONMON_CHANNEL_TYPE_STATUS:     return "status";
	case CONMON_CHANNEL_TYPE_BROADCAST:  return "broadcast";
	case CONMON_CHANNEL_TYPE_LOCAL:      return "local";
	case CONMON_CHANNEL_TYPE_SERIAL:     return "serial";
	case CONMON_CHANNEL_TYPE_KEEPALIVE:  return "keepalive";
	case CONMON_CHANNEL_TYPE_VENDOR_BROADCAST: return "vendor_broadcast";
	case CONMON_CHANNEL_TYPE_MONITORING: return "monitoring";
	default:                             return "unknown";
	}
}

const char *
conmon_example_rxstatus_to_string
(
	conmon_rxstatus_t rxstatus
) {
	switch(rxstatus)
	{
	case CONMON_RXSTATUS_NONE:          return "none";
	case CONMON_RXSTATUS_PREPARING:     return "preparing";
	case CONMON_RXSTATUS_RESOLVED:      return "resolved";
	case CONMON_RXSTATUS_UNRESOLVED:    return "unresolved";
	case CONMON_RXSTATUS_UNICAST:       return "unicast";
	case CONMON_RXSTATUS_MULTICAST:     return "multicast";
	case CONMON_RXSTATUS_DOMAIN:        return "domain";
	case CONMON_RXSTATUS_NO_CONNECTION: return "no connection";
	case CONMON_RXSTATUS_COMMS_ERROR:   return "comms error";
	case CONMON_RXSTATUS_INVALID_REPLY: return "invalid reply";
	case CONMON_RXSTATUS_POLICY:        return "policy";
	case CONMON_RXSTATUS_TX_NO_CHANNEL: return "tx no channel";
	default:                            return "unknown";
	}
}

// A helper function to print a device id to a string buffer
char *
conmon_example_device_id_to_string
(
	const conmon_device_id_t * id,
	char * buf,
	size_t len
) {
	SNPRINTF(buf, len,
		"%02x%02x%02x%02x%02x%02x%02x%02x",
		id->data[0], id->data[1],
		id->data[2], id->data[3],
		id->data[4], id->data[5],
		id->data[6], id->data[7]);
	return buf;
}

// A helper function to print an instance id to a string buffer
char *
conmon_example_instance_id_to_string
(
	const conmon_instance_id_t * id,
	char * buf,
	size_t len
) {
	if (id)
	{
		SNPRINTF(buf, len,
			"%02x%02x%02x%02x%02x%02x%02x%02x/%04x",
			id->device_id.data[0], id->device_id.data[1],
			id->device_id.data[2], id->device_id.data[3],
			id->device_id.data[4], id->device_id.data[5],
			id->device_id.data[6], id->device_id.data[7],
			id->process_id);
	}
	else
	{
		SNPRINTF(buf, len, "[null]");
	}
	return buf;
}

// A helper function to print a vendor id to a string buffer
char *
conmon_example_vendor_id_to_string
(
	const conmon_vendor_id_t * id,
	char * buf,
	size_t len
) {
	SNPRINTF(buf, len,
		"%02x%02x%02x%02x%02x%02x%02x%02x",
		id->data[0], id->data[1],
		id->data[2], id->data[3],
		id->data[4], id->data[5],
		id->data[6], id->data[7]);
	return buf;
}

// A helper function to print a vendor id to a string buffer
char *
conmon_example_model_id_to_string
(
	const conmon_audinate_model_id_t * id,
	char * buf,
	size_t len
) {
	SNPRINTF(buf, len,
		"%02x%02x%02x%02x%02x%02x%02x%02x",
		id->data[0], id->data[1],
		id->data[2], id->data[3],
		id->data[4], id->data[5],
		id->data[6], id->data[7]);
	return buf;
}

// A helper function to print a set of endpoint addresses to a string buffer

char *
conmon_example_endpoint_addresses_to_string
(
	const conmon_endpoint_addresses_t * addresses,
	char * buf,
	size_t len
) {
	struct in_addr in;
	int i, offset = 0;
	buf[0] = '\0';
	if (addresses)
	{
		if (addresses->num_networks)
		{
			char addr[32];

			for (i = 0; i < addresses->num_networks; i++)
			{
				in.s_addr = addresses->addresses[i].host;
				offset += SNPRINTF(buf+offset, len-offset, "%c%s:%u",(i ? ',' : '['),
					aud_inet_ntoa(in.s_addr, addr, 32), addresses->addresses[i].port);
			}
			SNPRINTF(buf+offset, len-offset, "]");
		}
		else
		{
			SNPRINTF(buf, len, "[]");
		}
	}
	return buf;
}

// A helper function to print a set of endpoint addresses to a string buffer
char *
conmon_example_networks_to_string
(
	const conmon_networks_t * networks,
	char * buf,
	size_t len
) {
	struct in_addr address;
	struct in_addr netmask;
	struct in_addr dns_server;
	struct in_addr gateway;

	int i, offset = 0;
	buf[0] = '\0';
	if (networks->num_networks)
	{
		for (i = 0; i < networks->num_networks; i++)
		{
			char addr[32];
			char mask[32];
			char gw[32];
			char dns[32];
			const conmon_network_t * n = networks->networks + i;
			address.s_addr = n->ip_address;
			netmask.s_addr = n->netmask;
			dns_server.s_addr = n->dns_server;
			gateway.s_addr = n->gateway;
			offset += SNPRINTF(buf+offset, len-offset, "%c%d:",
				(i ? ',' : '['),
				n->interface_index);
			if (n->flags & CONMON_NETWORK_FLAG_STATIC)
			{
				offset += SNPRINTF(buf+offset, len-offset, " STATIC");
			}
			offset += SNPRINTF(buf+offset, len-offset, " %s %d %02x:%02x:%02x:%02x:%02x:%02x",
				n->is_up ? "up" : "down", n->link_speed,
				n->mac_address[0], n->mac_address[1], n->mac_address[2],
				n->mac_address[3], n->mac_address[4], n->mac_address[5]);
			offset += SNPRINTF(buf + offset, len - offset, " addr=%s", aud_inet_ntoa(address.s_addr, addr, 32));
			offset += SNPRINTF(buf + offset, len - offset, " mask=%s", aud_inet_ntoa(netmask.s_addr, mask, 32));
			offset += SNPRINTF(buf + offset, len - offset, " dns=%s", aud_inet_ntoa(dns_server.s_addr, dns, 32));
			offset += SNPRINTF(buf + offset, len - offset, " gateway=%s", aud_inet_ntoa(gateway.s_addr, gw, 32));
		}
		offset += SNPRINTF(buf+offset, len-offset, "]");
	}
	else
	{
		SNPRINTF(buf, len, "[]");
	}
	return buf;
}

char *
conmon_example_subscription_infos_to_string
(
	uint16_t num_subscriptions,
	const conmon_subscription_info_t ** subscriptions,
	char * buf,
	size_t len
) {
	int i, offset = 0;
	buf[0] = '\0';
	for (i = 0; i < num_subscriptions; i++)
	{
		char id[128], addrs[1024];
		const conmon_subscription_info_t * s = subscriptions[i];
		conmon_example_instance_id_to_string(&s->instance_id, id, 128);
		conmon_example_endpoint_addresses_to_string(&s->addresses, addrs, 1024);
		offset += SNPRINTF(buf+offset, len-offset, "  %d: %s %s\n", i, id, addrs);
	}
	return buf;
}


char *
conmon_example_metering_peaks_to_string
(
	const conmon_metering_message_peak_t * peaks,
	uint16_t num_peaks,
	char * buf,
	size_t len
) {
	uint16_t c;
	size_t offset = 0;

	buf[0] = '\0';
	for (c = 0; c < num_peaks; c++)
	{
		switch (peaks[c])
		{
		case CONMON_METERING_PEAK_CLIP:
			offset += SNPRINTF(buf+offset, len-offset, " CLIP");
			break;
		case CONMON_METERING_PEAK_MUTE:
			offset += SNPRINTF(buf+offset, len-offset, " MUTE");
			break;
		case CONMON_METERING_PEAK_START_OF_MESSAGE:
			offset += SNPRINTF(buf+offset, len-offset, " SOM");
			break;
		default:
			offset += SNPRINTF(buf+offset, len-offset, " %4.1f",
				conmon_metering_message_peak_to_float(peaks[c]));
		}
	}
	return buf;
}


//----------------------------------------------------------

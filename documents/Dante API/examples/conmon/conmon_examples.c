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
#include <assert.h>
#include <stdio.h>

#if 1

//----------------------------------------------------------
// Timeout utilities
//----------------------------------------------------------

/*
	Validate that an absolute time is non-zero

	Returns NULL if input is NULL or 0.0
	Otherwise returns pointer to input
 */
static aud_utime_t *
abs_time_non_zero(aud_utime_t * abs_time)
{
	if (abs_time)
	{
		if (! (abs_time->tv_sec && abs_time->tv_usec))
		{
			return NULL;
		}
	}

	return abs_time;
}

/*
	Convert an absolute time into a delta from the current time.

	If input is NULL or 0.0 return NULL and don't modify input.
	Otherwise return pointer to modified input structure.
	If input is less than the current time then set to { 0, 0 }.
 */
static aud_utime_t *
abs_time_to_delta(aud_utime_t * abs_time)
{
	aud_utime_t curr_time;

	if (! abs_time_non_zero(abs_time))
		return NULL;

	aud_utime_get(&curr_time);
	if (aud_utime_compare(&curr_time, abs_time) >= 0)
	{
		abs_time->tv_sec = 0;
		abs_time->tv_usec = 0;
	}
	else
	{
		aud_utime_sub(abs_time, &curr_time);
	}
	return abs_time;
}

static void
time_min(aud_utime_t * time_dst, const aud_utime_t * time_src)
{
	if (! time_src)
		return;

	if (aud_utime_compare(time_src, time_dst) < 0)
	{
		*time_dst = *time_src;
	}
}
#endif

#if 1

//----------------------------------------------------------
// Implementation of event loop infrastructure
//----------------------------------------------------------

typedef struct conmon_examples_event_loop conmon_examples_event_loop_t;

struct conmon_examples_event_loop
{
	conmon_client_t * client;
	struct
	{
		dante_sockets_t curr, user, active;
		aud_bool_t stale, have_active;
	} sockets;
};

static conmon_examples_event_loop_t g_examples_event_loop = { NULL };

static void
on_conmon_sockets_changed
(
	conmon_client_t * client
)
{
	AUD_UNUSED(client);

	g_examples_event_loop.sockets.stale = AUD_TRUE;
		// mark sockets as stale
		// we reconfigure them just before calling 'select'
}

aud_error_t
conmon_examples_event_loop_init
(
	conmon_client_t * client
)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;

	if (loop_info->client)
		return AUD_ERR_INVALIDSTATE;

	loop_info->client = client;
	dante_sockets_clear(&loop_info->sockets.curr);
	dante_sockets_clear(&loop_info->sockets.user);
	dante_sockets_clear(&loop_info->sockets.active);
	loop_info->sockets.stale = AUD_TRUE;
	loop_info->sockets.have_active = AUD_FALSE;

	conmon_client_set_sockets_changed_callback(client, on_conmon_sockets_changed);
	return AUD_SUCCESS;
}


static aud_error_t
run_event_loop_body
(
	aud_utime_t * timeout_p
)
{
	int select_result;
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;

	if (loop_info->sockets.stale)
	{
		loop_info->sockets.stale = AUD_FALSE;
		dante_sockets_copy(&loop_info->sockets.user, &loop_info->sockets.curr);
			// NOTE: src is first argument!
		conmon_client_get_sockets(loop_info->client, &loop_info->sockets.curr);
	}

	dante_sockets_copy(&loop_info->sockets.curr, &loop_info->sockets.active);
	loop_info->sockets.have_active = AUD_FALSE;
#ifdef WIN32
	if (loop_info->sockets.active.n == 0)
	{
		unsigned int ms = (timeout_p->tv_sec*1000) + (timeout_p->tv_usec/1000);
		Sleep(ms);
		conmon_client_process_sockets(loop_info->client, NULL, NULL);
		return AUD_SUCCESS;
	}
#endif
	select_result = select(loop_info->sockets.active.n, &loop_info->sockets.active.read_fds, NULL, NULL, timeout_p);
	if (select_result > 0)
	{
		loop_info->sockets.have_active = AUD_TRUE;
		conmon_client_process_sockets(loop_info->client, &loop_info->sockets.active, NULL);
		return AUD_SUCCESS;
	}
	else if (select_result == 0)
	{
		conmon_client_process_sockets(loop_info->client, NULL, NULL);
		return AUD_ERR_TIMEDOUT;
	}
	else
	{
		return aud_error_get_last();
	}
}


aud_error_t
conmon_examples_event_loop_run_once
(
	const aud_utime_t * timeout
)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;
	aud_utime_t * timeout_p, local_timeout;

	if (! loop_info->client)
		return AUD_ERR_INVALIDSTATE;

	// This version converts the abs_time to a delta and then compares deltas

	// Get conmon's internal timeout
	conmon_client_get_next_action_time(loop_info->client, &local_timeout);
	timeout_p = abs_time_to_delta(&local_timeout);

	// If the client gave us a timeout, compare with our local one
	if (timeout)
	{
		if (timeout_p)
		{
			time_min(timeout_p, timeout);
		}
		else
		{
			local_timeout = * timeout;
			timeout_p = &local_timeout;
		}
	}

	return run_event_loop_body(timeout_p);
}


aud_error_t
conmon_examples_event_loop_run_until
(
	const aud_utime_t * abs_time
)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;
	aud_utime_t * timeout_p, local_abs_timeout;

	if (! loop_info->client)
		return AUD_ERR_INVALIDSTATE;

	// This version compares absolute times and then converts to a delta

	// Get conmon's internal timeout
	conmon_client_get_next_action_time(loop_info->client, &local_abs_timeout);
	timeout_p = abs_time_non_zero(&local_abs_timeout);

	// If the client gave us a timeout, compare with our local one
	if (abs_time)
	{
		if (timeout_p)
		{
			time_min(timeout_p, abs_time);
		}
		else
		{
			local_abs_timeout = * abs_time;
			timeout_p = &local_abs_timeout;
		}
	}

	if (timeout_p)
	{
		abs_time_to_delta(timeout_p);
	}
	return run_event_loop_body(timeout_p);
}


dante_sockets_t *
conmon_examples_event_loop_get_user_sockets(void)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;

	return &loop_info->sockets.user;
}


void
conmon_examples_event_loop_set_sockets_changed(void)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;

	loop_info->sockets.stale = AUD_TRUE;
}


dante_sockets_t *
conmon_examples_event_loop_get_active_sockets(void)
{
	conmon_examples_event_loop_t * loop_info = &g_examples_event_loop;

	if (loop_info->sockets.have_active)
		return &loop_info->sockets.active;
	else
		return NULL;
}
#endif

//----------------------------------------------------------
// Support functions for the example clients
//----------------------------------------------------------

aud_error_t
conmon_example_sleep
(
	const aud_utime_t * at
) {
	if (at == NULL || at->tv_sec < 0 || at->tv_usec < 0)
	{
		return AUD_ERR_INVALIDPARAMETER;
	}
	else
	{
#ifdef WIN32
		DWORD result;
		int ms = (at->tv_sec * 1000) + ((at->tv_usec+999)/1000);
		result = SleepEx(ms, TRUE);
		if (result)
		{
			return aud_error_from_system_error(result);
		}
#else
		if ((at->tv_sec > 0) && sleep(at->tv_sec) > 0)
		{
			return AUD_ERR_INTERRUPTED;
		}
		if ((at->tv_usec > 0) && (usleep(at->tv_usec) < 0))
		{
			return aud_error_get_last();
		}
#endif
	}
	return AUD_SUCCESS;
}

aud_error_t
conmon_example_client_process
(
	conmon_client_t * client,
	dante_sockets_t * sockets,
	const aud_utime_t * timeout,
	aud_utime_t * next_action_timeout
) {
	dante_sockets_t temp_sockets = *sockets;
	//fd_set fd;
	int result;
	aud_utime_t temp = *timeout;
	
	result = select(temp_sockets.n, &temp_sockets.read_fds, NULL, NULL, &temp); // in win32, nfds is the NUMBER of sockets
	if (result < 0)
	{
		return aud_error_from_system_error(aud_system_error_get_last());
	}
	else if (result == 0)
	{
		return AUD_ERR_TIMEDOUT;
	}
	return conmon_client_process_sockets(client, &temp_sockets, next_action_timeout);
}

//----------------------------------------------------------
// Client-depenendent printing routings

char *
conmon_example_client_subscriptions_to_string
(
	uint16_t num_subscriptions,
	const conmon_client_subscription_t * const * subscriptions,
	char * buf,
	size_t len
) {
	int i, offset = 0;
	buf[0] = '\0';
	for (i = 0; i < num_subscriptions; i++)
	{
		char id[128], addrs[1024];
		const conmon_client_subscription_t * s = subscriptions[i];
		const char * type  = conmon_example_channel_type_to_string(conmon_client_subscription_get_channel_type(s));
		const char * device = conmon_client_subscription_get_device_name(s);
		const char * rxstatus = conmon_example_rxstatus_to_string(conmon_client_subscription_get_rxstatus(s));

		conmon_example_instance_id_to_string(conmon_client_subscription_get_instance_id(s), id, 128);
		conmon_example_endpoint_addresses_to_string(conmon_client_subscription_get_addresses(s), addrs, 1024);
		offset += SNPRINTF(buf+offset, len-offset, "  %d: %s@%s (%s %s) %s\n", i, type, device, id, addrs, rxstatus);
	}
	return buf;
}


//----------------------------------------------------------

/*
 * File     : dapi_utils.c
 * Created  : March 2017
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Helper functions for Dante Domains - I/O and DDM connection management
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#include "dapi_utils.h"
#include <stdio.h>

aud_error_t 
dapi_utils_step(dante_runtime_t * runtime, aud_socket_t in_sock, dante_sockets_t * out_sockets)
{
	dante_sockets_t step_sockets;
	if (out_sockets == NULL)
	{
		out_sockets = &step_sockets;
	}
	dante_sockets_clear(out_sockets);
	aud_utime_t my_timeout = {1, 0};
	
	aud_error_t result = dante_runtime_get_sockets_and_timeout(runtime, out_sockets, &my_timeout);
	if (result != AUD_SUCCESS)
	{
		return result;
	}
	if (in_sock != AUD_SOCKET_INVALID)
	{
		// Add input stream to our list of sockets
		dante_sockets_add_read(out_sockets, in_sock);
	}
	if (my_timeout.tv_sec >= 1)
	{
		my_timeout.tv_sec = 1;
		my_timeout.tv_usec = 0;
	}
	else if (my_timeout.tv_sec == 0 && my_timeout.tv_usec == 0)
	{
		//fprintf(stderr, "Timeout zero\n");
	}
	
	if (out_sockets->n == 0)
	{
#ifdef WIN32
		DWORD result;
		int ms = (my_timeout.tv_sec * 1000) + ((my_timeout.tv_usec+999)/1000);
		result = SleepEx(ms, TRUE);
		if (result)
		{
			return aud_error_from_system_error(result);
		}
#else
		if ((my_timeout.tv_sec > 0) && sleep(my_timeout.tv_sec) > 0)
		{
			return AUD_ERR_INTERRUPTED;
		}
		if ((my_timeout.tv_usec > 0) && (usleep(my_timeout.tv_usec) < 0))
		{
			return aud_error_get_last();
		}
#endif
	}
	else
	{
		int select_result = select(out_sockets->n, &out_sockets->read_fds, &out_sockets->write_fds, NULL, &my_timeout);
		if (select_result < 0)
		{
			result = aud_error_get_last();
			printf("Error processing sockets: %s (%d)\n"
				, aud_error_get_name(result)
				, result
			);
			return result;
		}
	}
	return dante_runtime_process_with_sockets(runtime, out_sockets);
}

#ifdef WIN32

void dapi_utils_check_quick_edit_mode(aud_bool_t disable)
{
	DWORD mode = 0;
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	if (!GetConsoleMode(handle, &mode))
	{
		DWORD error = GetLastError();
		printf("Error getting console mode: windows error is 0x%08x\n", error);
		return;
	}
	if (!(mode & ENABLE_QUICK_EDIT_MODE))
	{
		printf("Quick edit mode not enabled\n");
		return;
	}
	if (disable)
	{
		mode &= ~ENABLE_QUICK_EDIT_MODE;
		if (!SetConsoleMode(handle, mode))
		{
			DWORD error = GetLastError();
			printf("Error disabling QuickEdit mode: windows error is 0x%08x\n", error);
			return;
		}
		printf("Disabled QuickEdit mode\n");
		return;
	}
	printf("\n");
	printf("WARNING: Quick edit mode is enabled for this console. Highlighting text in the console window while this application is running will stall the application. This can lead to timeouts and other unexpected DAPI events.\n");
	printf("\n");
}

#endif


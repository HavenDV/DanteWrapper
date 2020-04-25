/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Win32-specific platform layer definitions
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file platform_specific.h
 *
 * Windows platform-specific types and definitions required for the Dante API.
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */
#ifndef _AUD_PLATFORM_SPECIFIC_H
#define _AUD_PLATFORM_SPECIFIC_H

/**
 * The name of this platform, used internally by the API
 */
#define AUD_PLATFORM "win32"

/**
 * Tell Visual studio to allow standard (but deprecated) C functions.
 */
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

/**
* Tell Visual studio to allow standard (but insecure) C++ STL functions
*/
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

/**
 * Increase the maximum number of sockets allowed (64 sockets just ain't enough!)
 */
#define FD_SETSIZE 1024

// Include winsock2 before windows to ensure
// that we are using WS2

#include <winsock2.h>
#include <ws2tcpip.h> // for INET_ADDRSTRLEN
#include <windows.h>
#include <mswsock.h> // For WSARecvMsg
#if (_MSC_VER == 1500)
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** type definition for unsigned 8-bit values */
typedef UINT8  uint8_t;
/** type definition for signed 8-bit values */
typedef  INT8   int8_t;
/** type definition for unsigned 16-bit values */
typedef UINT16 uint16_t;
/** type definition for signed 16-bit values */
typedef  INT16  int16_t;
/** type definition for unsigned 32-bit values */
typedef UINT32 uint32_t;
/** type definition for signed 32-bit values */
typedef  INT32  int32_t;
/** type definition for unsigned 64-bit values */
typedef UINT64 uint64_t;
/** type definition for signed 64-bit values */
typedef  INT64  int64_t;

#ifndef UINT8_MAX
#define UINT8_MAX MAXUINT8
#endif
#ifndef INT8_MIN
#define INT8_MIN MININT8
#endif
#ifndef INT8_MAX
#define INT8_MAX MAXINT8
#endif

#ifndef UINT16_MAX
#define UINT16_MAX MAXUINT16
#endif
#ifndef INT16_MIN
#define INT16_MIN MININT16
#endif
#ifndef INT16_MAX
#define INT16_MAX MAXINT16
#endif

#ifndef UINT32_MAX
#define UINT32_MAX MAXUINT32
#endif
#ifndef INT32_MIN
#define INT32_MIN MININT32
#endif
#ifndef INT32_MAX
#define INT32_MAX MAXINT32
#endif

#ifndef UINT64_MAX
#define UINT64_MAX MAXUINT64
#endif
#ifndef INT64_MIN
#define INT64_MIN MININT64
#endif
#ifndef INT64_MAX
#define INT64_MAX MAXINT64
#endif


/** type definition for 32-bit floating point value */
typedef float float32_t;

/** type definition for 32-bit floating point value */
typedef double float64_t;

/** Use the existing timeval struct for aud utime */
typedef struct timeval aud_utime_t;

/** Windows timeval structures use a long for microseconds */
typedef long aud_tv_usec_t;

/** Enable inline API functions */
#define AUD_INLINE __inline

enum
{
	/** A useful default buffer length for error strings */
	AUD_ENV_ERROR_STRLEN = 512
};

/** Define the cross-platform marker for invalid sockets */
#define AUD_SOCKET_INVALID INVALID_SOCKET

/** Define the cross-platform socket type */
typedef SOCKET aud_socket_t;

/** Windows support interrupt signals */
#define AUD_ENV_HAS_SELECT 1

/** Windows support interrupt signals */
#define AUD_ENV_CAN_SIGINT 1

/** Windows supports the ConmonManager API */
#define AUD_ENV_HAS_CMM 1

/** no extra functions */
#define AUD_ENV_HAS_PLATFORM_EXTRA 0

/** Windows has stdin */
#define AUD_ENV_HAS_STDIN 1

/** Windows does not allow select on stdin */
#define AUD_ENV_HAS_SELECTABLE_STDIN 0

/** Windows has per-interface DNS and gateway information */
#define AUD_ENV_HAS_INTERFACE_DNS_GATEWAY 1

/**
 * What is the minimum version of Bonjour needed,
 * defined in dns_sd.h as major*10000 + minor*100
 * Currently set to version 2.0 ie. 200*10000 + 0*100
 */
#define AUD_ENV_MIN_DNSSD_VERSION (200*10000)

/**
 * FD_COPY is not supported on all platforms, so we add a platform-layer wrapper.
 *
 * @param FROM the source fd_set
 * @param TO the destination fd_set
 */ 
#define AUD_FD_COPY(FROM,TO) memcpy(TO,FROM,sizeof(fd_set))

#ifdef __cplusplus
}
#endif

#endif

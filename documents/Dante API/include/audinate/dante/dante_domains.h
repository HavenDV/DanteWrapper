/*
* File     : $RCSfile$
* Created  : 23 Aug 2016 12:19:03
* Updated  : $Date$
* Author   : James Westendorp
* Synopsis : Dante domains types and structures
*
* This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1
*/

/**
 * @file dante_domains.h
 * Dante Domain types and structures
 */

#ifndef _DANTE_DOMAINS_H
#define _DANTE_DOMAINS_H


#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#endif

#define DANTE_DOMAIN_NAME_LENGTH 128
#define DANTE_DOMAIN_UUID_LENGTH 16
#define DANTE_DOMAIN_UUID_HEX_LENGTH 33
#define DANTE_DOMAIN_UUID_STRING_LEN 37

#ifdef __cplusplus
#define DANTE_DOMAIN_UUID_CAST
#else
#define DANTE_DOMAIN_UUID_CAST (dante_domain_uuid_t)
#endif

#define DANTE_DOMAIN_UUID_ADHOC_BYTES {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define DANTE_DOMAIN_UUID_1 DANTE_DOMAIN_UUID_CAST{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01} }
#define DANTE_DOMAIN_UUID_NONE DANTE_DOMAIN_UUID_CAST{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }
#define DANTE_DOMAIN_UUID_ADHOC DANTE_DOMAIN_UUID_CAST{ DANTE_DOMAIN_UUID_ADHOC_BYTES }

#define IS_NO_DOMAIN_UUID(X)    (dante_domain_uuid_cmp(X, DANTE_DOMAIN_UUID_NONE) == 0)
#define IS_ADHOC_DOMAIN_UUID(X) (dante_domain_uuid_cmp(X, DANTE_DOMAIN_UUID_ADHOC) == 0)
#define IS_MANAGED_DOMAIN_UUID(X)   (!IS_NO_DOMAIN_UUID(X) && !IS_ADHOC_DOMAIN_UUID(X))
#define IS_DDM_DOMAIN_UUID(X)   (IS_MANAGED_DOMAIN_UUID(X))

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t dante_domain_id_t;
#define DANTE_DOMAIN_ID_NONE 0x0000
#define DANTE_DOMAIN_ID_ADHOC 0xFFFF

union dante_domain_uuid
{
	uint8_t data[16];
	uint32_t data32[4];
};

// 128 bit uuid
typedef union dante_domain_uuid dante_domain_uuid_t;

/*
	return 0 if the ids are equal, non-zero otherwise
*/
int32_t dante_domain_uuid_cmp(dante_domain_uuid_t a, dante_domain_uuid_t b);

struct dante_domain_uuid_string
{
	char str[DANTE_DOMAIN_UUID_STRING_LEN];
};

typedef struct dante_domain_uuid_string dante_domain_uuid_string_t;

struct dante_domain_uuid_hex
{
	char str[DANTE_DOMAIN_UUID_HEX_LENGTH];
};

typedef struct dante_domain_uuid_hex dante_domain_uuid_hex_t;

aud_error_t dante_domain_uuid_to_hex(const dante_domain_uuid_t* id_bytes, dante_domain_uuid_hex_t* id_hex);

aud_error_t dante_domain_uuid_from_hex(const char* id_hex, dante_domain_uuid_t* id_bytes);

aud_error_t dante_domain_uuid_to_string(const dante_domain_uuid_t* id_bytes, dante_domain_uuid_string_t* id_string);

aud_error_t dante_domain_uuid_from_string(const char* id_string, dante_domain_uuid_t* id_bytes);

/**
 * Every endpoint (device or controller) in a Dante domain has a unique identifier for
 * The purposes of packet routing. Each endpoint may consist of multiple components.
 * The <endpoint, component> tuple provides a unique identifier for packet routing
 * within a Dante domain.
 */
typedef uint32_t dante_domain_endpoint_id_t;
typedef uint16_t dante_domain_component_id_t;

typedef struct dante_domain_routing_id
{
	dante_domain_endpoint_id_t endpoint_id;
	dante_domain_component_id_t component_id;
} dante_domain_routing_id_t;

#ifdef __cplusplus
}
#endif

#endif /* _DANTE_DOMAINS_H */

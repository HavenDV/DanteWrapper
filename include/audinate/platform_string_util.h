/*
 * Created  : October 2014
 * Author   : Andrew White, Michael Ung
 * Synopsis : Miscellaneous String utils
 *
 * This software is copyright (c) 2004-2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#ifndef _AUD_PLATFORM_STRING_UTIL_H
#define _AUD_PLATFORM_STRING_UTIL_H

#ifndef AUD_PLATFORM
#error platform_string_util.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif

//--------------------
// String handling utilities

/**
 * Tests if the string pointer contains a non-null string
 */
AUD_INLINE aud_bool_t
aud_str_is_non_empty(const char * str)
{
	return str && str[0];
}

/**
 * Tests if the string pointer is null or empty string
 */
AUD_INLINE aud_bool_t
aud_str_is_empty(const char * str)
{
	return ! aud_str_is_non_empty(str);
}


//--------------------

#ifdef __cplusplus
}
#endif

#endif

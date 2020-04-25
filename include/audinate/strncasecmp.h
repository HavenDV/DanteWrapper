/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : strncasecmp implementation
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 * Implementation of strcasecmp for libc that are missing it.
 */

#ifndef _AUDINATE_PLATFORM_STRNCASECMP
#define _AUDINATE_PLATFORM_STRNCASECMP

//----------
// Include

#ifdef __cplusplus
extern "C"
{
#endif

//----------
// Functions

/*
	Case-insensitive string comparison, length bounded
	
	Returns
		0 if strings are a case insensitive match
		<0 if s1 < s2 
		>0 if s1 > s2
 */
int
strncasecmp(const char * s1, const char * s2, size_t n);


//----------

#ifdef __cplusplus
}
#endif

#endif	// _AUDINATE_PLATFORM_STRCASECMP

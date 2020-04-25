/*
 * Created  : October 2014
 * Author   : Andrew White, Michael Ung
 * Synopsis : Macros for manipulating bitmaps
 *
 * This software is copyright (c) 2004-2019 Audinate Pty Ltd and/or its licensors
 */

#ifndef _AUD_PLATFORM_FLAGS_UTIL_H
#define _AUD_PLATFORM_FLAGS_UTIL_H

#ifndef AUD_PLATFORM
#error platform_flags_util.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif

//--------------------
/*
	Macro wrappers for common bitfield / bitflag operations.

	NOTE: arguments may be evaluated more than once. If arguments have
		side-effects, behaviour is undefined.
 */

// Set the listed flag (or flags).  Returns field.
#define AUD_FLAG_SET(field,flag) ((field) |= (flag))

// Clear the listed flag (or flags).  Returns field.
#define AUD_FLAG_CLR(field,flag) ((field) &= ~(flag))

// True if any listed flag is set  (null set => false)
#define AUD_FLAG_ISSET(field,flag) ((field) & (flag))

// True if all listed flags are set (null set => true)
#define AUD_FLAG_ISSET_ALL(field,flag) (((field) & (flag)) == (flag))

// Set the listed flag (or flags) to the given value.  Returns field.
#define AUD_FLAG_SET_VALUE(field,flag,value) \
	((value) ? AUD_FLAG_SET(field,flag) : AUD_FLAG_CLR(field,flag))

// Invert the listed flag (or flags).  Returns field.
#define AUD_FLAG_INVERT(field,flag) ((field) ^= (flag))

// Set the flags covered by the mask to the given flag values.
//	Other flags are unchanged.  Returns field.
//	Note: This is equivalent to AUD_FLAG_SET_VALUE, with value treated as a
//		flag-set rather than a boolean.
//		The two calls are identical when values is 0 or ~0.
#define AUD_FLAG_SET_MASKED(field,mask,flags) \
	((field) = ((field) & ~(mask)) | ((mask) & (flags)))

#define AUD_FLAG_SET_MASKED_FLAGS AUD_FLAG_SET_MASKED
	// synonyms, prefer former in new code

// True if masked flags are set identically
#define AUD_FLAG_ISSET_MASKED(field,mask,flags) \
	(((field) & (mask)) == ((flags) & (mask)))


// As AUD_FLAG_SET / AUD_FLAG_CLR, but only perform operation if required
#define AUD_FLAG_SETIF(field, flag) { if (!AUD_FLAG_ISSET(field,flag)) AUD_FLAG_SET(field,flag); }
#define AUD_FLAG_CLRIF(field, flag) { if (AUD_FLAG_ISSET(field,flag)) AUD_FLAG_CLR(field,flag); }


//----------
// As above, except using indexes rather than flags

// Set the listed flag.  Returns field.
#define AUD_FLAGINDEX_SET(field,idx) AUD_FLAG_SET(field,(1 << (idx)))

// Clear the listed flag.  Returns field.
#define AUD_FLAGINDEX_CLR(field,idx) AUD_FLAG_CLR(field,(1 << (idx)))

// True if listed flag is set
#define AUD_FLAGINDEX_ISSET(field,idx) AUD_FLAG_ISSET(field,(1 << (idx)))

// Set the listed flag to the given value.  Returns field.
#define AUD_FLAGINDEX_SET_VALUE(field,idx,value) \
	AUD_FLAG_SET_VALUE(field,(1 << (idx)),value)

// Invert the listed flag.  Returns field.
#define AUD_FLAGINDEX_INVERT(field,idx) AUD_FLAG_INVERT(field,(1 << (idx)))


//----------
// As AUD_FLAG_SET, but applies a byteswap to flags

// Set the listed flag (or flags).  Returns field.
#define AUD_FLAG_SET_BYTESWAP(field,flag,byteswap_op) ((field) |= byteswap_op(flag))

// Clear the listed flag (or flags).  Returns field.
#define AUD_FLAG_CLR_BYTESWAP(field,flag,byteswap_op) ((field) &= ~byteswap_op(flag))

// True if any listed flag is set  (null set => false)
#define AUD_FLAG_ISSET_BYTESWAP(field,flag,byteswap_op) ((field) & byteswap_op(flag))

// True if all listed flags are set (null set => true)
#define AUD_FLAG_ISSET_ALL_BYTESWAP(field,flag,byteswap_op) (((field) & byteswap_op(flag)) == byteswap_op(flag))

// Set the listed flag (or flags) to the given value.  Returns field.
#define AUD_FLAG_SET_VALUE_BYTESWAP(field,flag,value,byteswap_op) \
	((value) ? AUD_FLAG_SET_BYTESWAP(field,flag,byteswap_op) : AUD_FLAG_CLR_BYTESWAP(field,flag,byteswap_op))


//--------------------

#ifdef __cplusplus
}
#endif

#endif

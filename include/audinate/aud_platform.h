/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Dante API platform header file
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file aud_platform.h
 *
 * This is the root header file for the Audinate platform abstraction.
 *
 * To make it easier to write cross-platform code, Audinate defines standardised
 * types, function wrappers and header include paths across all platforms.
 * The platform abstraction then makes these shared names work across all
 * platforms.
 *
 * This file should not be included directly, include "audinate/dante_api.h"
 * instead.
 */

#ifndef _AUD_PLATFORM_H
#define _AUD_PLATFORM_H

#include "platform_specific.h"
#include "platform_types.h"
#include "platform_error.h"
#include "platform_time.h"
#include "platform_env.h"
#include "platform_util.h"
#include "platform_flags_util.h"
#include "platform_string_util.h"

#if AUD_ENV_HAS_PLATFORM_EXTRA == 1
#include "platform_extra.h"
#endif

#endif // _AUD_PLATFORM_H

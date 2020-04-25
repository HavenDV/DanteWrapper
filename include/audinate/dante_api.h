/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Dante API main header file
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file dante_api.h
 *
 * This is the root header for the Audinate Dante API.
 *
 * Including this header includes the entire Dante API.
 */

#ifndef _DANTE_API_H
#define _DANTE_API_H

/** Used internally */
#define DAPI_FLAT_INCLUDE

#include "aud_platform.h"
#include "audinate/dante_api_version.h"
#include "audinate/dante_api_env.h"
#include "dante/dante_common.h"
#include "dante/dante_runtime.h"
#include "dante/dante_domains.h"
#include "dante/dante_aes67.h"
#include "dante/dante_sdp.h"

#include "dante/dapi_types.h"
#include "dante/domain_handler.h"
#include "dante/dapi.h"

#include "conmon/conmon.h"
#include "conmon/conmon_audinate_messages.h"
#include "conmon/conmon_metering.h"

#if AUD_ENV_HAS_CONMON_METERING_CORE == 1
#include "conmon/conmon_metering_core.h"
#endif

#if DAPI_ENV_HAS_CMM == 1
#include "conmon/cmm.h"
#endif

#include "dante/routing.h"
#include "dante/routing_batch.h"
#include "dante/routing_flows.h"
#include "dante/routing_aes67.h"
#include "dante/routing_manual.h"
#include "dante/routing_error_reporting.h"

#include "dante/browsing_types.h"
#include "dante/browsing_legacy.h"
#include "dante/browsing.h"

#endif

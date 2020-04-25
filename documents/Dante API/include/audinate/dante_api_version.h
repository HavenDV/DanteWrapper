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
 * @file dante_api_version.h
 * Contants for the Dante API Version
 */
#ifndef _DANTE_API_VERSION_H
#define _DANTE_API_VERSION_H

/** The major version number for the overall Dante API */
#define DANTE_API_VERSION_MAJOR  4

/** The minor version number for the overall Dante API */
#define DANTE_API_VERSION_MINOR  2

/** The bugfix version number for the overall Dante API */
#define DANTE_API_VERSION_BUGFIX 3

/**
 * Version number as binary uint32.
 *
 * XX - major version
 * XX - minor version
 * XXXX - bugfix version
 */
#define DANTE_API_VERSION_PACKED ((DANTE_API_VERSION_MAJOR <<24) | (DANTE_API_VERSION_MINOR << 16) | DANTE_API_VERSION_BUGFIX)

#endif


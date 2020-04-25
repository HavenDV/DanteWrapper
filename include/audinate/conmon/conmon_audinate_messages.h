/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Publicly available Audinate vendor-specific messages
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file conmon_audinate_messages.h
 *
 * Types and definitions for Audinate vendor-specific messages used for Dante
 * device control and querying. Dante devices support both control (via Query and Control messages)
 * and monitoring (via Status messages):
 *
 * Status messages provide information about a device; these are sent
 * when there are changes to information contained within that message,
 * or in response to query / control messages.
 *
 * Query messages act as a trigger for Dante devices to send a status message, allowing
 * a device to obtain up-to-date information about the state of devices of interest.
 *
 * Control messages allow configuration of various aspects of a Dante device. Each
 * Audinate control message type only provides new information for those fields for which
 * a value has been explicitly assigned. Unspecified fields will not be altered by the
 * receiver. A control message with no fields set is equivalent to a Query message.
 *
 * Each Audinate message includes the following information:
 * - Version: The Audinate protocol version number
 * - Type: The code indicating the type of Audinate message
 * - Congestion delay: A hint to the device providing a maximum value to use when
 *   adding randomized delay before sending a message (to reduce network congestion).
 *   A value of '0' indicates that the a default delay value should be used.
 *   Note that this value is a hint and may not always be observed.
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */
#ifndef _CONMON_AUDINATE_MESSAGES_H
#define _CONMON_AUDINATE_MESSAGES_H

#include "conmon.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------

/**
 * A constant value for the 'Audinate' vendor id
 */
extern const conmon_vendor_id_t * CONMON_VENDOR_ID_AUDINATE;

/**
 * A message type identifier
 */
typedef uint16_t conmon_audinate_message_type_t;

/**
 * A message version identifier
 */
typedef uint16_t conmon_audinate_message_version_t;

/**
 * Audinate versions.
 */
enum conmon_audinate_message_version
{
	/** Version 0.1.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_1_0 = 0x0100,

	/** Version 0.2.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_2_0 = 0x0200,

	/** Version 0.3.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_3_0 = 0x0300,

	/** Version 0.4.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_4_0 = 0x0400,

	/** Version 0.5.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_5_0 = 0x0500,

	/** Version 0.6.0 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_0 = 0x0600,

	/**
	 * Version 0.6.1. Added support for:
	 * - subdomain names
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_1 = 0x0601,

	/**
	 * Version 0.6.2. Added support for:
	 * - name <-> ID mappings (internal)
	 * - global "interfaces" flags
	 * - switched redundancy configuration / capability
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_2 = 0x0602,

	/**
	 * Unicast delay requests (Clock capability and configuration option)
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_3 = 0x0603,

	/**
	 * STATIC_IP capability
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_4 = 0x0604,

	/**
	 * PROPERTY_CHANGE message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_5 = 0x0605,

	/**
	 * Added capabilities field to manufacturer versions status
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_6 = 0x0606,

	/**
	 * - Added 'device status' bitmask to versions message
	 * - Added 'mute status' bitmasks to clocking message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_7 = 0x0607,

	/**
	 * - Added clock subdomain name to master status message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_8 = 0x0608,

	/**
	 * - Added ROUTING_READY query,
	 * - added channel counts to ROUTING_READY status
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_9 = 0x0609,

	/**
	 * - Added link-local enabled/disable to INTERFACE_CONTROL / INTERFACE_STATUS
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_10 = 0x060A,

	/**
	 * - Added option to enabled/disable multicast ports
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_6_11 = 0x060B,

	/**
	 * - Added HA Remote statistics
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_0 = 0x0700,

	/**
	 * - Added version build information to versions message
	 * - Added version build information to manufacturer versions message
	 * - Added friendly names to manufacturer versions message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_1 = 0x0701,


	/**
	 * - Added switch vlan config message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_2 = 0x0702,

	/**
	 * - Internal update to HA Remote format
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_3 = 0x0703,

	/**
	 * - Add uboot version to versions message
	 * - Add model (product) version to manf. versions message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_4 = 0x0704,

	/**
	 * - Add external wc status flag to clocking status message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_5 = 0x0705,

	/**
	 * - Add HA remote bridge modes
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_6 = 0x0706,

	/**
	 * - Add versions capability flag for audio interface status
	 * - Add support function for inferring missing capabilities in a versions message
	 * - Add clock sync type enumeration to versions message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_7 = 0x0707,

	/**
	 * - Add DANTE_READY message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_8 = 0x0708,

	/**
	 * - Add upgrade status / control v3
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_9 = 0x0709,

	/**
	 * - Add read-only capabilities
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_10 = 0x070A,

	/**
	 * - Add domain name in interface status /control messages
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_11 = 0x070B,

	/**
	 * - Add model name to versions status message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_12 = 0x070C,

	/**
	 * - Added RX_FLOW_ERROR_CHANGE
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_13 = 0x070D,

	/**
	 * - Add CLEAR_CONFIG messages
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_14 = 0x070E,

	/**
	 * - Added pull-up suppression flags
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_15 = 0x0700 + 15,

	/**
	 * - Added device store failure flag
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_16 = 0x0700 + 16,

	/**
	 * - Added subdomain to srate pull control and srate pull up status
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_17 = 0x0700 + 17,

	/**
	 * - Added product version friendly string to manf. versions
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_18 = 0x0700 + 18,

	/**
	 * - Added ifstats clear functionality
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_19 = 0x0700 + 19,

	/**
	 * - Extend routing ready to include active link bitmap
	 * - Add WC_STATE_MISSING
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_20 = 0x0700 + 20,

	/**
	 * - Added CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_UNMUTE_CONTROL
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_21 = 0x0700 + 21,

	/**
	 * - Added CONMON_AUDINATE_MESSAGE_TYPE_GPIO_QUERY / CONMON_AUDINATE_MESSAGE_TYPE_GPIO_STATUS
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_22 = 0x0700 + 22,

	/**
	 * - Added Guppy monitoring message mask (currently for internal use only)
	 * - Added slave_only flag in CLOCKING CONTROL/STATUS message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_23 = 0x0700 + 23,
	
	/**
	 * - Added a new flag to disable DHCP in the CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_CONTROL message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_24 = 0x0700 + 24,

	/**
	 * - Added new capability flags CONMON_AUDINATE_CAPABILITY_WILL_NOT_REBOOT_AFTER_UPGRADE and
	 * - Added new device status flag CONMON_AUDINATE_DEVICE_STATUS_FLAG_SWITCH_PARTITION_ERROR
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_25 = 0x0700 + 25,

	/**
	 * - Added an extra field to the CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_V3_CONTROL message
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_26 = 0x0700 + 26,

	/**
	 * - Added ACCESS_CONTROL_POLICY_CHANGED event
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_27 = 0x0700 + 27,
	
	/**
	* - Added extension2 to the CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_STATUS message
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_28 = 0x0700 + 28,

	/**
	* - Added CONMON_AUDINATE_CAPABILITY_CAN_LOCK capability
	* - Added CONMON_AUDINATE_CAPABILITY_HAS_NO_REMOTE_LOCK_RESET capability
	*/

	CONMON_AUDINATE_MESSAGE_VERSION_0_7_29 = 0x0700 + 29,
	
	/**
	 * - Added CONMON_AUDINATE_CAPABILITY_HAS_DDP capability
	 * - Added CONMON_AUDINATE_CAPABILITY_HAS_PB capability
	 * - Added CONMON_AUDINATE_CAPABILITY_HAS_USER capability
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_30 = 0x0700 + 30,

	/**
	 * - Added per port unicast delay request clocking config and status
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_31 = 0x0700 + 31,

	/**
	* - Added CONMON_AUDINATE_CAPABILITY1_HAS_NO_PTP to capability extension 1
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_32 = 0x0700 + 32,

	/**
	* - Added DDM enrolment config related messages
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_33 = 0x0700 + 33,

	/**
	* - Added DDM enrolled status
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_34 = 0x0700 + 34,

	/**
	* - Added Domain capability flags
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_35 = 0x0700 + 35,

	/**
	* - Added flag "Device Requires Reboot" to Lock status message.
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_36 = 0x0700 + 36,

	/**
	* - Added an offset field to the upgrade v3 message which points to an additional target and corresponding capability flag
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_37 = 0x0700 + 37,

	/**
	* - Added ext3 to the clocking status message, 
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_38 = 0x0700 + 38,

	/**
	* - Added Codec Query & Codec status message types,
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_39 = 0x0700 + 39,

	/**
	* - Added ptp v2 domain number to Clocking Status message
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_40 = 0x0700 + 40,

	/**
	* - Added following capabilities to the versions status message
	* - CONMON_AUDINATE_CAPABILITY1_HAS_MANF_OVERRIDE in capabilities1
	* - CONMON_AUDINATE_CAPABILITY1_CAN_ACCEPT_FQDN_DDM_ENROL_CONFIG in capabilities1
	* - CONMON_AUDINATE_CAPABILITY_DOMAIN__AUDIO_RTP_SUPPORTED in domain capabilities
	*/
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_41 = 0x0700 + 41,

	/**
	 * - Added CONMON_AUDINATE_CAPABILITY1_CAN_SEND_CHANNEL_LABEL_CHANGE capability
	 */
	CONMON_AUDINATE_MESSAGE_VERSION_0_7_42 = 0x0700 + 42,

	/** The current version */
	CONMON_AUDINATE_MESSAGE_VERSION = CONMON_AUDINATE_MESSAGE_VERSION_0_7_42
};


enum conmon_audinate_message_type
{
	/**
	 * Topology change messages are sent when a device thinks that the network topology
	 * MAY have changed. These messages are a guide only: Messages may occasionally be sent
	 * when not change has occurred and not all network changes are guaranteed to be observed
	 * by all devices.
	 *
	 * Format: N/A (no payload)
	 * Channel: broadcast
	 *
	 * @note: In the future, this message may be integrated into the ConMon infrastructure,
	 *   and the information provided via callbacks.
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TOPOLOGY_CHANGE   = 0x0010,

	/**
	 * Interface status messages describe the current state of a device's networks.
	 *
	 * Format: conmon_audinate_interface_status
	 * Channel: status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_STATUS  = 0x0011,

	/**
	 * Interface control messages allow configuration of a Dante device's
	 * network interface addresses
	 *
	 * Format: conmon_audinate_interface_control
	 * Channel: control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_CONTROL   = 0x0013,

	/**
	 * Query a device's interface status.
	 *
	 * Format: Query
	 * Channel: Control
	 *
	 * @deprecated use CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_CONTROL instead
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_INTERFACE_QUERY   = 0x0013,

	/**
	 * A switch vlan status messages describes the current switch VLAN
	 * configuration for devices where this is a configurable option
	 *
	 * Format: conmon_audinate_switch_vlan_status
	 * Channel: status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SWITCH_VLAN_STATUS   = 0x0014,

	/**
	 * A switch vlan status messages describews the current switch VLAN
	 * configuration for devices where this is a configurable option
	 *
	 * Format: conmon_audinate_switch_vlan_control
	 * Channel: control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SWITCH_VLAN_CONTROL   = 0x0015,


	/**
	 * Clocking status messages describe the state of a Dante device's clocking component
	 * and includes information such as clock state, drift, and per-port states.
	 *
	 * Format: conmon_audinate_clocking_status
	 * Channel: status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_STATUS   = 0x0020,

	/**

	* Clocking control messages allow configuration of a Dante device's
	 * clocking component.
	 *
	 * Format: conmon_audinate_clocking_control
	 * Channel: control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_CONTROL  = 0x0021,

	/**
	 * Master status messages are sent in response to MASTER_QUERY
	 * messages. Dante devices that are in the 'Master' state on
	 * one or more connected (ie. LINK_UP) links will send a response describing
	 * the port state for each of their clocking ports.
	 *
	 * Format: conmon_audinate_master_status
	 * Channel: Broadcast
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_MASTER_STATUS     = 0x0022,

	/**
	 * A master query is a network-wide request for any devices that are a clock master
	 * to identify themselves. As this message is transmitted on the broadcast channel
	 * and may trigger ultipole broadcast responses, it should not be called frequently.
	 *
	 * Format: query
	 * Channel: Broadcast
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_MASTER_QUERY      = 0x0023,

	CONMON_AUDINATE_MESSAGE_TYPE_UNICAST_CLOCKING_STATUS = 0x0024,

	CONMON_AUDINATE_MESSAGE_TYPE_UNICAST_CLOCKING_CONTROL = 0x0025,

	/**
	 * @deprecated
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_UNICAST_CLOCKING_QUERY = CONMON_AUDINATE_MESSAGE_TYPE_UNICAST_CLOCKING_CONTROL,

	/**
	 * Response to an 'name_id' command. Used to inform
	 * other devices of the device name of a device with
	 * a given instance id
	 *
	 * Format: conmon_audinate_name_id
	 * Channel: Broadcast
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_NAME_ID_STATUS   = 0x0026,

	/**
	 * Request for a device (or devices) with a given instance id
	 * to provide their dante device name
	 *
	 * Format: conmon_audinate_name_id
	 * Channel: Broadcast
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_NAME_ID_CONTROL   = 0x0027,

	/**
	 * PTP logging status
	 *
	 * Format: conmon_audinate_ptp_logging_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_PTP_LOGGING_STATUS      = 0x0028,

	/**
	 * Query or set PTP logging
	 *
	 * Format: conmon_audinate_ptp_logging_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_PTP_LOGGING_CONTROL     = 0x0029,


	/**
	 * Unmute a device that was started up in 'mute'
	 * Used to clear the boot mute on certain devices
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CLOCKING_UNMUTE_CONTROL = 0x002a,

	/**
	 * Ifstats status messages provide per-interface statistics for a
	 * device, including utilisation rates and error counts.
	 *
	 * Format: conmon_audinate_ifstats_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IFSTATS_STATUS    = 0x0040,

	/**
	 * Query a device's interface statistics
	 *
	 * Format: ifstats_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IFSTATS_CONTROL   = 0x0041,

	/**
	 * @deprecated
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IFSTATS_QUERY     = CONMON_AUDINATE_MESSAGE_TYPE_IFSTATS_CONTROL,

	/**
	 * The IGMP Version information for a device
	 *
	 * Format: conmon_audinate_igmp_vers_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IGMP_VERS_STATUS  = 0x0050,

	/**
	 * Set the IGMP Version for a device
	 *
	 * Format: conmon_audinate_igmp_vers_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IGMP_VERS_CONTROL = 0x0051,

	/**
	 * Dante Version information for a device, such as
	 * Dante software & firmware versions and Dante model id.
	 *
	 * Format: conmon_audinate_versions_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_VERSIONS_STATUS   = 0x0060,

	/**
	 * Query a Dante device's version information
	 *
	 * Format: Query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_VERSIONS_QUERY    = 0x0061,

	/**
	 * Response to an 'identify' command. Used to notify
	 * the transmitter that the command was received and
	 * processed
	 *
	 * Format: empty
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IDENTIFY_STATUS   = 0x0062,

	/**
	 * Identify a Dante device in some device-specific manner.
	 * May not be supported on all dante devices.
	 *
	 * Format: Query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_IDENTIFY_QUERY    = 0x0063,

	/**
	 * Upgrade status messages provide information about the
	 * state of a Dante device upgrade process.
	 *
	 * Format: conmon_audinate_upgrade_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_STATUS    = 0x0070,

	/**
	 * Upgrade config messages set the parameters
	 * to be used by a Dante device upgrade process.
	 *
	 * Format: conmon_audinate_upgrade_control
	 * Channel: Control
	 *
	 * Deprecated: see CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_V3_CONTROL instead
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_CONTROL    = 0x0071,

	/**
	 * Config status messages inform the device that some
	 * aspect of device configuration has changed.
	 *
	 * Format: conmon_audinate_config_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CONFIG_CONTROL     = 0x0073,

	/**
	 * Upgrade config messages set the parameters
	 * to be used by a Dante device upgrade process.
	 *
	 * Format: conmon_audinate_upgrade_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_UPGRADE_V3_CONTROL = 0x0075,

	/**
	 * Clear the config
	 *
	 * Format: conmon_audinate_clear_config_control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CLEAR_CONFIG_CONTROL = 0x0077,

	/**
	 * clear config status messages
	 *
	 * Format: conmon_audinate_clear_config_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CLEAR_CONFIG_STATUS  = 0x0078,


	/**
	 * Sample rate information for a Dante device
	 *
	 * Format: conmon_audinate_srate_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SRATE_STATUS      = 0x0080,

	/**
	 * Query or set sample rate information for a Dante device
	 *
	 * Format: conmon_audinate_srate_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SRATE_CONTROL     = 0x0081,

	/**
	 *  Audio encoding information for a Dante device
	 *
	 * Format: conmon_audinate_enc_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ENC_STATUS      = 0x0082,

	/**
	 * Query or set audio encoding information for a Dante device
	 *
	 * Format: conmon_audinate_enc_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ENC_CONTROL      = 0x0083,


	/**
	 * Sample rate pullup / pullldown for Dante device
	 *
	 * Format: conmon_audinate_srate_pullup_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SRATE_PULLUP_STATUS      = 0x0084,

	/**
	 * Query or set sample rate pullup pulldown for  a Dante device
	 *
	 * Format: conmon_audinate_srate_pullup_control
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SRATE_PULLUP_CONTROL     = 0x0085,


	/**
	 * Audio interface configuration status for Dante device
	 *
	 * Format: conmon_audinate_audio_interface_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_AUDIO_INTERFACE_STATUS      = 0x0086,

	/**
	 * Audio interface configuration query for Dante device
	 *
	 * Format: conmon_audinate_audio_interface_status
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_AUDIO_INTERFACE_QUERY      = 0x0087,

	/**
	 * System reset a Dante device
	 *
	 * Format: conmon_audinate_sys_reset_control
	 * Channel:
	 */

	CONMON_AUDINATE_MESSAGE_TYPE_SYS_RESET         = 0x0090,

	/**
	 * System Reset configuration status for Dante device
	 *
	 * Format: conmon_audinate_sys_reset_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SYS_RESET_STATUS	= 0x0092,

	/**
	 * EDK-specific information for a Dante EDK device
	 *
	 * Format: conmon_audinate_edk_board_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_EDK_BOARD_STATUS  = 0x00a0,

	/**
	 * Set EDK-specific information for a Dante EDK device
	 *
	 * Format: conmon_audinate_edk_board_control
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_EDK_BOARD_CONTROL = 0x00a1,

	/**
	 * The current access control setting for a Dante device
	 * (that is, whether or not remote control is allowed)
	 *
	 * Format: conmon_audinate_access_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ACCESS_STATUS     = 0x00b0,

	/**
	 * Set the current access control setting for a Dante device
	 * (that is, whether or not remote control is allowed)
	 *
	 * Format: conmon_audinate_access_control
	 * Channel: Local
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ACCESS_CONTROL	   = 0x00b1,

	/**
	 * Manufacturer version information for a Dante device
	 *
	 * Format: conmon_audinate_manf_versions_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_MANF_VERSIONS_STATUS	  = 0x00c0,

	/**
	 * Query the manufacturer version information for a Dante device
	 *
	 * Format: Query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_MANF_VERSIONS_QUERY	  = 0x00c1,

	/**
	 * Device LED status
	 *
	 * Format: conmon_audinate_led_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_LED_STATUS = 0x00d0,

	/**
	 * Query the LED status for a Dante device
	 *
	 * Format: Query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_LED_QUERY = 0x00d1,

	/**
	 * Metering parameters for a Dante device
	 *
	 * Format: conmon_audinate_metering_status
	 * Channel: Status
	 *
	 * @note: this message does NOT contains metering. It contains the current settings
	 *   used to generate metering messgages.
	 *
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_METERING_STATUS = 0x00e0,

	/**
	 * Query or configure the metering parameters for a Dante device
	 *
	 * Format: conmon_audinate_metering_control
	 * Channel: Control
	 *
	 * @note: this is NOT a request for metering message but a query or configuration
	 *   of the parameters controlling the audio metering process
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_METERING_CONTROL = 0x00e1,

	/**
	 * Serial port parameters for a Dante device
	 *
	 * Format: conmon_audinate_serial_port_status
	 * Channel: Status
	 *
	 * @note: this message does NOT contains serial data. It contains the current
	 *   serial port settings.
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SERIAL_PORT_STATUS = 0x00F0,

	/**
	 * Query or configure the serial port parameters for a Dante device
	 *
	 * Format: conmon_audinate_serial_port_control
	 * Channel: Control
	 *
	 * @note: this is NOT a request for serial port data but a query or configuration
	 *   of the current serial port settings
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_SERIAL_PORT_CONTROL = 0x00F1,

	/**
	 * Is the Dante device ready to receive routing configuration messages
	 * (via the Routing API?)
	 *
	 * Format: conmon_audinate_routing_ready
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_STATUS = 0x0100,

	/**
	 * Is the Dante device ready to receive routing configuration messages
	 * (via the Routing API?)
	 *
	 * Format: conmon_audinate_routing_ready
	 * Channel: Status
	 *
	 * @deprecated use CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_STATUS instead
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY        = 0x0100,

	/**
	 * TX channel change messages provide asynchronous notification that a device's
	 * transmit channel configuration has changed. Only the id's of changed channels
	 * are provided; detailed information about the channels is available via the
	 * routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TX_CHANNEL_CHANGE = 0x0101,

	/**
	 * RX channel change messages provide asynchronous notification that a device's
	 * receive channel configuration has changed. Only the id's of changed channels
	 * are provided; detailed information about the channels is available via the
	 * routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_CHANNEL_CHANGE = 0x0102,


	/**
	 * TX label change messages provide asynchronous notification that a device's
	 * transmit labels have changed. Only the id's of changed labels
	 * are provided; detailed information about the channels is available via the
	 * routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TX_LABEL_CHANGE   = 0x0103,

	/**
	 * TX flow change messages provide asynchronous notification that a device's
	 * transmit flows have changed. Only the id's of changed flows
	 * are provided; detailed information about the channels is available via the
	 * routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TX_FLOW_CHANGE    = 0x0104,

	/**
	 * RX flow change messages provide asynchronous notification that a device's
	 * receive flows has changed. Only the id's of changed flows
	 * are provided; detailed information about the channels is available via the
	 * routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_FLOW_CHANGE    = 0x0105,

	/**
	 * Property change messages provide asynchronous notification that a device's
	 * routing properties have changed.
	 *
	 * Format: none
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_PROPERTY_CHANGE    = 0x0106,

	/**
	 * RX flow change messages provide asynchronous notification that the error
	 * flags have changed for one or more of the device's rxflows.
	 *
	 * Format: none
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_FLOW_ERROR_CHANGE = 0x0108,


	/**
	 * TX signal reference level change messages provide asynchronous notification
	 * that a device's transmit signal reference level has changed.
	 * The message provides both the identifiers of changed channels and the
	 * new values.
	 *
	 * Format: conmon_audinate_id_value32_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TX_SIGNAL_REFLEVEL_CHANGE = 0x0109,

	/**
	 * RX signal reference level change messages provide asynchronous notification
	 * that a device's receive signal reference level has changed.
	 * The message provides both the identifiers of changed channels and the
	 * new values.
	 *
	 * Format: conmon_audinate_id_value32_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_SIGNAL_REFLEVEL_CHANGE = 0x010a,


	/**
	 * Query the devices 'routing ready' status
	 *
	 * Format: query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_QUERY = 0x010F,

	/**
	 * RX Channel RX error messages provide asynchronous notification that a
	 * device's receive channels have entered or left an RX error state.  The
	 * payload is a list of all RX channels that are currently suffering from
	 * a physical RX error condition.  Absence of a channel implies that it
	 * is correctly receiving data.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_CHANNEL_RX_ERROR = 0x0110,

	/**
	 * Query RX channel RX error status.
	 *
	 * Format: query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_CHANNEL_RX_ERROR_QUERY = 0x0111,

	/**
	 * RX Channel error threshold status message
	 * returns the threshold at which Rx Channel errors are triggered
	 *
	 * Format: conmon_audinate_rx_error_threshold_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_ERROR_THRES_STATUS = 0x0112,

	/**
	 * RX Channel error threshold control message
	 * Configures the threshold at which Rx Channel errors are triggered
	 *
	 * Format: conmon_audinate_rx_error_threshold_status
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_RX_ERROR_THRES_CONTROL = 0x0113,

	/**
	 * Important routing information has changed.
	 *
	 * Format:  none
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_DEVICE_CHANGE = 0x0120,

	/**
	 * All the Dante components are ready to communicate.
	 *
	 * Format:  none
	 * Channel: Local
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_DANTE_READY          = 0x0130,
	/**
	 * Query the Dante ready status.
	 *
	 * Format:  none
	 * Channel: Local
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_DANTE_READY_QUERY    = 0x0131,

	/**
	 * Control message to read / set GPIO pins.
	 *
	 * Format:  conmon_audinate_gpio_control_t
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_GPIO_QUERY           = 0x0140,
	/**
	 * Current state of the GPIO pins
	 *
	 * Format:  conmon_audinate_gpio_status_t
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_GPIO_STATUS          = 0x0141,

	/**
	 * Event on re-connection to MILLAU server
	 *
	 * Format:  none
	 * Channel: Local
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_MILLAU_CONNECTION_EVENT          = 0x0200,

	/**
	 * For a device with fine-grained access control, the local
	 * access control policy has changed in some way. Components enforcing
	 * access control rules should re-evaluate their configurations.
	 * For audinate use only.
	 *
	 * Format:  none
	 * Channel: Local
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_ACCESS_CONTROL_POLICY_CHANGED = 0x0202,

	/**
	 * HARemote status message. This is not a HARemote message,
	 * it is for configuration of a haremote daemon on a Dante device
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_STATUS       = 0x1000,

	/**
	 * @deprecated. Use CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_CONTROL instead
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_QUERY        = 0x1001,

	/**
	 * Query or configure a HARemote daemon on a Dante device
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_CONTROL      = 0x1001,

	/**
	 * HARemote message
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE              = 0x1002,

	/**
	 * HARemote statistics for a Dante device
	 *
	 * Format: conmon_audinate_haremote_stats_status
	 * Channel: Status
	 *
	 * @note: this message does NOT contains Ha Remote messages. It contains statistics for
	 *   HA Remote uart ports
	 *
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_STATS_STATUS = 0x1004,

	/**
	 * Query The devices HA Remote stats
	 *
	 * Format: query
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_HAREMOTE_STATS_QUERY  = 0x1005,

	/**
	* Query or enable/diable AES67 mode for a Dante device
	*
	* Format: conmon_audinate_aes67_control
	* Channel: Control
	*/
	CONMON_AUDINATE_MESSAGE_TYPE_AES67_CONTROL = 0x1006,

	/**
	* Status of AES67 mode (enabled/disabled) for a Dante device
	*
	* Format: conmon_audinate_aes67_control
	* Channel: Status
	*/
	CONMON_AUDINATE_MESSAGE_TYPE_AES67_STATUS = 0x1007,

	/**
	 * Query a device's lock status
	 *
	 * Format: Query
	 * Channel: Control
	 *
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_LOCK_QUERY = 0x1008,

	/**
	 * Device locked / unlocked status
	 *
	 * Format: bool
	 * Channel: Status
	 *
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_LOCK_STATUS = 0x1009,


	/**
	 * Query/control a device's codec parameters
	 *
	 * Format: conmon_audinate_codec_control_t
	 * Channel: Control
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CODEC_CONTROL = 0x100A,

	/**
	 * Status response from device containing codec information
	 *
	 * Format: conmon_audinate_codec_status_t
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_CODEC_STATUS = 0x100B,

	/**
	 * TX change label change messages provide asynchronous
	 * notification that a device's transmit labels have
	 * changed. Only the id's of the channels with changed labels
	 * are provided; detailed information about the channels and
	 * labels is available via the routing API.
	 *
	 * Format: conmon_audinate_id_set_msg
	 * Channel: Status
	 */
	CONMON_AUDINATE_MESSAGE_TYPE_TX_CHANNEL_LABEL_CHANGE = 0x100C,
};

/**
 * How many bytes in the conmon audinate message header?
 */
uint16_t
conmon_audinate_message_get_head_size(void);

/**
 * Get the format version for an audinate message
 *
 * @param aud_msg a conmon message body containing an Audinate vendor-specific message
 */
conmon_audinate_message_version_t
conmon_audinate_message_get_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the message type for an audinate message
 *
 * @param aud_msg a conmon message body containing an Audinate vendor-specific message
 */
conmon_audinate_message_type_t
conmon_audinate_message_get_type
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the recommended congestion delay (in microseconds) for an audinate message
 *
 * @param aud_msg a conmon message body containing an Audinate vendor-specific message
 */
uint32_t
conmon_audinate_message_get_congestion_delay
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise a ConMon message as an Audinate query message
 * with the given type and congestion delay
 *
 * @param aud_msg a conmon message body that will contain the Audinate query message
 * @param type the audinate message type for this query
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_query_message
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_message_type_t type,
	uint32_t congestion_delay_window_us
);

/**
 * Get the size (in bytes) of a query message
 *
 * @param aud_msg a conmon message body containing an Audinate query message
 */
uint16_t
conmon_audinate_query_message_get_size
(
	const conmon_message_body_t * aud_msg
);


/**
	Structure to hold the current and maximum size of a conmon message body.

	Some complex messages are built incrementally, and it is important to know
	the current and maximum message size.

	When passed as a parameter to an 'init' call, the current size will be
	initialised to the message minimum size.  If max is zero, it will be initialised
	to CONMON_MESSAGE_MAX_BODY_SIZE.  If the buffer has a different size, max
	should be set non-zero to the size of the buffer.
 */
typedef struct conmon_message_size_info
{

	/** Maximum conmon nessage size */
	size_t max;  

	/** Current conmon message size */
	size_t curr;
} conmon_message_size_info_t;


//----------------------------------------------------------
// Utility types
//----------------------------------------------------------

/**
	Structure to represent a uint32_t set of flags.

	Contains a pair of uint32_t fields representing 32 bit-flags.
	The first field is a mask of valid flags.
	The second field is the flags themselves.
 */
typedef struct conmon_audinate_message_flagset
{
	/** Flag masks */
	uint32_t mask;

	/** Flag values */
	uint32_t value;
} conmon_audinate_message_flagset_t;


/**
	An empty flag-set
 */
static const conmon_audinate_message_flagset_t
	CONMON_AUDINATE_MESSAGE_FLAGSET__EMPTY = { 0, 0 };


//----------------------------------------------------------
// Interface status / Address change notification
//----------------------------------------------------------

enum conmon_interface_flagindex
{
	/** A flag index indicating that the link is up */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_UP     = 0,

	/** A flag index indicating that the interface's ip address has been statically configured */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_STATIC = 1,

	/** For internal use only */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_RESERVED = 2,

	/** A flag index indicating that link-local addressing is DISABLED on the interface. Note the negation */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_LINK_LOCAL_DISABLED = 3,

	/** A flag index indicating that link-local addressing is DELAYED on the interface */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_LINK_LOCAL_DELAYED = 4,

	/** A flag index indicating that executing DHCP is DISABLED on the interface. Note the negation */
	CONMON_AUDINATE_INTERFACE_FLAGINDEX_DHCP_DISABLED = 5
};

enum conmon_interface_flags
{
	/** A flag indicating that the link is up */
	CONMON_AUDINATE_INTERFACE_FLAG_UP     = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_UP),

	/** A flag indicating that the interface's ip address has been statically configured */
	CONMON_AUDINATE_INTERFACE_FLAG_STATIC = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_STATIC),

	/** For internal use only */
	CONMON_AUDINATE_INTERFACE_FLAG_RESERVED = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_RESERVED),

	/** A flag indicating that link-local addressing is DISABLED on the interface. Note the negation */
	CONMON_AUDINATE_INTERFACE_FLAG_LINK_LOCAL_DISABLED = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_LINK_LOCAL_DISABLED),

	/** A flag indicating that link-local addressing is DISABLED on the interface. Note the negation */
	CONMON_AUDINATE_INTERFACE_FLAG_LINK_LOCAL_DELAYED = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_LINK_LOCAL_DELAYED),

	/** A flag indicating that executing DHCP is DISABLED on the interface. Note the negation */
	CONMON_AUDINATE_INTERFACE_FLAG_DHCP_DISABLED = (1 << CONMON_AUDINATE_INTERFACE_FLAGINDEX_DHCP_DISABLED)
};

enum
{
	/**
	 * A flag index indicating whether the switched redundancy is currently set to
	 * 'redundant' mode or 'switched' (non-redundant) mode
	 */
	CONMON_AUDINATE_INTERFACES_FLAGINDEX_SWITCH_REDUNDANCY        = 0,
	CONMON_AUDINATE_INTERFACES_FLAGINDEX_SWITCH_REDUNDANCY_REBOOT = 1
};

enum conmon_interface_switch_redundancy
{
	/** A flag indicating the redundancy mode - currently and on reboot*/
	CONMON_AUDINATE_INTERFACES_SWITCH_REDUNDANCY        = (1 << CONMON_AUDINATE_INTERFACES_FLAGINDEX_SWITCH_REDUNDANCY),
	CONMON_AUDINATE_INTERFACES_SWITCH_REDUNDANCY_REBOOT = (1 << CONMON_AUDINATE_INTERFACES_FLAGINDEX_SWITCH_REDUNDANCY_REBOOT)
};

/**
 * Flags for a given interface or port
 */
typedef uint16_t conmon_audinate_interface_flags_t;

/**
 * Flags across all interfaces
 */
typedef uint16_t conmon_audinate_interfaces_flags_t;

enum conmon_interface_mode
{
	/** In direct mode each logical dante interface connects directly to a single physical port.
	    There is exactly one link per dante interface, namely the physical link */
	CONMON_AUDINATE_INTERFACE_MODE_DIRECT = 0,

	/** In switched mode each logical dante interface is connected via an internal switch to
	  one or more physical ports. Each Dante Interface has an internal link, one or more
	  physical links and zero or more other, non-dante links */
	CONMON_AUDINATE_INTERFACE_MODE_SWITCHED = 1
};

/**
 * What mode of operation are the dante interfaces using?
 */
typedef uint16_t conmon_audinate_interface_mode_t;

/**
 * An opaque handle for per-interface network information
 */
typedef struct conmon_audinate_interface conmon_audinate_interface_t;

/**
 * What are the device-wide interface flags?
 *
 * @param aud_msg the message containing the interface information
 */
conmon_audinate_interface_flags_t
conmon_audinate_interface_status_get_flags
(
	const conmon_message_body_t * aud_msg
);

/**
 * How many interfaces in this message?
 *
 * @param aud_msg a conmon message body containing an Audinate interface_status message
 */
uint16_t
conmon_audinate_interface_status_num_interfaces
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is this device's interface mode?
 *
 * @param aud_msg a conmon message body containing an Audinate interface_status message
 */
conmon_audinate_interface_mode_t
conmon_audinate_interface_status_get_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get a handle to the interface status information at the given index in the message
 *
 * @param aud_msg a conmon message body containing an Audinate interface_status message
 * @param index the index (in the message) of the interface
 */
const conmon_audinate_interface_t *
conmon_audinate_interface_status_interface_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
 * What is the link speed for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 */
uint32_t
conmon_audinate_interface_get_link_speed
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);


/**
 * What are the flags for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 */
conmon_audinate_interface_flags_t
conmon_audinate_interface_get_flags
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the mac address for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 */
const uint8_t *
conmon_audinate_interface_get_mac_address
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the ip address for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's IP address in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_ip_address
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the netmask for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's IP netmask in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_netmask
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the dns server for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's DNS server in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_dns_server
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the gateway for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's gateway address in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_gateway
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the domain name for this interface?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 * @param size size of the aud msg
 */

const char *
conmon_audinate_interface_status_get_domain_name
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg,
	size_t size
);

/**
 * Has this interface been reconfigured for the next reboot?
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 */
aud_bool_t
conmon_audinate_interface_is_reboot_configured
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What flags are configured for this interface for the next reboot?
 * returns 0 if no reboot configuration is available
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 */
conmon_audinate_interface_flags_t
conmon_audinate_interface_get_reboot_flags
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What will the ip address be for this interface after the next reboot?
 * returns 0 if no reboot configuration is available or if configured
 * with dynamic addressing
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's post-reboot IP address in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_reboot_ip_address
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What will the netmask be for this interface after the next reboot?
 * returns 0 if no reboot configuration is available or if configured
 * with dynamic addressing
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's post-reboot IP netmask in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_reboot_netmask
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What will the dns server be for this interface after the next reboot?
 * returns 0 if no reboot configuration is available or if configured
 * with dynamic addressing
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's post-reboot DNS server address in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_reboot_dns_server
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);

/**
 * What will the gateway be for this interface after the next reboot?
 * returns 0 if no reboot configuration is available or if configured
 * with dynamic addressing
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 *
 * @return the interface's post-reboot gateway address in NETWORK order
 */
uint32_t
conmon_audinate_interface_get_reboot_gateway
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg
);
/**
 * What will be the domain name for this interface after the next reboot?
 * returns NULL if no reboot configuration is available or if configured
 * with dynamic addressing
 *
 * @param iface the interface
 * @param aud_msg the message containing the interface information
 * @param size the size of aud message containing the interface information
 */

const char *
conmon_audinate_interface_status_get_reboot_domain_name
(
	const conmon_audinate_interface_t * iface,
	const conmon_message_body_t * aud_msg,
	size_t size
);
/**
 * Initialise an interface control message
 *
 * @param aud_msg the message containing the interface information
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_interface_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Specify whether or not link local will be available when dynamic addressins is used
 *
 * WARNING: This function will erase existing 'interface control data' in the message.
 *
 * @param aud_msg the message containing the interface information
 * @param network_index the index of the interface to be configured.
 *     0 - for primary interface
 *     1 - for secondary interface
 */
void
conmon_audinate_interface_control_set_link_local_enabled
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	aud_bool_t enabled
);
/**
 * Specify whether or not link local will be delayed when dynamic addressins is used
 *
 * WARNING: This function will erase existing 'interface control data' in the message.
 *
 * @param aud_msg the message containing the interface information
 * @param network_index the index of the interface to be configured.
 *     0 - for primary interface
 *     1 - for secondary interface
 * @param enabled to specify true or false as to whether ll will be delayed or not
 */

void
conmon_audinate_interface_control_set_link_local_delayed
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	aud_bool_t enabled
);

/**
 * Specify that this control message will configure an interface
 * with dynamic addressing.
 *
 * WARNING: This function will erase existing 'interface control data' in the message.
 *
 * @param aud_msg the message containing the interface information
 * @param network_index the index of the interface to be configured.
 *     0 - for primary interface
 *     1 - for secondary interface
 */
void
conmon_audinate_interface_control_set_interface_address_dynamic
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index
);

/**
 * Specify that this control message will configure an interface
 * with static addressing, using the fields given.
 *
 * WARNING: This function will erase existing 'interface control data' in the message.
 *
 * @param aud_msg the message containing the interface information
 * @param network_index the index of the interface to be configured.
 *     0 - for primary interface
 *     1 - for secondary interface
 * @param ip_address the ip address in NETWORK order
 * @param netmask the netmask in NETWORK order
 * @param dns_server the dns_server in NETWORK order
 * @param gateway the gateway in NETWORK order
 */
void
conmon_audinate_interface_control_set_interface_address_static
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	uint32_t ip_address,
	uint32_t netmask,
	uint32_t dns_server,
	uint32_t gateway
);

/**
 * Specify that this control message will configure an interface
 * with domain name (in static addressing), using the fields given.
 *
 * This function will only return SUCCESS if 
 * conmon_audinate_interface_control_set_interface_address_static()
 * is already called to initialize the message. This function matches
 * 'network_index' provided here to the one provided in aforementioned fuction.
 *
 * @param aud_msg the message containing the interface information
 * @param size of the message
 * @param network_index the index of the interface to be configured
 *     0 - for primary interface
 *     1 - for secondary interface
 * @param domain name to be associated with the interface
 */

aud_error_t
conmon_audinate_interface_control_set_interface_domain_name
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	uint16_t network_index,
	const char * domain_name
 );


/**
 * Set the redundancy mode to use on reboot: redundant=true,
 * non-redundant = false
 *
 * @param aud_msg a conmon message body containing an interface control message
 * @param redundant a flag specifying whether or not the device should enable redundancy
 */
void
conmon_audinate_interface_control_set_switch_redundancy
(
	const conmon_message_body_t * aud_msg,
	aud_bool_t redundant
);

/**
* Specify whether or not to execute DHCP.
*
* WARNING: This function will erase existing 'interface control data' in the message.
*
* @param aud_msg the message containing the interface information
* @param network_index the index of the interface to be configured.
*     0 - for primary interface
*     1 - for secondary interface
* @param enabled to specify true or false as to whether exection of DHCP will be enabled or not
*/
void
conmon_audinate_interface_control_set_dhcp_enabled
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	aud_bool_t enabled
);

/**
 * Get the size (in bytes) of the interface control message
 *
 * @param aud_msg A conmon message body containing an interface control message
 */
uint16_t
conmon_audinate_interface_control_get_size
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the domain name of the interface control message
 *
 * @param aud_msg A conmon message body containing an interface control message
 * @param network_index the index of the interface to be configured
 *     0 - for primary interface
 *     1 - for secondary interface
 * @param size of the message
 */
const char *
conmon_audinate_interface_control_get_interface_domain_name
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	size_t size
 );
//----------------------------------------------------------
// ID set messages
//----------------------------------------------------------

/**
 * An type for ids in conmon audinate messages, especially routing elements
 */
typedef uint16_t conmon_audinate_id_t;

/**
 * An element in an id set is simply a byte containing the next 8 bits
 */
typedef uint8_t conmon_audinate_id_set_elem_t;

/**
 * How many elements (bytes) are there in this message?
 *
 * @param aud_msg a conmon message body containing an Audinate id set message
 */
uint16_t
conmon_audinate_id_set_num_elements
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the index'th element of this message
 *
 * @param aud_msg a conmon message body containing an Audinate id set message
 * @param index the element index
 */
uint8_t
conmon_audinate_id_set_element_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

//----------------------------------------------------------
// ID value32 messages
//----------------------------------------------------------

/**
 * How many elements are there in this message?
 *
 * @param aud_msg a conmon message body containing an Audinate id value32 message
 */
uint16_t
conmon_audinate_id_value32_num_elements
(
	const conmon_message_body_t * aud_msg
);

/**
 * Structure to hold the contents of a value32 element
 */
typedef struct conmon_audinate_id_value32
{
	conmon_audinate_id_t _from, _to;
		// range of identifiers
	uint32_t value;
} conmon_audinate_id_value32_t;

/**
 * Get the index'th element of this message
 *
 * @param aud_msg a conmon message body containing an Audinate id value32 message
 * @param index the element index
 * @param value structure to store resulting value
 *
 * @return AUD_SUCCESS on success, error code on failure
 */
aud_error_t
conmon_audinate_id_value32_element_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index,
	conmon_audinate_id_value32_t * value
);


AUD_INLINE int16_t
conmon_audinate_id_value32_to_int16
(
	conmon_audinate_id_value32_t * value
)
{
	return (int16_t) (value->value & 0xFFFF);
}


//----------------------------------------------------------
// Boolean messages
//----------------------------------------------------------

/**
 * What is the boolean value of this message?
 *
 * @param aud_msg a conmon message body containing an Audinate boolean message
 */
aud_bool_t
conmon_audinate_bool_msg_get_value
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Routing Ready messages
//----------------------------------------------------------

/**
 * Is the device ready to process audio routing configuration messages?
 *
 * @param aud_msg a conmon message body containing an Audinate routing ready message
 */
aud_bool_t
conmon_audinate_routing_ready_status_is_ready
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_routing_ready_status_num_txchannels
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_routing_ready_status_num_rxchannels
(
	const conmon_message_body_t * aud_msg
);

uint8_t
conmon_audinate_routing_ready_status_link
(
	const conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// Clocking control / status messages
//----------------------------------------------------------

enum conmon_port_state
{
	CONMON_AUDINATE_PORT_STATE_STARTUP,
	CONMON_AUDINATE_PORT_STATE_INITIALIZING,
	CONMON_AUDINATE_PORT_STATE_FAULTY,
	CONMON_AUDINATE_PORT_STATE_DISABLED,
	CONMON_AUDINATE_PORT_STATE_LISTENING,
	CONMON_AUDINATE_PORT_STATE_PRE_MASTER,
	CONMON_AUDINATE_PORT_STATE_MASTER,
	CONMON_AUDINATE_PORT_STATE_PASSIVE,
	CONMON_AUDINATE_PORT_STATE_UNCALIBRATED,
	CONMON_AUDINATE_PORT_STATE_SLAVE,
	CONMON_AUDINATE_PORT_STATE_PASSIVE_LISTENING,
	CONMON_AUDINATE_NUM_PORT_STATES
};

enum conmon_clock_state
{
	CONMON_AUDINATE_CLOCK_STATE_NONE,           // inactive, not being used eg. PTP_FAULTY / PTP_INITIALISING
	CONMON_AUDINATE_CLOCK_STATE_PASSIVE,        // inactive but might be read eg PTP_LISTENING
	CONMON_AUDINATE_CLOCK_STATE_UNDISCIPLINED,  // active but not disciplined, eg. a PTP MASTER
	CONMON_AUDINATE_CLOCK_STATE_DISCIPLINED,    // active and disciplined
	CONMON_AUDINATE_NUM_CLOCK_STATES
};

enum conmon_servo_state
{
	CONMON_AUDINATE_SERVO_STATE_FAULTY,
	CONMON_AUDINATE_SERVO_STATE_RESET,	 // servo has been reset
	CONMON_AUDINATE_SERVO_STATE_SYNCING, // servo is "locking"
	CONMON_AUDINATE_SERVO_STATE_SYNC,	 // servo is "locked"
	CONMON_AUDINATE_SERVO_STATE_UNKNOWN,
	CONMON_AUDINATE_SERVO_STATE_DELAY_RESET,
	CONMON_AUDINATE_SERVO_STATE_NONE,
	CONMON_AUDINATE_NUM_SERVO_STATES
};

enum conmon_clock_source
{
	CONMON_AUDINATE_CLOCK_SOURCE_INTERNAL = 0x0000, // The device's local clock is the clock source
	CONMON_AUDINATE_CLOCK_SOURCE_BNC = 0x0001,
	CONMON_AUDINATE_CLOCK_SOURCE_AES = 0x0002,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_1 = 0x0003,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_2 = 0x0004,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_3 = 0x0005,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_4 = 0x0006,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_5 = 0x0007,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_6 = 0x0008,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_7 = 0x0009,
	CONMON_AUDINATE_CLOCK_SOURCE_AES_8 = 0x000a,
	CONMON_AUDINATE_CLOCK_SOURCE_INVALID = 0x000b,
	CONMON_AUDINATE_NUM_CLOCK_SOURCES
};

enum
{
	CONMON_AUDINATE_CLOCK_SYNCMODE_DEFAULT = 0x0000,
	CONMON_AUDINATE_NUM_CLOCK_SYNCMODES
};

#define CONMON_AUDINATE_NUM_CLOCK_CAPABILITIES 10

enum conmon_clock_capability_flag
{
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_SUBDOMAINS            = 0x0001, // does the device support switching subdomains by index? (deprecated)
	//CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_PULL_BASE           = 0x0002, // does the device support pulling its base rate up or down? (not yet implemented)
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_SUBDOMAIN_NAMES       = 0x0004, // does the device support switching subdomains by name?
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_UNICAST_DELAY_REQESTS = 0x0008,  // does the device support sending/receiving ptp delay requests via unicast?
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_PULLUPS_HAVE_SUBDOMAIN= 0x0010,	// does the device support having subdomains in srate status message?
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_SLAVE_ONLY_DEVICE	    = 0x0020,	// does the device support slave only mode?
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_PTP_VERSION_1         = 0x0040,	// does the device support IEEE 1588-2002 (Version 1) ports? //NOT In Use
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_PTP_VERSION_2         = 0x0080,	// does the device support IEEE 1588-2008 (Version 2) ports?
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_DISALLOW_PREFERRED    = 0x0100,	//does the device do not allow preferred ptp Masterness
	CONMON_AUDINATE_CLOCK_CAPABILITY_FLAG_PER_PORT_UDELAY_REQUESTS = 0x0200	//does the device support per-port sending/receiving delay requests via unicast?
};

enum conmon_clock_mute_index
{
	CONMON_AUDINATE_CLOCK_MUTE_INDEX_SYNC,
	CONMON_AUDINATE_CLOCK_MUTE_INDEX_EXT_CLK,
	CONMON_AUDINATE_CLOCK_MUTE_INDEX_INT_CLK,
	CONMON_AUDINATE_CLOCK_MUTE_INDEX_PTP,
	CONMON_AUDINATE_CLOCK_MUTE_INDEX_USER,
	CONMON_AUDINATE_NUM_CLOCK_MUTES
};

enum conmon_clock_mute_flag
{
	CONMON_AUDINATE_CLOCK_MUTE_FLAG_SYNC = (1 << CONMON_AUDINATE_CLOCK_MUTE_INDEX_SYNC),
	CONMON_AUDINATE_CLOCK_MUTE_FLAG_EXT_CLK = (1 << CONMON_AUDINATE_CLOCK_MUTE_INDEX_EXT_CLK),	// external clock PLL is unlocked
	CONMON_AUDINATE_CLOCK_MUTE_FLAG_INT_CLK = (1 << CONMON_AUDINATE_CLOCK_MUTE_INDEX_INT_CLK),	// internal clock PLL is unlocked
	CONMON_AUDINATE_CLOCK_MUTE_FLAG_PTP = (1 << CONMON_AUDINATE_CLOCK_MUTE_INDEX_PTP),		// PTP has muted device
	CONMON_AUDINATE_CLOCK_MUTE_FLAG_USER = (1 << CONMON_AUDINATE_CLOCK_MUTE_INDEX_USER)		// Device is muted by user control
};

enum conmon_external_wc_state
{
	CONMON_AUDINATE_EXTERNAL_WC_STATE_UNKNOWN,
	CONMON_AUDINATE_EXTERNAL_WC_STATE_NONE,
	CONMON_AUDINATE_EXTERNAL_WC_STATE_INVALID,
	CONMON_AUDINATE_EXTERNAL_WC_STATE_VALID,
	CONMON_AUDINATE_EXTERNAL_WC_STATE_MISSING,
	CONMON_AUDINATE_NUM_EXTERNAL_WC_STATES
};

enum conmon_port_version
{
	CONMON_AUDINATE_PORT_VERSION_UNKNOWN,
	CONMON_AUDINATE_PORT_VERSION_V1,	//PTPv1 IEEE-2002
	CONMON_AUDINATE_PORT_VERSION_V2,	//PTPv2 IEEE-2008
	CONMON_AUDINATE_NUM_PORT_VERSIONS
};

enum conmon_port_path_type
{
	CONMON_AUDINATE_PORT_PATH_TYPE_UNKNOWN,
	CONMON_AUDINATE_PORT_PATH_TYPE_MULTICAST,
	CONMON_AUDINATE_PORT_PATH_TYPE_UNICAST,
	CONMON_AUDINATE_NUM_PORT_PATH_TYPES
};

enum conmon_transport_layer
{
	CONMON_AUDINATE_TRANSPORT_LAYER_UNKNOWN,
	CONMON_AUDINATE_TRANSPORT_LAYER_2,
	CONMON_AUDINATE_TRANSPORT_LAYER_3,
	CONMON_AUDINATE_NUM_TRANSPORT_LAYERS
};

/**
 * This enumeration is deprecated. Use per flag accessor function.
 */
enum conmon_port_index
{
	CONMON_AUDINATE_PORT_INDEX_USER_DISABLED,
	CONMON_AUDINATE_PORT_INDEX_LINK_DOWN,
	CONMON_AUDINATE_PORT_NUM_PORT_FLAGS,
};

/**
 * These flags are deprecated. Use per flag accessor function.
 */
enum conmon_port_flag
{
	CONMON_AUDINATE_PORT_FLAG_USER_DISABLED = (1 << CONMON_AUDINATE_PORT_INDEX_USER_DISABLED),
	CONMON_AUDINATE_PORT_FLAG_LINK_DOWN = (1 << CONMON_AUDINATE_PORT_INDEX_LINK_DOWN),
};

enum conmon_port_link_down
{
	CONMON_AUDINATE_PORT_LINK_DOWN_UNKNOWN,
	CONMON_AUDINATE_PORT_LINK_DOWN,
	CONMON_AUDINATE_PORT_LINK_NOT_DOWN,
	CONMON_AUDINATE_PORT_NUM_LINK_DOWN
};

enum conmon_port_user_disabled
{
	CONMON_AUDINATE_PORT_USER_DISABLED_UNKNOWN,
	CONMON_AUDINATE_PORT_USER_DISABLED,
	CONMON_AUDINATE_PORT_USER_NOT_DISABLED,
	CONMON_AUDINATE_PORT_NUM_USER_DISABLED
};

enum conmon_port_unicast_delay_request
{
	CONMON_AUDINATE_PORT_UDELAY_REQUEST_UNKNOWN,
	CONMON_AUDINATE_PORT_UDELAY_REQUEST_DISABLED,
	CONMON_AUDINATE_PORT_UDELAY_REQUEST_ENABLED,
	CONMON_AUDINATE_PORT_NUM_UDELAY_REQUESTS
};

/**
 * A type representation for the various possible states of a Dante device port (ie. interface)
 */
typedef uint16_t conmon_audinate_port_state_t;

/**
 * A type representation for the various possible states of a Dante device clock
 */
typedef uint16_t conmon_audinate_clock_state_t;

/**
 * A type representation for the various possible states of a Dante device servo
 */
typedef uint16_t conmon_audinate_servo_state_t;

/**
 * A type representation for the various possible sources for a Dante device clock
 */
typedef uint16_t conmon_audinate_clock_source_t;

/**
 * A type representation for the various possible subdomain indexes for a Dante device clock
 */
typedef uint16_t conmon_audinate_clock_subdomain_t;

/**
 * A type representation for the various possible external word clock states for a Dante device.
 */
typedef uint16_t conmon_audinate_ext_wc_state_t;

/**
 * The maximum length (including a trailing zero) of a clock subdomain name
 */
#define CONMON_AUDINATE_CLOCK_SUBDOMAIN_NAME_LENGTH 16

/**
 * A type representation for the various possible capabilities for a Dante device clock
 *
 * @since Audinate message version 0.4.0
 */
typedef uint16_t conmon_audinate_clock_capabilities_t;

/**
 * An opaque reference to status information for a particular port
 * deprecated since version 0.7.26
 */
typedef struct conmon_audinate_port_status conmon_audinate_port_status_t;

/**
* An opaque reference to status information for a particular port
*/
typedef struct conmon_audinate_port_status_ext2 conmon_audinate_port_status_ext2_t;

/**
* A type representation for ptp port's version
*/
typedef uint8_t conmon_audinate_port_version_t;

/**
 * A type representation for possible ptp port's path type (unicast or multicast)
 */
typedef uint8_t conmon_audinate_port_path_type_t;

/**
* A type representation for possible ptp port's transport layer (2 or 3)
*/
typedef uint8_t conmon_audinate_port_transport_layer_t;

/**
* A type representation for possible ptp port's unicast delay request status
*/
typedef uint8_t conmon_audinate_port_udelay_request_t;

/**
* A type representation for possible ptp port's link down flag
*/
typedef uint8_t conmon_audinate_port_link_down_t;

/**
* A type representation for possible ptp port's user disabled flag
*/
typedef uint8_t conmon_audinate_port_user_disabled_t;

/**
* An opaque reference to status information for a particular port
*/
typedef struct conmon_audinate_port_status_ext3 conmon_audinate_port_status_ext3_t;

/**
 * The length of an Audinate clock ID
 */
#define CONMON_AUDINATE_CLOCK_UUID_LENGTH 6

/**
 * IEEE1588 devices are identified by a 6-byte UUID
 */
typedef struct
{
	  /** the UUID itself */
	uint8_t data[CONMON_AUDINATE_CLOCK_UUID_LENGTH];
	  /** A padding field to force force 32-bit alignment */
	uint16_t padding;
} conmon_audinate_clock_uuid_t;

/**
 * Get a string representation of the given port state
 *
 * @param port_state the port state
 */
const char *
conmon_audinate_port_state_string
(
	conmon_audinate_port_state_t port_state
);

/**
 * Get a string representation of the given clock state
 *
 * @param clock_state the clock state
 */
const char *
conmon_audinate_clock_state_string
(
	conmon_audinate_clock_state_t clock_state
);


/**
 * Get a string representation of the given servo state
 *
 * @param servo_state the servo state
 */
const char *
conmon_audinate_servo_state_string
(
	conmon_audinate_servo_state_t servo_state
);


/**
 * Get a string representation of the given port state
 *
 * @param clock_source the clock source
 */
const char *
conmon_audinate_clock_source_string
(
	conmon_audinate_clock_source_t clock_source
);

/**
 * What is the Dante device's servo state?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
conmon_audinate_servo_state_t
conmon_audinate_clocking_status_get_servo_state
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's clock state?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
conmon_audinate_clock_state_t
conmon_audinate_clocking_status_get_clock_state
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's clock source?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
conmon_audinate_clock_source_t
conmon_audinate_clocking_status_get_clock_source
(
	const conmon_message_body_t * aud_msg
);

/**
 * Is the Dante device's clock a preferred master?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
aud_bool_t
conmon_audinate_clocking_status_is_clock_preferred
(
	const conmon_message_body_t * aud_msg
);

/**
 * Deprecated from Version 0.7.31.
 * Unicast delay requests are no longer Dante device specific. They are now ptp port specific.
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
aud_bool_t
conmon_audinate_clocking_status_get_unicast_delay_requests
(
	const conmon_message_body_t * aud_msg
);

aud_bool_t
conmon_audinate_clocking_status_get_multicast_ports_enabled
(
	const conmon_message_body_t * aud_msg
);


aud_bool_t
conmon_audinate_clocking_status_get_slave_only_enabled
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's clock stratum?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
uint8_t
conmon_audinate_clocking_status_get_clock_stratum
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's current clock drift (in parts per billion)?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
int32_t
conmon_audinate_clocking_status_get_drift
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's maximum clock drift (in parts per billion)
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 *
 * @since Audinate message version 0.4.0
 */
int32_t
conmon_audinate_clocking_status_get_max_drift
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get this device's UUID
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
const conmon_audinate_clock_uuid_t *
conmon_audinate_clocking_status_get_uuid
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get this device's grandmaster
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
const conmon_audinate_clock_uuid_t *
conmon_audinate_clocking_status_get_grandmaster_uuid
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the master for the given port
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
const conmon_audinate_clock_uuid_t *
conmon_audinate_clocking_status_get_master_uuid
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the Dante device's clock capabilities?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 *
 * @since Audinate message version 0.4.0
 */
conmon_audinate_clock_capabilities_t
conmon_audinate_clocking_status_get_capabilities
(
	const conmon_message_body_t * aud_msg
);

/**
* What is the index of the PTP subdomain this device is using?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 *
 * @since Audinate message version 0.4.0
 *
 * @deprecated use conmon_audinate_clocking_status_get_subdomain_name instead
 */
conmon_audinate_clock_subdomain_t
conmon_audinate_clocking_status_get_subdomain
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the index of the PTP subdomain this device is using?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 *
 * @since Audinate message version 0.4.0
 *
 * @deprecated use conmon_audinate_clocking_status_get_subdomain_name instead
 */
conmon_audinate_clock_subdomain_t
conmon_audinate_clocking_status_get_subdomain_index
(
	const conmon_message_body_t * aud_msg
);


/**
 * What is the name of the PTP subdomain index this device is using?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 *
 * @since Audinate message version 0.6.1
 */
const char *
conmon_audinate_clocking_status_get_subdomain_name
(
	const conmon_message_body_t * aud_msg
);

/**
 * List the reason(s) why the device is muted
 *
 * @since Audinate message version 0.6.7
 */
uint16_t
conmon_audinate_clocking_status_get_mute_flags
(
	const conmon_message_body_t * aud_msg
);

/**
 * What is the state of external wc if there is any
 *
 * @since Audinate message version 0.7.5
 */
uint16_t
conmon_audinate_clocking_status_get_ext_wc_state
(
	const conmon_message_body_t * aud_msg
);

/**
 * How many v1 multicast clock ports on this Dante device?
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 */
uint16_t
conmon_audinate_clocking_status_num_ports
(
	const conmon_message_body_t * aud_msg
);

/**
* How many total clock ports (inclucing unicast/multicast/version1/version2) on this Dante device?
* * @since Audinate message version 0.7.28
* @param aud_msg A conmon message body containing an Audinate clocking status message
*/

uint16_t
conmon_audinate_clocking_status_total_num_ports
(
const conmon_message_body_t * aud_msg
);

/**
 * Get the port status information for a given port
 *
 * @param aud_msg A conmon message body containing an Audinate clocking status message
 * @param index the index (in the message) of the port
 */
const conmon_audinate_port_status_t *
conmon_audinate_clocking_status_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
 * Get the port state for the given port
 *
 * @param port_status the port information
 * @param aud_msg A conmon message body containing the port status information
 */
conmon_audinate_port_state_t
conmon_audinate_port_status_get_port_state
(
	const conmon_audinate_port_status_t * port_status,
	const conmon_message_body_t * aud_msg
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port state for a give port index.
*/

const conmon_audinate_port_status_ext2_t *
conmon_audinate_port_status_ext2_get_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port state for a give port id.
*/

const conmon_audinate_port_status_ext2_t *
conmon_audinate_port_status_ext2_get_port_at_port_id
(
	const conmon_message_body_t * aud_msg,
	uint16_t port_id
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port ptp status.
*/
conmon_audinate_port_state_t
conmon_audinate_clocking_status_get_port_state
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get v2 port ptp status.
*/
conmon_audinate_port_state_t
conmon_audinate_clocking_status_get_v2_port_state
(
	const conmon_message_body_t * aud_msg
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port path type.
*/
conmon_audinate_port_path_type_t
conmon_audinate_clocking_status_get_port_path_type
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port transport layer type.
*/
conmon_audinate_port_transport_layer_t
conmon_audinate_clocking_status_get_port_transport_layer
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);
    
/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port interface index.
*/
uint32_t
conmon_audinate_clocking_status_get_port_interface_index
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port ptp version.
*/
uint8_t
conmon_audinate_clocking_status_get_port_ptp_version
(
const conmon_message_body_t * aud_msg,
const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.28
* Get port id for a given port status struct.
*/
uint16_t
conmon_audinate_clocking_status_get_port_id
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* @since Audinate message version 0.7.31
* This message is deprecated. Use per flag accessor functions given below.
*/
uint16_t
conmon_audinate_clocking_status_get_port_flags
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.31
* Get port user_disabled flag
*/
conmon_audinate_port_user_disabled_t
conmon_audinate_clocking_status_get_port_flag_user_disabled
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.31
* Get port link down flag
*/
conmon_audinate_port_link_down_t
conmon_audinate_clocking_status_get_port_flag_link_down
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension
* @since Audinate message version 0.7.31
* Get port unicast delay request flag
*/
conmon_audinate_port_udelay_request_t
conmon_audinate_clocking_status_get_port_flag_unicast_delay_request
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext2_t * ps
);

/**
* Clocking status extension 3
* @since Audinate message version 0.7.38
* Get port status extension3 for a give port index.
*/

const conmon_audinate_port_status_ext3_t *
conmon_audinate_port_status_ext3_get_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
* Clocking status extension 3
* @since Audinate message version 0.7.38
* Get port network index.
*/
dante_network_index_t
conmon_audinate_clocking_status_get_port_network_index
(
	const conmon_message_body_t * aud_msg,
	const conmon_audinate_port_status_ext3_t * ps
);

/**
* Clocking status extension 3
* @since Audinate message version 0.7.40
* Get ptp v2 domain number.
* returns
*/
aud_error_t
conmon_audinate_clocking_status_get_v2_domain_number
(
	const conmon_message_body_t * aud_msg,
	uint8_t * domain_number
);

/**
 * Initialise a clocking control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_clocking_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the source field for a clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param source the new source for the clock
 */
void
conmon_audinate_clocking_control_set_source
(
	conmon_message_body_t * aud_msg,
	uint16_t source
);

/**
 * Set the preferred field for a clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param preferred the new 'preferred' value for the clock
 */
void
conmon_audinate_clocking_control_set_preferred
(
	conmon_message_body_t * aud_msg,
	uint8_t preferred
);

/**
 * Set the subdomain index field for a clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param subdomain the new subdomain for the clock
 *
 * @deprecated use conmon_audinate_clocking_control_set_subdomain_name instead
 */
void
conmon_audinate_clocking_control_set_subdomain
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_clock_subdomain_t subdomain
);

/**
 * Set the subdomain name field for a clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param subdomain_name the new subdomain for the clock
 *
 * @deprecated use conmon_audinate_clocking_control_set_subdomain_name instead
 */
void
conmon_audinate_clocking_control_set_subdomain_name
(
	conmon_message_body_t * aud_msg,
	const char * subdomain_name
);

/**
 * This control is no longer valid for devices supporting per-port unicast delay request.
 * Per port unicast delay request feature is added from conmon version 0.7.31.
 *
 * Set the unicast_delay_requests field for a clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param unicast_delay_requests the new unicast_delay_requests setting for the clock
 */
void
conmon_audinate_clocking_control_set_unicast_delay_requests
(
	conmon_message_body_t * aud_msg,
	aud_bool_t unicast_delay_requests
);

void
conmon_audinate_clocking_control_set_multicast_ports_enabled
(
	conmon_message_body_t * aud_msg,
	aud_bool_t multicast_ports_enabled
);

/**
* Enable/Disable ports based on ptp version (v1 or v2).
*
*/
void
conmon_audinate_clocking_control_set_version_specific_ports_enabled
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_port_version_t ptp_version,
	aud_bool_t ports_enabled
);

/**
* Enable/Disable ports based on transport layer.
*
*/
void
conmon_audinate_clocking_control_set_transport_layer_ports_enabled
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_port_transport_layer_t layer,
	aud_bool_t ports_enabled
);

/**
* Enable/Disable a port using port ID.
*
*/
void
conmon_audinate_clocking_control_set_port_id_enabled
(
	conmon_message_body_t * aud_msg,
	uint16_t port_id,
	aud_bool_t port_enabled
);

/**
 * Enabled/Disable unicast delay request on a specific ptp version
 *
 * @param port_id: the port id on which to enabled/disable unicast delay requests
 * @param unicast_delay_requests: boolean value set to TRUE to enable and FALSE to DISABLE
 */

void
conmon_audinate_clocking_control_set_version_specific_unicast_delay_requests
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_port_version_t ptp_version,
	aud_bool_t unicast_delay_requests
);

void
conmon_audinate_clocking_control_set_slave_only_enabled
(
	conmon_message_body_t * aud_msg,
	aud_bool_t slave_only_enabled
);

/**
 * Get the size (in bytes) of the clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 */
uint16_t
conmon_audinate_clocking_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Clock master query / status
//----------------------------------------------------------

/**
 * How many ports are present in the 'master status' message?
 *
 * @param aud_msg the master status message
 */
uint16_t
conmon_audinate_master_status_num_ports
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the port state for the port at the given index
 *
 * @param aud_msg the master status message
 * @param index the index of the port state to be obtained
 */
conmon_audinate_port_state_t
conmon_audinate_master_status_port_state_at_index
(
	const conmon_message_body_t * aud_msg,
	unsigned int index
);

/**
 * Get the clock subdomain for the device
 */
const char *
conmon_audinate_master_status_get_clock_subdomain_name
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Unicast Clocking control / status
//----------------------------------------------------------

/** A structure representing the current state of a unicast ptp port */
typedef struct conmon_audinate_unicast_port_status conmon_audinate_unicast_port_status_t;

/**
 * How many unicast ports does this device have?
 *
 * @param aud_msg a conmon message body containing the unicast port port status message
 */
uint16_t
conmon_audinate_unicast_clocking_status_num_ports
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the unicast port status information at the given index
 *
 * @param aud_msg a conmon message body containing the unicast port port status message
 * @param index the index of the desired unicast port
 */
const conmon_audinate_unicast_port_status_t *
conmon_audinate_unicast_clocking_status_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
 * What is the current unicast port state?
 *
 * @param status a unicast port status object
 * @param aud_msg a conmon message body containing the unicast port port status message
 */
conmon_audinate_port_state_t
conmon_audinate_unicast_port_status_get_port_state
(
	const conmon_audinate_unicast_port_status_t * status,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the current unicast domain size that can be supported by this device?
 *
 * @param status a unicast port status object
 * @param aud_msg a conmon message body containing the unicast port port status message
 */
uint16_t
conmon_audinate_unicast_port_status_num_devices
(
	const conmon_audinate_unicast_port_status_t * status,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the maximum unicast domain size that can be supported by this device?
 *
 * @param status a unicast port status object
 * @param aud_msg a conmon message body containing the unicast port port status message
 */
uint16_t
conmon_audinate_unicast_port_status_max_devices
(
	const conmon_audinate_unicast_port_status_t * status,
	const conmon_message_body_t * aud_msg
);


/**
 * Initialise a clocking control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_unicast_clocking_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Enable / Disable the unicast clocking port
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param enabled the new enabled value for the unicast port
 */
void
conmon_audinate_unicast_clocking_control_set_enabled
(
	conmon_message_body_t * aud_msg,
	aud_bool_t enabled
);

/**
 * Set the message flag that tells the unicast port to reload its devices list.
 * This action may cause the port to be shut down and reinitialised.
 *
 * @param aud_msg A conmon message body containing a clocking control message
 * @param reload_devices the new reload devices setting for the unicast port
 */
void
conmon_audinate_unicast_clocking_control_set_reload_devices
(
	conmon_message_body_t * aud_msg,
	aud_bool_t reload_devices
);

/**
 * Get the size (in bytes) of the clocking control message
 *
 * @param aud_msg A conmon message body containing a clocking control message
 */
uint16_t
conmon_audinate_unicast_clocking_control_get_size
(
	const conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// Name/ID Control & Status
//----------------------------------------------------------

void
conmon_audinate_init_name_id_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us,
	uint16_t num_devices
);


void
conmon_audinate_init_name_id_status
(
	conmon_message_body_t * aud_msg,
	uint16_t num_devices
);

uint16_t
conmon_audinate_name_id_get_size
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_name_id_num_devices
(
	const conmon_message_body_t * aud_msg
);

const char *
conmon_audinate_name_id_device_name_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

aud_bool_t
conmon_audinate_name_id_instance_id_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index,
	conmon_instance_id_t * instance_id
);

const conmon_audinate_clock_uuid_t *
conmon_audinate_name_id_ptp_uuid_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

aud_error_t
conmon_audinate_name_id_set_device_name_id_at_index
(
	conmon_message_body_t * aud_msg,
	uint16_t index,
	const char * device_name,
	const char * dns_domain_name,
	const conmon_instance_id_t * instance_id,
	const conmon_audinate_clock_uuid_t * uuid
);

//----------------------------------------------------------
// PTP logging status and control
//----------------------------------------------------------

/**
 * Initialise a ptp network logging control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_ptp_logging_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

uint16_t
conmon_audinate_ptp_logging_control_get_size
(
	const conmon_message_body_t * aud_msg
);

/**
 * Enable / Disable ptp network logging
 *
 * @param aud_msg A conmon message body containing a ptp logging message
 * @param enabled the new enabled value for network logging
 */
void
conmon_audinate_ptp_logging_network_set_enabled
(
	conmon_message_body_t * aud_msg,
	aud_bool_t enabled
);

/**
 * Is PTP network logging enabled?
 *
 * @param aud_msg the message containing the ptp logging information
 */
aud_bool_t
conmon_audinate_ptp_logging_network_is_enabled
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Interface port statistics status and query
//----------------------------------------------------------

/**
 * Interface statistics for a given link
 */
typedef struct conmon_audinate_ifstats conmon_audinate_ifstats_t;


// Message data
// This data applies to the device or message rather than specific interfaces

enum conmon_ifstats
{
		// A Dante link (either a direct link or an internal link between dante and a switch)
	CONMON_AUDINATE_IFSTATS_PORT_TYPE_DANTE,

		// a physical link that is not a Dante link (a plug on a box that feeds a switch)
	CONMON_AUDINATE_IFSTATS_PORT_TYPE_PHYSICAL
};

/**
 * How many dante interfaces are there on the device?
 *
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint16_t
conmon_audinate_ifstats_status_num_interfaces
(
	const conmon_message_body_t * aud_msg
);


/**
 * Interface statistics capabilities
 */
enum conmon_audinate_ifstats_capability
{
	CONMON_AUDINATE_IFSTATS_CAPABILITY__UTILIZATION  = 1 << 0,
		//!< Can get utilization counters
	CONMON_AUDINATE_IFSTATS_CAPABILITY__ERRORS       = 1 << 1,
		//!< Can get error counters
	CONMON_AUDINATE_IFSTATS_CAPABILITY__CLEAR_ERRORS = 1 << 2,
		//!< Can clear error counters (via query / control message)

	CONMON_AUDINATE_IFSTATS_CAPABILITY__ALL = 0x7
		//!< All currently valid capabilities
};

typedef uint32_t conmon_audinate_ifstats_capability_t;

/**
 * Bitset of ifstats capabilities supported by this device
 */
conmon_audinate_ifstats_capability_t
conmon_audinate_ifstats_status_get_capabilities
(
	const conmon_message_body_t * aud_msg,
	size_t msg_size
);


// Interface-specific data

/**
 * How many ports are there for the given dante interface?
 *
 * @param aud_msg a conmon message body containing the interface stats message
 * @param network_index the index of the dante interface being queried
 *     0 - for primary interface
 *     1 - for secondary interface
 */
uint16_t
conmon_audinate_ifstats_status_num_interface_ports
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index
);

/**
 * Get the interface status for the dante interfaces actual link.
 * For direct-mode interfaces this is the only link. For switch-mode devices
 * this is the link between dante and the switch
 *
 * @param aud_msg a conmon message body containing the interface stats message
 * @param network_index the index of the dante interface being queried

 * @deprecated use conmon_audinate_ifstatus_status_dante_link_at_index instead
 */
const conmon_audinate_ifstats_t *
conmon_audinate_ifstats_status_interface_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index
);

/**
 * Get the interface status for the dante interfaces actual link.
 * For direct-mode interfaces this is the only link. For switch-mode devices
 * this is the link between dante and the switch
 *
 * @param aud_msg a conmon message body containing the interface stats message
 * @param network_index the index of the dante interface being queried
 * @param port_index the index of the port on the given dante interface
 *
 * @deprecated use conmon_audinate_ifstatus_status_dante_link_at_index instead
 */
const conmon_audinate_ifstats_t *
conmon_audinate_ifstats_status_interface_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t network_index,
	uint16_t port_index
);

/**
 * What is the tx link utilization for the given link?
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint32_t
conmon_audinate_ifstats_get_tx_util
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * What is the rx link utilization for the given link?
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint32_t
conmon_audinate_ifstats_get_rx_util
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * How many tx errors have been seen on the given link?
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint32_t
conmon_audinate_ifstats_get_tx_errors
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * How many rx errors have been seen on the given link?
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint32_t
conmon_audinate_ifstats_get_rx_errors
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * Get the link type for this link, eg. Dante, physical
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint8_t
conmon_audinate_ifstats_get_port_type
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * Get the link index for links of this type. eg phyical 0 or phsyical 1
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint8_t
conmon_audinate_ifstats_get_port_type_index
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * Get the interface flags for a given link
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
conmon_audinate_interface_flags_t
conmon_audinate_ifstats_get_flags
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);

/**
 * Get the link speed for a given link
 *
 * @param ifstats an interface statistics pointer obtain via the interface_port_at_index function
 * @param aud_msg a conmon message body containing the interface stats message
 */
uint32_t
conmon_audinate_ifstats_get_link_speed
(
	const conmon_audinate_ifstats_t * ifstats,
	const conmon_message_body_t * aud_msg
);


/**
 * Initialise an ifstats query / control message as a simple query.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_ifstats_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Size of this ifstats query / control
 */
uint16_t
conmon_audinate_ifstats_control_get_size
(
	const conmon_message_body_t * aud_msg
);


/**
 * Reset the ifstats error counters after populating the next status message.
 */
void
conmon_audinate_ifstats_control_set_clear_errors
(
	conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// IGMP version control and status
//----------------------------------------------------------

enum
{
	CONMON_AUDINATE_IGMP_VERSION_DEFAULT = 0x0000000,
	CONMON_AUDINATE_IGMP_VERSION_ONE = 0x0000001,
	CONMON_AUDINATE_IGMP_VERSION_TWO = 0x0000002,
	CONMON_AUDINATE_IGMP_VERSION_THREE = 0x0000003
};

/**
 * Get the igmp version in the igmp version status message
 *
 * @param aud_msg A conmon message body containing the igmp version status message
 */
uint16_t
conmon_audinate_igmp_version_status_get_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise an IGMP version control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_igmp_version_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the igmp version field in the igmp version control message
 *
 * @param aud_msg A conmon message body containing the igmp version control message
 * @param version the igmp version
 */
void
conmon_audinate_igmp_version_control_set_version
(
	conmon_message_body_t * aud_msg,
	uint16_t version
);

/**
 * Get the size (in bytes) of the igmp version control message
 *
 * @param aud_msg A conmon message body containing a igmp version control message
 */
uint16_t
conmon_audinate_igmp_version_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Audinate device version query and status
//----------------------------------------------------------

/**
 * Control message that queries and gets audinate version information
 */

/**
 * The length (in bytes) of a model ID
 */
#define CONMON_AUDINATE_MODEL_ID_LENGTH 8

/** A structure representing an audinate model ID */
typedef struct
{
        /** the model id data as an 8-byte array */
        uint8_t data[CONMON_AUDINATE_MODEL_ID_LENGTH];
} conmon_audinate_model_id_t;

char *
conmon_audinate_model_id_to_string
(
	const conmon_audinate_model_id_t * id,
	char * buf,
	size_t len
);

enum conmon_capability_index
{
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_IDENTIFY = 0,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SYS_RESET = 1,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_WEBSERVER = 2,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_SRATE = 3,

	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_ENCODING = 4,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_EDK_BOARD = 5,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_TFTPCLIENT = 6,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_NO_EXT_WORD_CLOCK = 7, // NOTE the negation!

	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_LEDS = 8,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_PULLUPS = 9,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SERIAL_PORTS = 10,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_UNICAST_CLOCKING = 11,

	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_MANF_VERSIONS = 12,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SWITCH_REDUNDANCY = 13,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_STATIC_IP = 14,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_METERING = 15,

	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_DISABLE_LINK_LOCAL = 16,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_HAREMOTE = 17,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SWITCH_VLAN_CONFIG = 18,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_AUDIO_INTERFACE = 19,

	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_ENABLE_PTP_LOGGING = 20,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_DELAY_LINK_LOCAL = 21,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_CLEAR_CONFIG = 22,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_GPIO_CONTROL = 23,

	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_XMODEM_UPGRADE = 24,
	CONMON_AUDINATE_CAPABILITY_INDEX_WILL_NOT_REBOOT_AFTER_UPGRADE = 25,  // NOTE the negation!

	CONMON_AUDINATE_CAPABILITY_INDEX_SUPPORTS_AES67 = 26,
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_LOCK = 27,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_NO_REMOTE_LOCK_RESET = 28,

	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_DDP = 29,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_PB = 30,
	CONMON_AUDINATE_CAPABILITY_INDEX_HAS_USER = 31,

	CONMON_AUDINATE_NUM_CAPABILITIES
};

enum conmon_capability
{
	CONMON_AUDINATE_CAPABILITY_CAN_IDENTIFY                     = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_IDENTIFY),
	CONMON_AUDINATE_CAPABILITY_CAN_SYS_RESET                    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SYS_RESET),
	CONMON_AUDINATE_CAPABILITY_HAS_WEBSERVER                    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_WEBSERVER),
	CONMON_AUDINATE_CAPABILITY_CAN_SET_SRATE                    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_SRATE),
	CONMON_AUDINATE_CAPABILITY_CAN_SET_ENCODING                 = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_ENCODING),
	CONMON_AUDINATE_CAPABILITY_HAS_EDK_BOARD                    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_EDK_BOARD),
	CONMON_AUDINATE_CAPABILITY_HAS_TFTPCLIENT                   = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_TFTPCLIENT),
	CONMON_AUDINATE_CAPABILITY_HAS_NO_EXT_WORD_CLOCK            = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_NO_EXT_WORD_CLOCK),
	CONMON_AUDINATE_CAPABILITY_HAS_LEDS                         = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_LEDS),
	CONMON_AUDINATE_CAPABILITY_CAN_SET_PULLUPS                  = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_SET_PULLUPS),
	CONMON_AUDINATE_CAPABILITY_HAS_SERIAL_PORTS                 = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SERIAL_PORTS),
	CONMON_AUDINATE_CAPABILITY_HAS_UNICAST_CLOCKING             = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_UNICAST_CLOCKING),
	CONMON_AUDINATE_CAPABILITY_HAS_MANF_VERSIONS                = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_MANF_VERSIONS),
	CONMON_AUDINATE_CAPABILITY_HAS_SWITCH_REDUNDANCY            = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SWITCH_REDUNDANCY),
	CONMON_AUDINATE_CAPABILITY_HAS_STATIC_IP                    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_STATIC_IP),
	CONMON_AUDINATE_CAPABILITY_HAS_METERING                     = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_METERING),
	CONMON_AUDINATE_CAPABILITY_CAN_DISABLE_LINK_LOCAL           = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_DISABLE_LINK_LOCAL),
	CONMON_AUDINATE_CAPABILITY_HAS_HAREMOTE                     = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_HAREMOTE),
	CONMON_AUDINATE_CAPABILITY_HAS_SWITCH_VLAN_CONFIG           = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_SWITCH_VLAN_CONFIG),
	CONMON_AUDINATE_CAPABILITY_HAS_AUDIO_INTERFACE              = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_AUDIO_INTERFACE),
	CONMON_AUDINATE_CAPABILITY_CAN_ENABLE_PTP_LOGGING           = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_ENABLE_PTP_LOGGING),
	CONMON_AUDINATE_CAPABILITY_CAN_DELAY_LINK_LOCAL             = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_DELAY_LINK_LOCAL),
	CONMON_AUDINATE_CAPABILITY_CAN_CLEAR_CONFIG                 = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_CLEAR_CONFIG),
	CONMON_AUDINATE_CAPABILITY_CAN_GPIO_CONTROL                 = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_GPIO_CONTROL),
	CONMON_AUDINATE_CAPABILITY_CAN_XMODEM_UPGRADE               = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_XMODEM_UPGRADE),
	CONMON_AUDINATE_CAPABILITY_WILL_NOT_REBOOT_AFTER_UPGRADE    = (1 << CONMON_AUDINATE_CAPABILITY_INDEX_WILL_NOT_REBOOT_AFTER_UPGRADE),
	CONMON_AUDINATE_CAPABILITY_SUPPORTS_AES67	  	    		= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_SUPPORTS_AES67),
	CONMON_AUDINATE_CAPABILITY_CAN_LOCK	  	            		= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_LOCK),
	CONMON_AUDINATE_CAPABILITY_HAS_NO_REMOTE_LOCK_RESET	    	= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_NO_REMOTE_LOCK_RESET),
	CONMON_AUDINATE_CAPABILITY_HAS_DDP	    					= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_DDP),
	CONMON_AUDINATE_CAPABILITY_HAS_PB	    					= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_PB),
	CONMON_AUDINATE_CAPABILITY_HAS_USER	    					= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_HAS_USER),
};

enum conmon_capability1_index
{
	CONMON_AUDINATE_CAPABILITY_INDEX_CAN_GET_DATA = 0,
	CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_NO_PTP = 1,
	CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_SEND_UPGRADE_STATUS_ADDITONAL_TARGET = 2,
	CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_CODEC_CONTROL = 3,
	CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_MANF_OVERRIDE = 4,
	CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_ACCEPT_FQDN_DDM_ENROL_CONFIG = 5,
	CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_SEND_CHANNEL_LABEL_CHANGE = 6,

	CONMON_AUDINATE_NUM_CAPABILITIES1
};

enum conmon_capability1
{
	CONMON_AUDINATE_CAPABILITY_CAN_GET_DATA					= (1 << CONMON_AUDINATE_CAPABILITY_INDEX_CAN_GET_DATA),
	CONMON_AUDINATE_CAPABILITY1_HAS_NO_PTP					= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_NO_PTP),
	CONMON_AUDINATE_CAPABILITY1_CAN_SEND_UPGRADE_STATUS_ADDITIONAL_TARGET	= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_SEND_UPGRADE_STATUS_ADDITONAL_TARGET),
	CONMON_AUDINATE_CAPABILITY1_HAS_CODEC_CONTROL				= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_CODEC_CONTROL),
	CONMON_AUDINATE_CAPABILITY1_HAS_MANF_OVERRIDE				= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_HAS_MANF_OVERRIDE),
	CONMON_AUDINATE_CAPABILITY1_CAN_ACCEPT_FQDN_DDM_ENROL_CONFIG		= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_ACCEPT_FQDN_DDM_ENROL_CONFIG),
	CONMON_AUDINATE_CAPABILITY1_CAN_SEND_CHANNEL_LABEL_CHANGE  		= (1 << CONMON_AUDINATE_CAPABILITY1_INDEX_CAN_SEND_CHANNEL_LABEL_CHANGE),
};

enum conmon_capability_domain_index
{
	CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__SUPPORTED = 0,
	CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__CAN_RESET = 1,
	CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__AUDIO_RTP_SUPPORTED = 2,

	CONMON_AUDINATE_NUM_DOMAIN_CAPABILITIES
};

enum conmon_capability_domain
{
	CONMON_AUDINATE_CAPABILITY_DOMAIN__SUPPORTED				= (1 << CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__SUPPORTED),
	CONMON_AUDINATE_CAPABILITY_DOMAIN__CAN_RESET				= (1 << CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__CAN_RESET),
	CONMON_AUDINATE_CAPABILITY_DOMAIN__AUDIO_RTP_SUPPORTED			= (1 << CONMON_AUDINATE_CAPABILITY_DOMAIN_INDEX__AUDIO_RTP_SUPPORTED),
};


enum
{
	CONMON_AUDINATE_DEVICE_STATUS_INDEX_CAPABILITY_PARTITION_ERROR,
	CONMON_AUDINATE_DEVICE_STATUS_INDEX_USER_PARTITION_ERROR,
	CONMON_AUDINATE_DEVICE_STATUS_INDEX_CONFIG_STORE_FAILURE,
	CONMON_AUDINATE_DEVICE_STATUS_INDEX_SWITCH_PARTITION_ERROR,
	CONMON_AUDINATE_NUM_DEVICE_STATUSES
};

enum
{
	/** The capability partition is corrupted */
	CONMON_AUDINATE_DEVICE_STATUS_FLAG_CAPABILITY_PARTITION_ERROR
		= (1 << CONMON_AUDINATE_DEVICE_STATUS_INDEX_CAPABILITY_PARTITION_ERROR),

	/** The user partition is corrupted */
	CONMON_AUDINATE_DEVICE_STATUS_FLAG_USER_PARTITION_ERROR
		= (1 << CONMON_AUDINATE_DEVICE_STATUS_INDEX_USER_PARTITION_ERROR),

	/** The dante config has failed - no config can be stored or restored */
	CONMON_AUDINATE_DEVICE_STATUS_FLAG_CONFIG_STORE_FAILURE
		= (1 << CONMON_AUDINATE_DEVICE_STATUS_INDEX_CONFIG_STORE_FAILURE),

	/** The switch/phy partition is corrupted */
	CONMON_AUDINATE_DEVICE_STATUS_FLAG_SWITCH_PARTITION_ERROR
		= (1 << CONMON_AUDINATE_DEVICE_STATUS_INDEX_SWITCH_PARTITION_ERROR)
};

enum
{
	CONMON_AUDINATE_CLOCK_PROTOCOL_INDEX_PTPV1,
	CONMON_AUDINATE_NUM_CLOCK_PROTOCOLS
};

enum
{
	CONMON_AUDINATE_CLOCK_PROTOCOL_FLAG_PTPV1 = (1 << CONMON_AUDINATE_CLOCK_PROTOCOL_INDEX_PTPV1),
};

typedef uint32_t conmon_audinate_clock_protocol_flags_t;

/**
 * Get the software version from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_dante_software_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the software version and build information from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 * @param version_ptr a pointer to a version structure that will hold the software version
 * @param version_build_ptr a pointer to a version build structure that will hold the software build version
 */
void
conmon_audinate_versions_status_get_dante_software_version_build
(
	const conmon_message_body_t * aud_msg,
	dante_version_t * version_ptr,
	dante_version_build_t * version_build_ptr
);

/**
 * Get the firmware version from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_dante_firmware_version
(
	const conmon_message_body_t * aud_msg
);


/**
 * Get the firmware version and build information from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 * @param version_ptr a pointer to a version structure that will hold the firmware version
 * @param version_build_ptr a pointer to a version build structure that will hold the firmware build version
 */
void
conmon_audinate_versions_status_get_dante_firmware_version_build
(
	const conmon_message_body_t * aud_msg,
	dante_version_t * version_ptr,
	dante_version_build_t * version_build_ptr
);

/**
 * Get the api version from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_dante_api_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the uboot version from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_uboot_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the upgrade version from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
unsigned
conmon_audinate_versions_status_get_upgrade_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the model id from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
const conmon_audinate_model_id_t *
conmon_audinate_versions_status_get_dante_model_id
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the model name from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
const char *
conmon_audinate_versions_status_get_dante_model_name
(
	const conmon_message_body_t * aud_msg
);

/**
 * Set the model name for a version status message
 *
 * @param aud_msg a conmon message body containing a versions status message
 * @param model_name friendly string
 */
void
conmon_audinate_versions_status_set_dante_model_name
(
	conmon_message_body_t * aud_msg,
	const char * name
);

/**
 * Get the capability flags from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_capability_flags
(
	const conmon_message_body_t * aud_msg
);

/**
 * Capability flags provide a description of a device's features.
 * However, there are cases where a feature was added before a corresponding
 * capability was defined. This function infers missing capabilities
 * from the versions message and adds these to the capabilities that
 * are explicitly described in the message
 *
 * @return The set of explicit and inferred capabilities
 *
 * Capabilities that may be inferred include:
 * - HAS_AUDIO_INTERFACE
 */
uint32_t
conmon_audinate_versions_status_infer_all_capability_flags
(
	const conmon_message_body_t * aud_msg
);

#define CONMON_AUDINATE_CAPABILITY_RO_EXT_WORD_CLOCK CONMON_AUDINATE_CAPABILITY_HAS_NO_EXT_WORD_CLOCK

/**
 * Most device capabilities incoporate some form of configurable data (eg.
 * HAS_SRATE implies a 'sample rate'). Devices supporting these capabilities
 * will normally support getting and setting these values via conmon messages.
 * In some cases the device can provide current state via conmon but does
 * support configuration via conmon. These capabilities are read-only from
 * form a conmon perspective. Certain capabilities are already capable of
 * expressing this (eg. for samplerates and encodings the mode can be set to 'none').
 * Other capabilites need to be explicitly marked as read-only.
 *
 * Capabilities that may be marked read-only include:
 * - EXT_WORD_CLOCK
 *
 * This bitmask defines which of a device's capabilities only have read-only
 * support.
 *
 * @note: For historical reasons, the external word clock capability was inverted, with a
 *   bit set to indicate absence (CONMON_AUDINATE_CAPABILITY_HAS_NO_EXT_WORD_CLOCK).
 *   In the read-only bitmask, this capability's read/write value is intepreted the
 *   same as for other capabilities, ie. 0 = Read/Write, 1=Read Only.
 */
uint32_t
conmon_audinate_versions_status_get_readonly_capability_flags
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the preferred link speed from the versions status message
 *
 * @param aud_msg a conmon message body containing an audinate versions message
 */
uint32_t
conmon_audinate_versions_status_get_preferred_link_speed
(
	const conmon_message_body_t * aud_msg
);

uint32_t
conmon_audinate_versions_status_get_device_status
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_clock_protocol_flags_t
conmon_audinate_versions_status_get_clock_protocols
(
	const conmon_message_body_t * aud_msg
);

uint32_t
conmon_audinate_versions_status_get_guppy_monitoring_flags
(
	const conmon_message_body_t * aud_msg
);

uint32_t
conmon_audinate_versions_status_get_capability1_flags
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_message_flagset_t
conmon_audinate_versions_status_get_domain_capability_flags
(
	const conmon_message_body_t * aud_msg
);


conmon_audinate_message_flagset_t
conmon_audinate_versions_status_get_domain_capability_flags
(
	const conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// Manufacturer device version query and status
//----------------------------------------------------------

/**
 * Get the manufacturer id from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer version status message
 */
const conmon_vendor_id_t *
conmon_audinate_manf_versions_status_get_manufacturer
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the user-friendly manufacturer name from a manufacturer version status message.
 * The manufacturer name is a UTF-8 character string
 *
 * @param aud_msg a conmon message body containing a manufacturer version status message
 *
 * @return the name specified in the message, or NULL if no name is specified (or no name is available)
 */
const char *
conmon_audinate_manf_versions_status_get_manufacturer_name
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the model id information from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
const conmon_audinate_model_id_t *
conmon_audinate_manf_versions_status_get_model_id
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the user-friendly model / product name from a manufacturer version status message.
 * The model / product name is a UTF-8 character string.
 *
 * @param aud_msg a conmon message body containing a manufacturer version status message
 */
const char *
conmon_audinate_manf_versions_status_get_model_name
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the model (product) version from a manufacturer versions status message.
 * This function returns a 3-value field containing 8/8/16 bits
 *
 * @param aud_msg a conmon message body containing a manufacturer version status message
 */
uint32_t
conmon_audinate_manf_versions_status_get_model_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the model version friendly string.
 * This is a textual alternative to the model version. Vendors wanting to use product version
 * formats other than MAJOR.MINOR.BUGFIX can add a string here for controllers to render.
 * The product / version string is an ascii string.
 *
 * @param aud_msg a conmon message body containing a manufacturer version status message
 */
const char *
conmon_audinate_manf_versions_status_get_model_version_string
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the serial number information from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
const dante_device_id_t *
conmon_audinate_manf_versions_status_get_serial_id
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the software version information from a manufacturer version status message
 * This function returns a 3-value field containing 8/8/16 bits
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
uint32_t
conmon_audinate_manf_versions_status_get_software_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the software version and build information from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
void
conmon_audinate_manf_versions_status_get_software_version_build
(
	const conmon_message_body_t * aud_msg,
	dante_version_t * version_ptr,
	dante_version_build_t * version_build_ptr
);

/**
 * Get the firmware version information from a manufacturer version status message
 * This function returns a 3-value field containing 8/8/16 bits
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
uint32_t
conmon_audinate_manf_versions_status_get_firmware_version
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the firmware version and build information from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
void
conmon_audinate_manf_versions_status_get_firmware_version_build
(
	const conmon_message_body_t * aud_msg,
	dante_version_t * version_ptr,
	dante_version_build_t * version_build_ptr
);

/**
 * Get the manufacturer capabilities information from a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
uint32_t
conmon_audinate_manf_versions_status_get_capabilities
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise a manufacturer versions status message
 *
 * @param aud_msg a conmon message boy that will contain a manufacturer versions status message
 */
void
conmon_audinate_manf_versions_status_init
(
	conmon_message_body_t * aud_msg
);

/**
 * Set the manufacturer id for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param vendor_id the manufacturers vendor id. This value is provided to the manufacturer by Audinate.
 */
void
conmon_audinate_manf_versions_status_set_manufacturer
(
	conmon_message_body_t * aud_msg,
	const conmon_vendor_id_t * vendor_id
);

/**
 * Set the manufacturer name for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param vendor_id the manufacturers friendly name.
 *   This is a manufacturer-specific UTF8-encoded string containing the Manufacturer's name
 */
void
conmon_audinate_manf_versions_status_set_manufacturer_name
(
	conmon_message_body_t * aud_msg,
	const char * name
);

/**
 * Set the model id for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param model_id the model id. This value is manufacturer-specific.
 */
void
conmon_audinate_manf_versions_status_set_model_id
(
	conmon_message_body_t * aud_msg,
	const conmon_audinate_model_id_t * model_id
);

/**
 * Set the model name for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param model_id the model id. This is a manufacturer-specific UTF-encoded string
 *   containing the name of the product
 */
void
conmon_audinate_manf_versions_status_set_model_name
(
	conmon_message_body_t * aud_msg,
	const char * name
);

/**
 * Set the model (product) version for this device
 */
void
conmon_audinate_manf_versions_status_set_model_version
(
	conmon_message_body_t * aud_msg,
	uint32_t version
);

/**
 * Set the model (product) version string for this device. This field
 * is optional and only needed if the version is to be rendered in a form other 
 * than MAJOR.MINOR.BUGFIX
 */
void
conmon_audinate_manf_versions_status_set_model_version_string
(
	conmon_message_body_t * aud_msg,
	const char * version_string
);

/**
 * Set the serial number information for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param serial_id the serial number of the device
 */
void
conmon_audinate_manf_versions_status_set_serial_id
(
	conmon_message_body_t * aud_msg,
	const dante_device_id_t * serial_id
);

/**
 * Set the software version information for a manufacturer version status message.
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param software_version the software version. If unset a value of '0' will be returned.
 *
 * @note Version format is intepreted as 8-bit major version, 8-bit minor version, 16-bit bugfix version
 */
void
conmon_audinate_manf_versions_status_set_software_version
(
	conmon_message_body_t * aud_msg,
	uint32_t software_version
);

/**
 * Set the software version and build information for a manufacturer version status message.
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param software_version the software version. If NULL a value of '0' will be used.
 * @param software_build the software version build. If NULL a value of '0' will be used.
 */
void
conmon_audinate_manf_versions_status_set_software_version_build
(
	conmon_message_body_t * aud_msg,
	const dante_version_t * software_version,
	const dante_version_build_t * software_build

);

/**
 * Set the firmware version information for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param firmware_version the firmware version. If unset a value of '0' will be returned.
 *
 * @note Version format is intepreted as 8-bit major version, 8-bit minor version, 16-bit bugfix version
 */
void
conmon_audinate_manf_versions_status_set_firmware_version
(
	conmon_message_body_t * aud_msg,
	uint32_t firmware_version
);

/**
 * Set the firmware version and build information for a manufacturer version status message.
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param firmware_version the software version. If NULL a value of '0' will be used.
 * @param firmware_build the software version build. If NULL a value of '0' will be used.
 */
void
conmon_audinate_manf_versions_status_set_firmware_version_build
(
	conmon_message_body_t * aud_msg,
	const dante_version_t * firmware_version,
	const dante_version_build_t * firmware_build
);

/**
 * Set the capabilities for a manufacturer version status message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 * @param capabilities manuafacturer-specific capabilities supported on this device
 */
void
conmon_audinate_manf_versions_status_set_capabilities
(
	conmon_message_body_t * aud_msg,
	uint32_t capabilities
);

/**
 * Get the size of a manufacturer versions message
 *
 * @param aud_msg a conmon message body containing a manufacturer versions status message
 */
uint16_t
conmon_audinate_manf_versions_status_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// device sample rate
//----------------------------------------------------------

/**
	ConMon can be used to query a device's current and available sample rates,
	and to set the current sample rate.  On some devices, setting the current
	sample rate takes effect immediately, while others apply changes only on
	reboot.
 */
enum conmon_srate
{
	CONMON_AUDINATE_SRATE_44K = 44100,
	CONMON_AUDINATE_SRATE_48K = 48000,
	CONMON_AUDINATE_SRATE_96K = 96000,
	CONMON_AUDINATE_SRATE_192K = 192000,
	CONMON_AUDINATE_SRATE_88K = 88200,
	CONMON_AUDINATE_SRATE_176K = 176400
};

enum conmon_srate_mode
{
	// Samplerate change is not possible via conmon audinate messages
	CONMON_AUDINATE_SRATE_MODE_FIXED  = 0x0000,

	// Sample rate change requires a reboot to take effect
	CONMON_AUDINATE_SRATE_MODE_REBOOT  = 0x0001,

	// Sample rate change will take effect immediately
	CONMON_AUDINATE_SRATE_MODE_IMMEDIATE = 0x0002
};

// Control message

/**
 * Initialise a sample rate control message
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_srate_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the new sample rate. This value may not be applied until after a reboot
 *
 * @param aud_msg A conmon message body containing a sample rate control message
 * @param rate the new sample rate for the device
 */
void
conmon_audinate_srate_control_set_rate
(
	conmon_message_body_t * aud_msg,
	uint32_t rate
);

/**
 * Get the size of a sample rate control message
 *
 * @param aud_msg A conmon message body containing a sample rate control message
 *
 * @return the size of the sample rate control message
 */
uint16_t
conmon_audinate_srate_control_get_size
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the sample rate change mode for the device
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 *
 * @return the sample rate change mode
 */
uint16_t
conmon_audinate_srate_get_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the current sample rate for the device
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 *
 * @return the current sample rate
 */
uint32_t
conmon_audinate_srate_get_current
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the sample rate the device will use after a reboot, if applicable
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 *
 * @return the sample rate the device will have after a reboot, or 0 if not applicable
 */
uint32_t
conmon_audinate_srate_get_new
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the number of available sample rates
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 *
 * @return number of rates, or 0 on error
 */
uint32_t
conmon_audinate_srate_get_available_count
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the available sample rate at the given index
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 * @param index Index of sample rate.  Must be less than get_available_count.
 *
 * @return the available sample rate at the given index
 */
uint32_t
conmon_audinate_srate_get_available
(
	const conmon_message_body_t * aud_msg,
	unsigned int index
);


//----------------------------------------------------------
// device channel encoding
//----------------------------------------------------------

/*
	ConMon can be used to query a device's current and available channel
	encodings, and to set the current channel encoding.  On some devices,
	setting the current channel encoding takes effect immediately, while
	others apply changes only on reboot.
 */
// Channel encodings are defined in APCP

enum conmon_enc_mode
{
	// Encoding change is not possible via conmon audinate messages
	CONMON_AUDINATE_ENC_MODE_FIXED  = CONMON_AUDINATE_SRATE_MODE_FIXED,

	// Encoding  change requires a reboot to take effect
	CONMON_AUDINATE_ENC_MODE_REBOOT  = CONMON_AUDINATE_SRATE_MODE_REBOOT,

	// Encoding change will take effect immediately
	CONMON_AUDINATE_ENC_MODE_IMMEDIATE = CONMON_AUDINATE_SRATE_MODE_IMMEDIATE
};


// Control message

/**
 * Initialise an encoding control message
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_enc_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the new encoding to use. This value may not be applied until after a reboot
 *
 * @param aud_msg A conmon message body containing a sample rate control message
 * @param enc the new encoding for the device
 */
void
conmon_audinate_enc_control_set_encoding
(
	conmon_message_body_t * aud_msg,
	uint16_t enc
);

/**
 * Get the size of an encoding control message
 *
 * @param aud_msg A conmon message body containing an encoding control message
 *
 * @return the size of the encoding control message
 */
uint16_t
conmon_audinate_enc_control_get_size
(
	const conmon_message_body_t * aud_msg
);


// Status message

/**
 * Get the channel encoding change mode for the device
 *
 * @param aud_msg A conmon message body containing an encoding status message
 *
 * @return the channel encoding change mode
 */
uint16_t
conmon_audinate_enc_get_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the current encoding for the device
 *
 * @param aud_msg A conmon message body containing an encoding status message
 *
 * @return the current encoding
 */
uint16_t
conmon_audinate_enc_get_current
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the enncoding the device will use after a reboot, if applicable
 *
 * @param aud_msg A conmon message body containing an encoding status message
 *
 * @return the encoding the device will have after a reboot, or 0 if not applicable
 */
uint16_t
conmon_audinate_enc_get_new
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the number of available encodings
 *
 * @param aud_msg A conmon message body containing an encoding status message
 *
 * @return number of encodings, or 0 on error
 */
uint32_t
conmon_audinate_enc_get_available_count
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the available encoding at the given index
 *
 * @param aud_msg A conmon message body containing an encoding status message
 * @param index Index of encoding.  Must be less than get_available_count.
 *
 * @return the available encoding at the given index
 */
uint16_t
conmon_audinate_enc_get_available
(
	const conmon_message_body_t * aud_msg,
	unsigned int index
);

//----------------------------------------------------------
// device sample rate pullup / pulldown
//----------------------------------------------------------

/**
	ConMon can be used to query a device's current and available sample
	rate pullup / pulldown values and set the sample rate pullup /
	pulldown to be used when the device is next started.
 */
enum conmon_srate_pullup
{
	CONMON_AUDINATE_SRATE_PULLUP_NONE,
	CONMON_AUDINATE_SRATE_PULLUP_PLUSFOURPOINTONESIXSIXSEVEN,
	CONMON_AUDINATE_SRATE_PULLUP_PLUSPOINTONE,
	CONMON_AUDINATE_SRATE_PULLUP_MINUSPOINTONE,
	CONMON_AUDINATE_SRATE_PULLUP_MINUSFOUR,
	CONMON_AUDINATE_SRATE_PULLUP_MAX
};

enum conmon_srate_pullup_mode
{
	// Samplerate pullup change is not possible via conmon audinate messages
	CONMON_AUDINATE_SRATE_PULLUP_MODE_FIXED  = 0x0000,

	// Sample rate pullup change requires a reboot to take effect
	CONMON_AUDINATE_SRATE_PULLUP_MODE_REBOOT  = 0x0001,

	// Sample rate pullup change will take effect immediately
	CONMON_AUDINATE_SRATE_PULLUP_MODE_IMMEDIATE = 0x0002
};

/*
	These flags indicate additional information about the pull-ups returned.
 */
enum
{
	// Pull-ups are not supported by host device
	// If set, the Dante card supports pull-ups but the host device does not
	CONMON_AUDINATE_SRATE_PULLUP_FLAG__HOST_NOT_SUPPORTED = 1 << 0,
	// All flags defined by this version of the protocol
	CONMON_AUDINATE_SRATE_PULLUP_FLAG__ALL = 0x1
};


// Control message

/**
 * Initialise a sample rate pullup control message
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_srate_pullup_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the sample rate pullup
 *
 * @param aud_msg A conmon message body containing a sample rate pullup control message
 * @param pullup the new sample rate pullup for the device
 */
void
conmon_audinate_srate_pullup_control_set_rate
(
	conmon_message_body_t * aud_msg,
	uint32_t pullup
);

/**
 * Set the subdomain name field for a srate control message
 *
 * @param aud_msg A conmon message body containing a sample rate pullup control message
 * @param subdomain_name the new subdomain for the clock
 */
void
conmon_audinate_srate_pullup_control_set_subdomain
(
	conmon_message_body_t * aud_msg,
	const char * subdomain_name
);
/**
 * Get the size of a sample rate pullup control message
 *
 * @param aud_msg A conmon message body containing a sample rate pullup control message
 *
 * @return the size of the sample rate pullup control message
 */
uint16_t
conmon_audinate_srate_pullup_control_get_size
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the sample rate pullup change mode for the device
 *
 * @param aud_msg A conmon message body containing a sample rate status message
 */
uint16_t
conmon_audinate_srate_pullup_get_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the current sample rate pullup for the device
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 */
uint32_t
conmon_audinate_srate_pullup_get_current
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the new sample rate pullup for the device
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 *
 * @return the encoding the device will have after a reboot, or the current pullup if not applicable
 */
uint32_t
conmon_audinate_srate_pullup_get_new
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the number of available sample rate pullups / pulldowns
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 *
 * @return number of pullups, or 0 on error
 */
uint16_t
conmon_audinate_srate_pullup_get_available_count
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the available sample rate pullup at the given index
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 * @param index Index of sample rate pullup.  Must be less than get_available_count.
 */
uint32_t
conmon_audinate_srate_pullup_get_available
(
	const conmon_message_body_t * aud_msg,
	unsigned int index
);

/**
 * Get pull-up flags that this device knows about
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 *
 * @return bitmask of flags that the device knows about.
 */
uint32_t
conmon_audinate_srate_pullup_get_flags_known
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get pull-up flags
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 *
 * @return bitmask of active pullup flags
 *
 * This will be a subset of the flags returned by flags_known.
 */
uint32_t
conmon_audinate_srate_pullup_get_flags
(
	const conmon_message_body_t * aud_msg
);

/**
 * Does srate pullup status has clock subdomain?
 *
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 */
aud_bool_t conmon_audinate_srate_pullup_has_subdomain
(
	const conmon_message_body_t  * aud_msg
);

/**
 * Get the clock subdomain for the device from srate status message
 * @param aud_msg A conmon message body containing a sample rate pullup status message
 */
const char *
conmon_audinate_srate_pullup_get_subdomain
(
	const conmon_message_body_t  * aud_msg
);

//----------------------------------------------------------
// device reset control audio interface query
//----------------------------------------------------------
enum
{
	/**  Audio clock framing is left / right clock **/
	CONMON_AUDINATE_AUDIO_INTERFACE_FRAME_LRCLK = 0,

	/**  Audio clock framing is start of frame pulse **/
	CONMON_AUDINATE_AUDIO_INTERFACE_FRAME_SOF = 1
};

enum
{
	/** TDM word alignment is 1 bit delay (I2S style) **/
	CONMON_AUDINATE_AUDIO_INTERFACE_ALIGNMENT_INS = 0,

	/** TDM word alignment is left justified **/
	CONMON_AUDINATE_AUDIO_INTERFACE_ALIGNMENT_LJ = 1
};

enum
{
	/** Audio channels assigment begins on falling edge
	 * Channels are remapped so physical channel assignment happen
	 * across TDM lines
	 **/
	CONMON_AUDINATE_AUDIO_INTERFACE_START_FALLING = 0,

	/** Audio channels assigment begins on rising edge
	 * Channels are remapped so physical channel assignment happen
	 * across TDM lines
	 **/
	CONMON_AUDINATE_AUDIO_INTERFACE_START_RISING = 1,

	/** Audio channels assigned such that logical to
	 * physical assignment is 1-1
	 **/
	CONMON_AUDINATE_AUDIO_INTERFACE_LINEAR = 2
};

/**
 * Get the audio interface channels per TDM
 *
 * @param aud_msg A conmon message body containing an audio interface status  message
 */
uint8_t
conmon_audinate_audio_interface_get_chans_per_tdm
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the audio interface frame type
 *
 * @param aud_msg A conmon message body containing an audio interface status  message
 */
uint8_t
conmon_audinate_audio_interface_get_framing
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the audio interface alignment
 *
 * @param aud_msg A conmon message body containing an audio interface status  message
 */
uint8_t
conmon_audinate_audio_interface_get_alignment
(
	const conmon_message_body_t * aud_msg
);


/**
 * Get the audio interface channel mapping
 *
 * @param aud_msg A conmon message body containing an audio interface status  message
 */
uint8_t
conmon_audinate_audio_interface_get_channel_mapping
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// device reset control
//----------------------------------------------------------

enum conmon_sys_reset
{
	CONMON_AUDINATE_SYS_RESET_SOFT = 0x0000,
	CONMON_AUDINATE_SYS_RESET_FACTORY = 0x0001
};

/**
 * Initialise a system reset control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_sys_reset_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

// set the reset mode field in the device sys reset control message
void
conmon_audinate_sys_reset_control_set_mode
(
	conmon_message_body_t * aud_msg,
	uint16_t mode
);

uint16_t
conmon_audinate_sys_reset_control_get_size
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the reset mode field in the device sys reset control message
 *
 * @param aud_msg a conmon message body containing  sys reset control message
 * @param mode_ptr a pointer to a location to put the reset mode
 *
 * @return AUD_SUCCESS if message had a valid mode, and error otherwise
 */
aud_error_t
conmon_audinate_sys_reset_control_get_mode
(
	const conmon_message_body_t * aud_msg,
	uint16_t * mode_ptr
);

/**
 * Get the reset mode field in the device sys reset status message
 */
uint16_t
conmon_audinate_sys_reset_status_get_mode
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// device access control and status
//----------------------------------------------------------

enum conmon_access_control
{
	CONMON_AUDINATE_ACCESS_DISABLE = 0x0000,
	CONMON_AUDINATE_ACCESS_ENABLE = 0x0001,
	CONMON_AUDINATE_INETD_ENABLE = 0x0002,
	CONMON_AUDINATE_INETD_DISABLE = 0x0003
};

uint16_t
conmon_audinate_access_status_get_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise an access control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_access_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

// set the access field in the device access control message
void
conmon_audinate_access_control_set_mode
(
	conmon_message_body_t * aud_msg,
	uint16_t mode
);

uint16_t
conmon_audinate_access_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// device upgrade status
//----------------------------------------------------------

/** Upgrade version
	Returned by newer versions of versions messages
 */
enum
{
	CONMON_AUDINATE_UPGRADE_VERSION__UNKNOWN = 0,
		//!< Unspecified upgrade version
	CONMON_AUDINATE_UPGRADE_VERSION__UNSUPPORTED = 0x100,
		//!< Device cannot be remote-upgraded (via ConMon)
	CONMON_AUDINATE_UPGRADE_VERSION__LEGACY = 0x0200,
		//!< Legacy upgrade support
	CONMON_AUDINATE_UPGRADE_VERSION__3 = 0x0300,
		//!< Version 3 upgrade support (current)
};


//! Upgrade status
enum conmon_upgrade
{
	CONMON_AUDINATE_UPGRADE_STATE_NONE = 0,
		//!< Upgrade has never been run this session
	CONMON_AUDINATE_UPGRADE_STATE_GET_FILE = 1,
		//!< Upgrade is in file download stage
	CONMON_AUDINATE_UPGRADE_STATE_WRITE = 4,
		//!< Upgrade is in flash write stage
	CONMON_AUDINATE_UPGRADE_STATE_DONE = 2,
		//!< Last upgrade completed successfully
	CONMON_AUDINATE_UPGRADE_STATE_FAIL = 3,
		//!< Last upgrade failed (see error code for details)

	CONMON_AUDINATE_UPGRADE_STATE_COUNT = 5,

	// Legacy names
	CONMON_AUDINATE_UPGRADE_START = 0x0001,
	CONMON_AUDINATE_UPGRADE_END_SUCCESS = 0x0002,
	CONMON_AUDINATE_UPGRADE_END_FAIL = 0x0003,
	CONMON_AUDINATE_UPGRADE_FLASH_START = 0x0004
};


//! Upgrade error codes
enum conmon_upgrade_errors
{
	CONMON_AUDINATE_UPGRADE_ERROR__NONE = 0,

	// Correctness errors
	CONMON_AUDINATE_UPGRADE_ERROR__DEVICE_CODE_MISMATCH = 0x101,
	CONMON_AUDINATE_UPGRADE_ERROR__MANUFACTURER_ID_MISMATCH = 0x102,
	CONMON_AUDINATE_UPGRADE_ERROR__MODEL_ID_MISMATCH = 0x103,
	CONMON_AUDINATE_UPGRADE_ERROR__DNT_FILE_MALFORMED = 0x111,
	CONMON_AUDINATE_UPGRADE_ERROR__DNT_BAD_CRC = 0x112,
	CONMON_AUDINATE_UPGRADE_ERROR__DNT_VERSION_UNSUPPORTED = 0x113,
	CONMON_AUDINATE_UPGRADE_ERROR__UNSUPPORTED_OPTIONS = 0x121,
		//!< Unsupported options in command (unsupported actions flags, etc)

	// Flashing errors
	CONMON_AUDINATE_UPGRADE_ERROR__FLASH_ERROR = 0x0200,
		//!< Internal error while flashing
	CONMON_AUDINATE_UPGRADE_ERROR__FLASH_ACCESS_FAILED = 0x0201,
		//!< Failed to access flash

	// Get file errors
	CONMON_AUDINATE_UPGRADE_ERROR__DOWNLOAD_ERROR = 0x0300,
		//!< Internal file download error
	CONMON_AUDINATE_UPGRADE_ERROR__UNKNOWN_FILE = 0x0301,
		//!< Specified file could not be found
	CONMON_AUDINATE_UPGRADE_ERROR__FILE_PERMISSION_DENIED = 0x0302,
		//!< Server would not allow access to given file
	CONMON_AUDINATE_UPGRADE_ERROR__NO_SUCH_SERVER = 0x0303,
		//!< Could not find specified file server
	CONMON_AUDINATE_UPGRADE_ERROR__SERVER_ACCESS_FAILED = 0x0304,
		//!< Connection to file server failed
	CONMON_AUDINATE_UPGRADE_ERROR__SERVER_TIMEOUT = 0x0305,
		//!< Connection to file server timed out
	CONMON_AUDINATE_UPGRADE_ERROR__UNSUPPORTED_PROTOCOL = 0x0306,
		//!< Unsupported protocol
	CONMON_AUDINATE_UPGRADE_ERROR__FILENAME_TOO_LONG = 0x0307,
		//!< Filename is too long
	CONMON_AUDINATE_UPGRADE_ERROR__UNKNOWN = 0x0308,
		//!< Not defined error.
	CONMON_AUDINATE_UPGRADE_ERROR__ILLEGAL_OPERATION = 0x0309,
		//!< Illegal tftp operation.
	CONMON_AUDINATE_UPGRADE_ERROR__UNKNOWN_TRANSFER_ID = 0x030A,
		//!< Unknown transfer id.
	CONMON_AUDINATE_UPGRADE_ERROR__MALLOC_ERROR = 0x030B,
		//!< Malloc error on device
	CONMON_AUDINATE_UPGRADE_ERROR__PARTITION_VERSION_UNSUPPORTED = 0x030C,
		//!< the flash partition contained in the DNT file is too old / unsupported on this platform
	CONMON_AUDINATE_UPGRADE_ERROR__PERMISSION_DENIED = 0x030D,
			//!< the device does not allow upgrade (normally from being locked)
	CONMON_AUDINATE_UPGRADE_ERROR__IN_PROGRESS = 0x030E,
	CONMON_AUDINATE_UPGRADE_ERROR__REBOOT_REQUIRED = 0x030F,
			//!< device has been placed into read only mode after a clear config
};


/**
	Return a string representation of an upgrade state

	@returns
		Short lower-case string representation of given state,
		or empty string on invalid entry.
 */
const char *
conmon_audinate_upgrade_state_to_string(unsigned);

/**
	Return a string representation of an upgrade error

	@returns
		Short lower-case string representation of given state,
		or empty string on invalid entry.
 */
const char *
conmon_audinate_upgrade_error_to_string(unsigned);


/**
 * Upgrade status structure.
 *
 * @note Progress is tracked as current of total: the units
 * of curr and total are implementation dependant, but often
 * count bytes.
 * If total is 0, then curr counts progress, but the end
 * point is not available.
 */
typedef struct conmon_audinate_upgrade_status
{
	uint16_t status;
		//!< Current status of upgrade

	uint16_t last_error;
		//!< Last error since upgrade last started

	struct conmon_audinate_upgrade_status_progress
	{
		uint32_t curr;  //!< Current progress 
		uint32_t total; //!< Total progress, may be 0
	} progress;
		//!< Progress of current upgrade

	dante_id64_t manufacturer;
		//!< Effective manufacturer ID being used by upgrade
	dante_id64_t model;
		//!< Effective model ID being used by upgrade
} conmon_audinate_upgrade_status_t;


enum
{
	CONMON_AUDINATE_UPGRADE_PROTOCOL_LOCAL = 1,
		//!< Path is on local file system
	CONMON_AUDINATE_UPGRADE_PROTOCOL_TFTP_GET = 2,
		//!< Download file from tftp server
	CONMON_AUDINATE_UPGRADE_PROTOCOL_XMODEM = 3,
		//!< Upgrade using a serial interface (UART, SPI) using XModem

	CONMON_AUDINATE_UPGRADE_PROTOCOL_COUNT = 3
};

typedef struct conmon_audinate_upgrade_source_file
{
	uint16_t protocol; 
	uint16_t port;    
	struct in_addr addr_inet;
	uint32_t file_len;
	const char * filename;
} conmon_audinate_upgrade_source_file_t;

enum
{
	XMODEM_PORT_UARTA = 1,
	XMODEM_PORT_UARTB,
	XMODEM_PORT_SPI0,
};

enum
{
	XMODEM_UART_PARITY_NONE = 1,
	XMODEM_UART_PARITY_EVEN,
	XMODEM_UART_PARITY_ODD
};

enum
{
	XMODEM_UART_FLOW_CTRL_NONE = 1,
	XMODEM_UART_FLOW_CTRL_RTS_CTS
};

enum
{
	XMODEM_SPI_POLARITY_IDLE_LOW = 1,
	XMODEM_SPI_POLARITY_IDLE_HIGH
};

enum
{
	XMODEM_SPI_PHASE_SAMPLE_LEADING = 1,
	XMODEM_SPI_PHASE_SAMPLE_TRAILING
};

enum
{
	XMODEM_SPI_DEV_MASTER = 0,
	XMODEM_SPI_DEV_SLAVE,
	XMODEM_SPI_DEV_NUM_DEVS
};

typedef struct xmodem_uart_params
{
	uint16_t parity;
	uint16_t flow_control;
} xmodem_uart_params_t;

typedef struct xmodem_spi_params
{
	uint16_t polarity;
	uint16_t phase;
} xmodem_spi_params_t;

typedef struct conmon_audinate_upgrade_xmodem_params
{
	uint32_t port;
	uint32_t baud_rate;

	union {
		xmodem_uart_params_t uart;
		xmodem_spi_params_t spi[XMODEM_SPI_DEV_NUM_DEVS];
	} s_params;
} conmon_audinate_upgrade_xmodem_params_t;

enum
{
	CONMON_AUDINATE_UPGRADE_ADDITIONAL_TARGET_IP_PORT = 0x1,
};

typedef struct conmon_audinate_upgrade_additional_target
{
	uint16_t fields;
	uint16_t port;
	struct in_addr addr_inet;
} conmon_audinate_upgrade_additional_target_t;

/**
 * Get the status of an upgrade
 *
 * @param aud_msg A conmon message body containing an upgrade status message
 * @param len Size (in bytes) of the message body
 * @param status Status structure that will be initialised by message
 */
aud_error_t
conmon_audinate_upgrade_status_get_upgrade_status
(
	const conmon_message_body_t * aud_msg,
	size_t len,
	conmon_audinate_upgrade_status_t * status
);


/**
	Get information about the source file associated with an upgrade

	@param aud_msg A conmon message body containing an upgrade status message
	@param len Size (in bytes) of the message body
	@param status Status structure that will be initialised by message

	Note: the filename field in the updated file_info points into the message
	 body, and will go out of scope when the message body does.
 */
aud_error_t
conmon_audinate_upgrade_status_get_source_file_info
(
	const conmon_message_body_t * aud_msg,
	size_t len,
	conmon_audinate_upgrade_source_file_t * file_info
);

//----------------------------------------------------------
// device upgrade control
//----------------------------------------------------------

/**
	Initialise an empty upgrade control message

	@param aud_msg A conmon message body that will hold the message
	@param size Message size tracker with 'max' field initialised to the size of the
		message body buffer (or zero to use default buffer)
	@param congestion_delay_window_us the recommended maximum delay before sending a response
 */
aud_error_t
conmon_audinate_init_upgrade_control_v3
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	uint32_t congestion_delay_window_us
);

/**
	Set the source file for the upgrade

	@param aud_msg Initialised upgrade control message
	@param size Initialised message size tracker
	@param file_info File info to add to message

	Enables the flash action. Enables the download action if a file download protocol
	(eg tftp) is chosen.
 */
aud_error_t
conmon_audinate_upgrade_control_set_source_file
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	const conmon_audinate_upgrade_source_file_t * file_info
);

/**
	Sets the XModem parameters for the upgrade

	@param aud_msg Initialised upgrade control message
	@param size Initialised message size tracker
	@param xm_params XModem parameters to add to message (baud rate, file size, UART related params, SPI related params, etc.)

	Enables the flash action and download action.
*/
aud_error_t
conmon_audinate_upgrade_control_set_xmodem_params
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	const conmon_audinate_upgrade_xmodem_params_t * xm_params
);

/**
	Sets the additional target information so that upgrade status messages can be sent to this target

	@param aud_msg Initialised upgrade control message
	@param size Initialised message size tracker
	@param target Additional target information 
*/
aud_error_t
conmon_audinate_upgrade_control_set_additional_target
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	const conmon_audinate_upgrade_additional_target_t * target
);

/**
	Override the manufacturer and/or model identifier.

	@param aud_msg Initialised upgrade control message
	@param manufacturer If non-null, apply upgrade as if the manufacturer field
		had this value
	@param model If non-null, apply upgrade as if the model field
		had this value

	Note: overriding to zero ( &k_dante_id64_zero ) will accept any file
 */
aud_error_t
conmon_audinate_upgrade_control_set_override
(
	conmon_message_body_t * aud_msg,
	const dante_id64_t * manufacturer,
	const dante_id64_t * model
);

/**
Set the additional target 

@param aud_msg Initialised upgrade control message
@param size Initialised message size tracker
@param target Additional target information to send the upgrade status message to (e.g. IP address, port)

Enables the flash action and download action.
*/
aud_error_t
conmon_audinate_upgrade_control_set_additional_target
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	const conmon_audinate_upgrade_additional_target_t * target
);

/**
	Flags to control what upgrade actions run.

	Note: this interface is not needed in normal operation

	The accessors above normally enable all appropriate actions by default.
	These flags exist to allow specific actions to be turned off.
	Calls to other set functions (especially set_source_file) made after setting
	actions may reset actions to default values.
 */
enum
{
	CONMON_AUDINATE_UPGRADE_CONTROL__ACTION_DOWNLOAD = 0x1,
		//!< Setting this flag enables the download action
	CONMON_AUDINATE_UPGRADE_CONTROL__ACTION_FLASH = 0x2,
		//!< Setting this flag enables the flash action

	CONMON_AUDINATE_UPGRADE_CONTROL__ALL = 0x3
};

typedef uint32_t conmon_audinate_upgrade_control_actions_t;

/**
	Get bit-mask marking enabled actions.

	@param aud_msg Initialised upgrade control message
 */
conmon_audinate_upgrade_control_actions_t
conmon_audinate_upgrade_control_message_get_actions
(
	const conmon_message_body_t * aud_msg
);

/**
	Set bit-mask marking enabled actions.

	@param aud_msg Initialised upgrade control message
	@param actions replaces the current enabled actions
 */
void
conmon_audinate_upgrade_control_message_set_actions
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_upgrade_control_actions_t actions
);

/**
	Enable or disable the specified actions.

	@param aud_msg Initialised upgrade control message
	@param actions bit-mask of actions to modify. Other actions are not affected.
	@param enable whether the indicated actions are to be enabled or disabled
 */
void
conmon_audinate_upgrade_control_message_set_actions_enable
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_upgrade_control_actions_t actions,
	aud_bool_t enable
);

/**
	Get information about the source file associated with an upgrade control message

	@param aud_msg Initialised upgrade control message
	@param len len Size (in bytes) of the message body
	@param file_info information about the upgrade source file
*/
aud_error_t
conmon_audinate_upgrade_control_get_source_file_info
(
	const conmon_message_body_t * aud_msg,
	size_t len,
	conmon_audinate_upgrade_source_file_t * file_info
);

/**
	Get information about the manufacturer and/or model override control message

	@param aud_msg Initialised upgrade control message
	@param manufacturer The override manufacturer field
	@param model The override model field
*/
aud_error_t
conmon_audinate_upgrade_control_get_override
(
	const conmon_message_body_t * aud_msg,
	dante_id64_t * manufacturer,
	dante_id64_t * model
);


//----------------------------------------------------------
// device upgrade status (legacy)
//----------------------------------------------------------

/**
 * Get the status of an upgrade
 *
 * @param aud_msg A conmon message body containing an upgrade status message
 */
uint16_t
conmon_audinate_upgrade_status_get_status
(
	const conmon_message_body_t * aud_msg
);


  /** the length of an upgrade path */
#define CONMON_AUDINATE_UPGRADE_PATH_LEN 64

/**
 * Initialise an upgrade control message
 *
 * @param aud_msg A conmon message body that will hold the upgrade control message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 * @param server the address of the upgrade server in HOST order
 * @param port the port for the upgrade server
 * @param path the path to the upgrade file on the upgrade server. MAX supported length
 *   including the trailing NULL terminate is CONMON_AUDINATE_UPGRADE_PATH_LEN
 */
void
conmon_audinate_init_upgrade_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us,
	uint32_t server,
	uint16_t port,
	char * path
);

/**
 * Get the size of an upgrade control message
 *
 * @param aud_msg A conmon message body containing an upgrade control message
 */
uint16_t
conmon_audinate_upgrade_control_get_size
(
	const conmon_message_body_t * aud_msg
);

// clear config control

/** Clear config
 * Enumerated types for clearing the configuration of a device.
 */
enum conmon_clear_config_index
{
	CONMON_AUDINATE_CLEAR_CONFIG_CLEAR_ALL_INDEX = 0,
		//!< clear all of the configuration (name, IP config etc).
	CONMON_AUDINATE_CLEAR_CONFIG_KEEP_IP_INDEX   = 1
		//!< Clear all the config except the IP config (IP address, redundancy and VLAN config) and DDM configuration (if enrolled).
};

/** Clear config actions
 * Specify which parts of the configuration should be cleared,
 * or have been cleared.
 * These options are multually exculsive.
 */
enum conmon_clear_config
{
	CONMON_AUDINATE_CLEAR_CONFIG_QUERY        = 0x0,
	CONMON_AUDINATE_CLEAR_CONFIG_CLEAR_ALL    =  (1<<CONMON_AUDINATE_CLEAR_CONFIG_CLEAR_ALL_INDEX),
	CONMON_AUDINATE_CLEAR_CONFIG_KEEP_IP      =  (1<<CONMON_AUDINATE_CLEAR_CONFIG_KEEP_IP_INDEX)
};

typedef uint32_t conmon_audinate_clear_config_action_t;

/**
	Initialise an empty clear config control message

	@param aud_msg A conmon message body that will hold the message
	@param size Message size tracker with 'max' field initialised to the size of the
		message body buffer (or zero to use default buffer)
	@param congestion_delay_window_us the recommended maximum delay before sending a response
 */
aud_error_t
conmon_audinate_init_clear_config_control
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	uint32_t congestion_delay_window_us
);

/**
	Set the flags and fields to clear of the config

	@param aud_msg Initialised clear config control message
	@param size Initialised message size tracker
	@param flags bitmask of the config sections to clear
	@param valid_mask bitmask indicating which bits of the flags are valid
 */
aud_error_t
conmon_audinate_clear_config_control_set_action
(
	conmon_message_body_t * aud_msg,
	conmon_message_size_info_t * size,
	conmon_audinate_clear_config_action_t action
);

aud_error_t
conmon_audinate_clear_config_status_get_status
(
	const conmon_message_body_t * aud_msg,
	size_t len,
	conmon_audinate_clear_config_action_t * supported,
	conmon_audinate_clear_config_action_t * executed
);

//----------------------------------------------------------
// GPIO control
//----------------------------------------------------------

enum
{
	CONMON_AUDINATE_GPIO_CONTROL_FIELD_QUERY_STATE 	  = 0x0000,
	CONMON_AUDINATE_GPIO_CONTROL_FIELD_OUTPUT_STATE   = 0x0001
};

enum
{
	CONMON_AUDINATE_GPIO_STATUS_FIELD_QUERY_STATE 	  = 0x0000,
	CONMON_AUDINATE_GPIO_STATUS_FIELD_OUTPUT_STATE   = 0x0001,
	CONMON_AUDINATE_GPIO_STATUS_FIELD_INPUT_STATE    = 0x0002,
	CONMON_AUDINATE_GPIO_STATUS_FIELD_INTERRUPT_STATE   = 0x0003
};

void
conmon_audinate_init_gpio_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us,
	uint16_t fields,
	uint16_t gpio_state_num
);

uint16_t
conmon_audinate_gpio_control_get_fields
(
	conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_gpio_control_get_state_num
(
	conmon_message_body_t * aud_msg
);

void
conmon_audinate_gpio_control_state_set_at_index
(
	conmon_message_body_t * aud_msg,
	uint16_t index,
	uint32_t output_state_valid_mask,
	uint32_t output_state_values
);

void
conmon_audinate_gpio_control_state_get_at_index
(
	conmon_message_body_t * aud_msg,
	uint16_t index,
	uint32_t *output_state_valid_mask,
	uint32_t *output_state_values
);

uint16_t
conmon_audinate_gpio_control_get_size
(
	conmon_message_body_t * aud_msg
);



uint16_t
conmon_audinate_gpio_status_get_fields
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_gpio_status_get_state_num
(
	const conmon_message_body_t * aud_msg
);

void
conmon_audinate_gpio_status_state_get_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index,
	uint32_t *gpio_trigger_mask,
	uint32_t *gpio_input_mask,
	uint32_t *gpio_input_value,
	uint32_t *gpio_output_mask,
	uint32_t *gpio_output_value
);

//----------------------------------------------------------
// flash config control
//----------------------------------------------------------

/*
	Use this message to notify the flash config that the data is dirty and
	needs to be written to flash.

	The Dirty bit indicates that the current data on disk is dirty and should
	be written at a convenient time.  Use this after changing the config.

	The Write Now bit indicates that the config should be saved to flash
	immediately.  Use this if no further changes are expected in the near future.
 */

enum
{
	CONMON_AUDINATE_CONFIG_DIRTY = 0x01,
		// File has changed on disk (is dirty)
	CONMON_AUDINATE_CONFIG_WRITE_NOW = 0x02,
		// Disk should be saved to flash (or equivalent) immediately

	CONMON_AUDINATE_CONFIG_DIRTY_WRITE_NOW =
		CONMON_AUDINATE_CONFIG_DIRTY | CONMON_AUDINATE_CONFIG_WRITE_NOW
		// Convenience value for combining above flags - most applications will
		// want this rather than just CONMON_AUDINATE_CONFIG_WRITE_NOW.
};


/**
 * Initialise a config control message.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 * @param state config state bit-mask (see above)
 */
void
conmon_audinate_init_config_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us,
	uint8_t state
);


// Get the current config
uint8_t
conmon_audinate_config_control_get_state
(
	const conmon_message_body_t * aud_msg
);


uint16_t
conmon_audinate_config_control_get_size
(
	const conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// edk board control and status
//----------------------------------------------------------
//

enum
{
        CONMON_AUDINATE_EDK_BOARD_REV_OTHER = 0x0000,
        CONMON_AUDINATE_EDK_BOARD_REV_GREEN = 0x0001,
        CONMON_AUDINATE_EDK_BOARD_REV_RED = 0x0002,
        CONMON_AUDINATE_EDK_BOARD_REV_MAX
};

enum
{
        CONMON_AUDINATE_EDK_BOARD_PAD_0DB = 0,
        CONMON_AUDINATE_EDK_BOARD_PAD_MINUS6DB = 6,
        CONMON_AUDINATE_EDK_BOARD_PAD_MINUS12DB = 12,
        CONMON_AUDINATE_EDK_BOARD_PAD_MINUS24DB = 24,
        CONMON_AUDINATE_EDK_BOARD_PAD_MINUS48DB = 48,
        CONMON_AUDINATE_EDK_BOARD_PAD_MAX = 127
};

enum
{
        CONMON_AUDINATE_EDK_BOARD_DIG_SPDIF = 0x0001,
        CONMON_AUDINATE_EDK_BOARD_DIG_AES = 0x0002,
        CONMON_AUDINATE_EDK_BOARD_DIG_TOSLINK = 0x0003
};

enum
{
        CONMON_AUDINATE_EDK_BOARD_SRC_SYNC = 0x0000,
        CONMON_AUDINATE_EDK_BOARD_SRC_ASYNC = 0x0001
};

enum
{
	CONMON_AUDINATE_EDK_BOARD_SRATE_44K = 44100,
	CONMON_AUDINATE_EDK_BOARD_SRATE_48K = 48000,
	CONMON_AUDINATE_EDK_BOARD_SRATE_96K = 96000,
	CONMON_AUDINATE_EDK_BOARD_SRATE_192K = 192000,
	CONMON_AUDINATE_EDK_BOARD_SRATE_88K = 88200,
	CONMON_AUDINATE_EDK_BOARD_SRATE_176K = 176400
};


uint16_t
conmon_audinate_edk_board_status_get_rev
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_edk_board_status_get_pad
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_edk_board_status_get_dig
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_edk_board_status_get_src
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise an EDK board control message but do not set any of the fields.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_edk_board_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);


/**
 * Set the board rev field in the edk board control message
 *
 * @param aud_msg a conmon message body containing an edk board control message
 * @param rev the new board rev value
 */
void
conmon_audinate_edk_board_set_rev
(
	conmon_message_body_t * aud_msg,
	uint16_t rev
);

/**
 * Set the board pad field in the edk board control message
 *
 * @param aud_msg a conmon message body containing an edk board control message
 * @param pad the new board pad value
 */
void
conmon_audinate_edk_board_set_pad
(
	conmon_message_body_t * aud_msg,
	uint16_t pad
);

/**
 * Set the board dig field in the edk board control message
 *
 * @param aud_msg a conmon message body containing an edk board control message
 * @param dig the new board dig value
 */
void
conmon_audinate_edk_board_set_dig
(
	conmon_message_body_t * aud_msg,
	uint16_t dig
);

/**
 * Set the board src field in the edk board control message
 *
 * @param aud_msg a conmon message body containing an edk board control message
 * @param src the new board src value
 */
void
conmon_audinate_edk_board_set_src
(
	conmon_message_body_t * aud_msg,
	uint16_t src
);

/**
 * Get the size of an edk board control message
 *
 * @param aud_msg a conmon message body containing an edk board control message
 */
uint16_t
conmon_audinate_edk_board_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// rx error threshold control
//----------------------------------------------------------
//

// threshold of missing samples that trigger an audio rx error (samples)
uint16_t
conmon_audinate_rx_error_threshold_status_get_threshold
(
	const conmon_message_body_t * aud_msg
);

// window over which missing samples are measured (samples)
uint16_t
conmon_audinate_rx_error_threshold_status_get_window
(
	const conmon_message_body_t * aud_msg
);

// time to wait for no errors before restoring rx channels in error (seconds)
uint16_t
conmon_audinate_rx_error_threshold_status_get_reset_time
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise an rx error threshold message body that will hold the message
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_rx_error_threshold_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

// set the rx error threshold in samples
void
conmon_audinate_rx_error_threshold_set_threshold
(
	conmon_message_body_t * aud_msg,
	uint16_t threshold
);

// set the rx error window in samples
void
conmon_audinate_rx_error_threshold_set_window
(
	conmon_message_body_t * aud_msg,
	uint16_t window
);

// set time to wait for no errors before restoring rx channels in error (seconds)
void
conmon_audinate_rx_error_threshold_set_reset_time
(
	conmon_message_body_t * aud_msg,
	uint16_t seconds
);


uint16_t
conmon_audinate_rx_error_threshold_control
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_rx_error_threshold_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// LED state
//----------------------------------------------------------

enum
{
	CONMON_AUDINATE_LED_TYPE_UNKNOWN = 0x0000,
	CONMON_AUDINATE_LED_TYPE_SYSTEM  = 0x0001,
	CONMON_AUDINATE_LED_TYPE_SYNC    = 0x0002,
	CONMON_AUDINATE_LED_TYPE_SERIAL  = 0x0003,
	CONMON_AUDINATE_LED_TYPE_ERROR   = 0x0004
};

enum
{
	CONMON_AUDINATE_LED_COLOUR_NONE         = 0x0000,
	CONMON_AUDINATE_LED_COLOUR_GREEN        = 0x0001,
	CONMON_AUDINATE_LED_COLOUR_ORANGE       = 0x0002
};

enum
{
	CONMON_AUDINATE_LED_STATE_OFF          = 0x0000,
	CONMON_AUDINATE_LED_STATE_ON           = 0x0001,
	CONMON_AUDINATE_LED_STATE_BLINK        = 0x0002
};

typedef uint16_t conmon_audinate_led_type_t;
typedef uint16_t conmon_audinate_led_colour_t;
typedef uint16_t conmon_audinate_led_state_t;

uint16_t
conmon_audinate_led_status_num_leds
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_led_type_t
conmon_audinate_led_status_led_type_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

conmon_audinate_led_colour_t
conmon_audinate_led_status_led_colour_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

conmon_audinate_led_state_t
conmon_audinate_led_status_led_state_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

//----------------------------------------------------------
// Metering
//----------------------------------------------------------
#if ( !(defined CONMON_HAS_NO_METERING) || !(CONMON_HAS_NO_METERING == 1) )
float32_t
conmon_audinate_metering_status_get_peak_holdoff
(
	const conmon_message_body_t * aud_msg
);

float32_t
conmon_audinate_metering_status_get_peak_decay
(
	const conmon_message_body_t * aud_msg
);

uint32_t
conmon_audinate_metering_status_get_update_rate
(
	const conmon_message_body_t * aud_msg
);

void
conmon_audinate_init_metering_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Currently unsupported
 */
void
conmon_audinate_metering_control_set_peak_holdoff
(
	conmon_message_body_t * aud_msg,
	float32_t peak_holdoff
);

/**
 * Currently unsupported
 */
void
conmon_audinate_metering_control_set_peak_decay
(
	conmon_message_body_t * aud_msg,
	float32_t peak_decay
);

void
conmon_audinate_metering_control_set_update_rate
(
	conmon_message_body_t * aud_msg,
	uint32_t update_rate
);

uint16_t
conmon_audinate_metering_control_get_size
(
	const conmon_message_body_t * aud_msg
);
#endif //#if ( !(defined CONMON_HAS_NO_METERING) || !(CONMON_HAS_NO_METERING == 1) )
//----------------------------------------------------------
// Serial Port status and configuration
// * Serial port status messages describe each port as well as the
//   available configuration options for those ports
// * Serial port control messages allow either querying of
//   serial port information or configuration of a single port
//----------------------------------------------------------

enum conmon_serial_port_parity
{
	CONMON_AUDINATE_SERIAL_PORT_PARITY_EVEN,
	CONMON_AUDINATE_SERIAL_PORT_PARITY_ODD,
	CONMON_AUDINATE_SERIAL_PORT_PARITY_NONE
};

// what is the serial port attached to?
enum 
{
	CONMON_AUDINATE_SERIAL_PORT_MODE_UNKNOWN,
	CONMON_AUDINATE_SERIAL_PORT_MODE_UNATTACHED,
	CONMON_AUDINATE_SERIAL_PORT_MODE_CONSOLE,
	CONMON_AUDINATE_SERIAL_PORT_MODE_BRIDGE,
	CONMON_AUDINATE_SERIAL_PORT_MODE_METERING,
	CONMON_AUDINATE_SERIAL_PORT_MODE_COUNT
};

typedef uint16_t conmon_audinate_serial_port_mode_t;
typedef uint32_t conmon_audinate_serial_port_baud_rate_t;
typedef uint8_t conmon_audinate_serial_port_bits_t;
typedef uint8_t conmon_audinate_serial_port_parity_t;
typedef uint8_t conmon_audinate_serial_port_stop_bits_t;

typedef struct conmon_audinate_serial_port conmon_audinate_serial_port_t;

uint16_t
conmon_audinate_serial_port_get_index
(
	const conmon_audinate_serial_port_t * serial_port
);

conmon_audinate_serial_port_mode_t
conmon_audinate_serial_port_get_mode
(
	const conmon_audinate_serial_port_t * serial_port
);

conmon_audinate_serial_port_baud_rate_t
conmon_audinate_serial_port_get_baud_rate
(
	const conmon_audinate_serial_port_t * serial_port
);

conmon_audinate_serial_port_bits_t
conmon_audinate_serial_port_get_bits
(
	const conmon_audinate_serial_port_t * serial_port
);

conmon_audinate_serial_port_parity_t
conmon_audinate_serial_port_get_parity
(
	const conmon_audinate_serial_port_t * serial_port
);

conmon_audinate_serial_port_stop_bits_t
conmon_audinate_serial_port_get_stop_bits
(
	const conmon_audinate_serial_port_t * serial_port
);

aud_bool_t
conmon_audinate_serial_port_is_configurable
(
	const conmon_audinate_serial_port_t * serial_port
);

uint16_t
conmon_audinate_serial_port_status_num_ports
(
	const conmon_message_body_t * aud_msg
);

const conmon_audinate_serial_port_t *
conmon_audinate_serial_port_status_port_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

uint16_t
conmon_audinate_serial_port_status_num_available_baud_rates
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_serial_port_baud_rate_t
conmon_audinate_serial_port_status_available_baud_rate_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

uint16_t
conmon_audinate_serial_port_status_num_available_bits
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_serial_port_bits_t
conmon_audinate_serial_port_status_available_bits_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

uint16_t
conmon_audinate_serial_port_status_num_available_parities
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_serial_port_parity_t
conmon_audinate_serial_port_status_available_parity_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

uint16_t
conmon_audinate_serial_port_status_num_available_stop_bits
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_serial_port_stop_bits_t
conmon_audinate_serial_port_status_available_stop_bits_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

/**
 * initialise a serial port control message.
 * If no port is configured then the message is
 * treated as a query.
 *
 * @param aud_msg A conmon message body that will hold the message
 * @param congestion_delay_window_us the recommended maximum delay before sending a response
 */
void
conmon_audinate_init_serial_port_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Add configuration data for a serial port to the serial port control message.
 * Only one port may be configured at a time.
 *
 * @param aud_msg the message
 * @param port_index the index of the port to be configured (not the index in the packet)
 * @param baud_rate the new baud rate settings for the serial port
 * @param bits the new bits setting for the serial port
 * @param parity the new parity setting for the serial port
 * @param stop_bits the new stop bits setting for the serial port
 */
void
conmon_audinate_serial_port_control_set_port
(
	conmon_message_body_t * aud_msg,
	uint16_t port_index,
	conmon_audinate_serial_port_baud_rate_t baud_rate,
	conmon_audinate_serial_port_bits_t bits,
	conmon_audinate_serial_port_parity_t parity,
	conmon_audinate_serial_port_stop_bits_t stop_bits
);

/**
 * Get the size of a serial port control message
 *
 * @param aud_msg the message
 */
uint16_t
conmon_audinate_serial_port_control_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Identify status
//----------------------------------------------------------

/**
 * Initialise a ConMon message as an identify status message
 * with the given type and congestion delay
 *
 * @param aud_msg a conmon message body that will contain the interface status message
 */
void
conmon_audinate_init_identify_status
(
	conmon_message_body_t * aud_msg
);

/**
 * Get the size (in bytes) of a identify status message
 *
 * @param aud_msg a conmon message body containing an identify status message
 */
uint16_t
conmon_audinate_identify_status_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// HA Remote (client-side)
//----------------------------------------------------------

enum
{
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_ALL          = 0,
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NONE         = 1,
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_SLOT_DB9     = 2,
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_SLOT = 3,
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_DB9  = 4,
	CONMON_AUDINATE_HAREMOTE_NUM_BRIDGE_MODES         = 5
};

enum
{
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_FLAG_ALL          = (1 << CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_ALL),
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_FLAG_NONE         = (1 << CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NONE),
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_FLAG_SLOT_DB9     = (1 << CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_SLOT_DB9),
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_FLAG_NETWORK_SLOT = (1 << CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_SLOT),
	CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_FLAG_NETWORK_DB9  = (1 << CONMON_AUDINATE_HAREMOTE_BRIDGE_MODE_NETWORK_DB9)
};

typedef uint16_t conmon_audinate_haremote_bridge_mode_t;
typedef uint32_t conmon_audinate_haremote_bridge_mode_flags_t;

/**
 * Get a bitfield of the supported bridging modes for this device.
 */
conmon_audinate_haremote_bridge_mode_flags_t
conmon_audinate_haremote_status_get_supported_bridge_modes
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get the currently active brdging mode for this device
 */
conmon_audinate_haremote_bridge_mode_t
conmon_audinate_haremote_status_get_bridge_mode
(
	const conmon_message_body_t * aud_msg
);

/**
 * Initialise a HA Remote control message
 */
void
conmon_audinate_init_haremote_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

/**
 * Set the HA Remote bridge mode for a device
 */
void
conmon_audinate_haremote_control_set_bridge_mode
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_haremote_bridge_mode_t bridge_mode
);

/**
 * Get the size of a HARemote control message
 */
uint16_t
conmon_audinate_haremote_control_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// HA Remote (server-side)
//----------------------------------------------------------

// control flags for interfaces indicating which fields are set...
enum
{
	// this flag is set if the 'bridge mode' field has a meaningful value
	CONMON_AUDINATE_HAREMOTE_CONTROL_FIELD_BRIDGE_MODE = (1 << 0)
};

typedef uint32_t conmon_audinate_haremote_control_fields_t;

conmon_audinate_haremote_control_fields_t
conmon_audinate_haremote_control_get_fields
(
	const conmon_message_body_t * aud_msg
);

conmon_audinate_haremote_bridge_mode_t
conmon_audinate_haremote_control_get_bridge_mode
(
	const conmon_message_body_t * aud_msg
);

void
conmon_audinate_init_haremote_status
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_haremote_bridge_mode_flags_t supported_bridge_modes
);

void
conmon_audinate_haremote_status_set_bridge_mode
(
	conmon_message_body_t * aud_msg,
	conmon_audinate_haremote_bridge_mode_t bridge_mode
);

uint16_t
conmon_audinate_haremote_status_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// HA Remote stats (client-side)
//----------------------------------------------------------

typedef struct conmon_audinate_haremote_port_stats conmon_audinate_haremote_port_stats_t;

uint16_t
conmon_audinate_haremote_stats_status_num_ports
(
	const conmon_message_body_t * aud_msg
);

const conmon_audinate_haremote_port_stats_t *
conmon_audinate_haremote_stats_status_port_stats_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

int
conmon_audinate_haremote_port_stats_get_port_number
(
	const conmon_audinate_haremote_port_stats_t * port_stats
);

uint32_t
conmon_audinate_haremote_port_stats_num_recv_packets
(
	const conmon_audinate_haremote_port_stats_t * port_stats
);

uint32_t
conmon_audinate_haremote_port_stats_num_sent_packets
(
	const conmon_audinate_haremote_port_stats_t * port_stats
);

uint32_t
conmon_audinate_haremote_port_stats_num_checksum_fails
(
	const conmon_audinate_haremote_port_stats_t * port_stats
);

uint32_t
conmon_audinate_haremote_port_stats_num_timeouts
(
	const conmon_audinate_haremote_port_stats_t * port_stats
);

//----------------------------------------------------------
// HA Remote stats (server-size)
//----------------------------------------------------------

void
conmon_audinate_init_haremote_stats_status
(
	conmon_message_body_t * aud_msg,
	uint16_t num_uarts
);

aud_error_t
conmon_audinate_haremote_stats_status_set_uart_stats_at_index
(
	conmon_message_body_t * aud_msg,
	unsigned int index, // the index in the packet (must be < num_uarts above
	int port_number,
	uint32_t num_recv_packets, // packets recved from the uart (and sent to network / other uarts)
	uint32_t num_sent_packets, // packets forwarded to the uart from network / other uarts
	uint32_t num_checksum_fails,
	uint32_t num_timeouts
);

uint16_t
conmon_audinate_haremote_stats_status_get_size
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Switch VLAN control / status
//----------------------------------------------------------

typedef struct conmon_audinate_switch_vlan_config conmon_audinate_switch_vlan_config_t;

typedef uint32_t conmon_audinate_switch_vlan_port_mask_t;

enum
{
	CONMON_AUDINATE_SWITCH_VLAN_PRIMARY,
	CONMON_AUDINATE_SWITCH_VLAN_SECONDARY,
	CONMON_AUDINATE_SWITCH_VLAN_USER2,
	CONMON_AUDINATE_SWITCH_VLAN_USER3
};

uint16_t
conmon_audinate_switch_vlan_status_get_current_config_id
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_switch_vlan_status_get_reboot_config_id
(
	const conmon_message_body_t * aud_msg
);

/**
 * How many distinct vlans are supported for any given option
 */
uint16_t
conmon_audinate_switch_vlan_status_max_vlans
(
	const conmon_message_body_t * aud_msg
);

/**
 * Get a mask containing all valid switch ports
 */
conmon_audinate_switch_vlan_port_mask_t
conmon_audinate_switch_vlan_status_get_ports_mask
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_switch_vlan_status_num_configs
(
	const conmon_message_body_t * aud_msg
);

const conmon_audinate_switch_vlan_config_t *
conmon_audinate_switch_vlan_status_config_at_index
(
	const conmon_message_body_t * aud_msg,
	uint16_t index
);

uint16_t
conmon_audinate_switch_vlan_config_get_id
(
	const conmon_audinate_switch_vlan_config_t * config,
	const conmon_message_body_t * aud_msg
);

/**
 * UTF-8 string containing a friendly name for the config
 */
const char *
conmon_audinate_switch_vlan_config_get_name
(
	const conmon_audinate_switch_vlan_config_t * config,
	const conmon_message_body_t * aud_msg
);

/**
 * Which ports are enabled for the given vlan, for this config
 */
conmon_audinate_switch_vlan_port_mask_t
conmon_audinate_switch_vlan_config_get_vlan_port_mask
(
	const conmon_audinate_switch_vlan_config_t * config,
	uint16_t vlan_index,
	const conmon_message_body_t * aud_msg
);


enum
{
	CONMON_AUDINATE_SWITCH_VLAN_CONTROL_FIELD_ID = 0
};

void
conmon_audinate_init_switch_vlan_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

uint16_t
conmon_audinate_switch_vlan_control_get_flags
(
	conmon_message_body_t * aud_msg
);

void
conmon_audinate_switch_vlan_control_set_config_id
(
	conmon_message_body_t * aud_msg,
	uint16_t id
);

uint16_t
conmon_audinate_switch_vlan_control_get_config_id
(
	const conmon_message_body_t * aud_msg
);

uint16_t
conmon_audinate_switch_vlan_control_get_size
(
	conmon_message_body_t * aud_msg
);


//----------------------------------------------------------
// AES67 control / status
//----------------------------------------------------------

/**
* Initialise a aes67 config control message but do not set any of the fields.
*
* @param aud_msg A conmon message body that will hold the message
* @param congestion_delay_window_us the recommended maximum delay before sending a response
*/
void
conmon_audinate_init_aes67_control
(
	conmon_message_body_t * aud_msg,
	uint32_t congestion_delay_window_us
);

void
conmon_audinate_aes67_control_set_enable
(
	conmon_message_body_t * aud_msg,
	aud_bool_t	enable
);

uint16_t
conmon_audinate_aes67_control_get_size
(
	const conmon_message_body_t * aud_msg
);

aud_bool_t
conmon_audinate_aes67_status_is_enabled
(
	const conmon_message_body_t * aud_msg
);

aud_bool_t
conmon_audinate_aes67_status_is_enabled_on_reboot
(
	const conmon_message_body_t * aud_msg
);

//----------------------------------------------------------
// Lock query / status
//----------------------------------------------------------

aud_bool_t
conmon_audinate_get_lock_status
(
	const conmon_message_body_t * aud_msg
);

#ifdef __cplusplus
}
#endif

#endif

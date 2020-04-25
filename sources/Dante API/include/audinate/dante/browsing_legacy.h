#ifndef _BROWSING_LEGACY_H
#define _BROWSING_LEGACY_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/dante_common.h"
#include "dante/browsing_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * How many channels have been discovered for this device?
 * This function returns the number of discovered channels, which
 * may not match the actual number of channels on the device.
 *
 * @param device device node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
unsigned int
db_browse_device_get_num_channels
(
	const db_browse_device_t * device
);

/**
 * Get the channel node at the given index for the device node.
 * The index refers to the channel's index as a child node in the
 * network tree and does correlate to the any actual channel indexes
 * on the discovered device.
 *
 * @param device device node
 * @param index index of channel node (from 0)
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_channel_t *
db_browse_device_channel_at_index
(
	const db_browse_device_t * device,
	unsigned int index
);

/**
 * Get the channel node with the given name for the device node.
 * The txname_type_t parameter specifies whether or not to check
 * a channel's canonical name and / or label names.  A device node
 * only contains channel child node when browsing for media channels.
 *
 * @param device device node
 * @param channel_name name of channel
 * @param txname_type type of name to check. Legal values are
 *   DR_TXNAME_TYPE_CANONICAL, DR_TXNAME_TYPE_LABEL and
 *   DR_TXNAME_TYPE_ALL
 *
 * @return NULL if no child channel exists with the given name
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_channel_t *
db_browse_device_channel_with_name
(
	const db_browse_device_t * device,
	const char * channel_name,
	dante_txname_type_t txname_type
);

/**
 * Get device node of this channel
 *
 * @param channel channel node
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_device_t *
db_browse_channel_get_device
(
	const db_browse_channel_t * channel
);

// Channels
//----------------------------------------------------------

/**
 * Get the types of information available for this channel across all
 * interfaces.
 * If the function returns '0' then the channel was not explictly
 * discovered, but exists to encapsulate a label that was
 * discovered for this channel's id
 *
 * @param channel channel node
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_channel_get_browse_types
(
	const db_browse_channel_t * channel
);

/**
 * Get the network interface bit mask for a specific browse type for this channel
 * If the function returns '0' then no channel information has been seen
 * for a particular browse type.
 *
 * @param channel chanel node
 * @param browse_types the browse types for which network interface mask is required
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
unsigned int
db_browse_channel_get_networks_for_browse_type
(
	const db_browse_channel_t * channel,
	db_browse_types_t types
);

/**
 * Get the types of information seen for this channel on the given network.
 * If the function returns '0' then no device information has been seen
 * on localhost
 *
 * @param channel channel node
 * @param network the network index. This is NOT an interface index
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_channel_get_browse_types_on_network
(
	const db_browse_channel_t * channel,
	unsigned int network
);

/**
 * Get the types of information seen for this channel on the localhost interface.
 * If the function returns '0' then no device information has been seen
 * on localhost
 *
 * @param channel channel node
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_channel_get_browse_types_on_localhost
(
	const db_browse_channel_t * channel
);

/**
 * Get the canonical name for this channel. A canonical name is
 * only known if a channel is explicitly discovered. That is, the
 * DB_BROWSE_TYPE_MEDIA_CHANNEL bit must be set when calling
 * db_browse_channel_get_browse_types is called for this channel.
 *
 * @param channel channel node
 *
 * @return the channel's canonical name if known, or NULL otherwise
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
const char *
db_browse_channel_get_canonical_name
(
	const db_browse_channel_t * channel
);

/**
 * Get the identifier of this channel.
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
dante_id_t
db_browse_channel_get_id
(
	const db_browse_channel_t * channel
);

/**
 * Get the set of formats supported by this channel
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
const dante_formats_t *
db_browse_channel_get_formats
(
	const db_browse_channel_t * channel
);


/**
 * Get the sample rate of this channel.
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
dante_samplerate_t
db_browse_channel_get_sample_rate
(
	const db_browse_channel_t * channel
);

/**
 * Get the number of sample encodings supported by this channel.
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
uint16_t
db_browse_channel_get_num_encodings
(
	const db_browse_channel_t * channel
);

/**
 * Get one sample encoding supported by this channel.
 *
 * @param channel channel node
 * @param index index of encoding (from 0).  Must be less than the number of
 *	encodings returned by get_num_encodings.
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
dante_encoding_t
db_browse_channel_encoding_at_index
(
	const db_browse_channel_t * channel,
	uint16_t index
);

/**
 * How many labels have been discovered for this channel?
 * This function returns the number of discovered labels, which
 * might not match the actual number of channel labels on the device.
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
unsigned int
db_browse_channel_get_num_labels
(
	const db_browse_channel_t * channel
);

/**
 * Get the label node at the given index for the channel node.
 * The index refers to the labels's index as a child node of the channel in
 * the network tree and does not correlate to label ordering on the device.
 *
 * @param channel channel node
 * @param index index of label (from 0)
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_label_t *
db_browse_channel_label_at_index
(
	const db_browse_channel_t * channel,
	unsigned int index
);

/**
 * Get user context for this channel node
 *
 * The context pointer provides a reference back into the caller's data structures.
 *
 * @param channel channel node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
void *
db_browse_channel_get_context
(
	const db_browse_channel_t * channel
);

/**
 * Set user context for this channel node
 *
 * @param channel channel node
 * @param context user context
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
void
db_browse_channel_set_context
(
	db_browse_channel_t * channel,
	void * context
);

/**
 * Re-confirm the validity of this channel (if explicitly discovered) and optionally all its children
 *
 * @param channel channel node
 * @param reconfirm_children should this nodes children also be reconfirmed?
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
aud_error_t
db_browse_channel_reconfirm
(
	db_browse_channel_t * channel,
	aud_bool_t reconfirm_children
);

// Labels
//----------------------------------------------------------

/**
 * Get the types of information available for this channel across all
 * interfaces.
 *
 * This function acts as future-proofing in case multiple information
 * sources or types subsequently become available for labels.
 *
 * @param label label node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_label_get_browse_types
(
	const db_browse_label_t * label
);

/**
 * Get the network interface bit mask for a specific browse type for this label
 * If the function returns '0' then no label information has been seen
 * for a particular browse type.
 *
 * @param label label node
 * @param browse_types the browse types for which network interface mask is required
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
unsigned int
db_browse_label_get_networks_for_browse_type
(
	const db_browse_label_t * label,
	db_browse_types_t types
);

/**
 * Get the types of information seen for this label on the given network.
 * If the function returns '0' then no device information has been seen
 * on localhost
 *
 * @param label label node
 * @param network the network index. This is NOT an interface index
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_label_get_browse_types_on_network
(
	const db_browse_label_t * label,
	unsigned int network
);

/**
 * Get the types of information seen for this label on the localhost interface.
 * If the function returns '0' then no device information has been seen
 * on localhost
 *
 * @param label label node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_types_t
db_browse_label_get_browse_types_on_localhost
(
	const db_browse_label_t * label
);

/**
 * Get the channel node associated with a label node.
 *
 * @param label label node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
db_browse_channel_t *
db_browse_label_get_channel
(
	const db_browse_label_t * label
);

/**
 * Get the name for this label. A label name is
 * only known if a label is explicitly discovered. That is, the
 * DB_BROWSE_TYPE_MEDIA_CHANNEL bit must be set when calling
 * db_browse_label_get_browse_types is called for this label.
 *
 * @param label label node
 *
 * @return the label's name if known, or NULL otherwise
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
const char *
db_browse_label_get_name
(
	const db_browse_label_t * label
);

/**
 * Get user context for this label node
 *
 * The context pointer provides a reference back into the caller's data structures.
 *
 * @param label label node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
void *
db_browse_label_get_context
(
	const db_browse_label_t * label
);

/**
 * Set user context for this label node
 *
 * @param label label node
 * @param context user context
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
void
db_browse_label_set_context
(
	db_browse_label_t * label,
	void * context
);

/**
 * Re-confirm the validity of this label
 *
 * @param label label node
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
aud_error_t
db_browse_label_reconfirm
(
	db_browse_label_t * label
);



#ifdef __cplusplus
}
#endif



#endif


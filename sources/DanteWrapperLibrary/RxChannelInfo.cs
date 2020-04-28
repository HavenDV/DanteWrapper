using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct InternalRxChannelInfo
    {
        public ushort id;
        public int stale;
        public string name;
        public string format;
        public string latency;
        public int muted;
        public short dbu;
        public string sub;
        public byte status;
        public string flow;
    }

    public enum RxStatus
    {
		/// <summary>
		/// Channel is not subscribed or otherwise doing anything interesting
		/// </summary>
		None = 0,

		/// <summary>
		/// Name not yet found
		/// </summary>
		Unresolved = 1,

		/// <summary>
		/// Name has been found, but not yet processed. This is an transient state
		/// </summary>
		Resolved = 2,

		/// <summary>
		/// Error: an error occurred while trying to resolve name
		/// </summary>
		ResolveFail = 3,

		/// <summary>
		/// Channel is successfully subscribed to own TX channels (local loopback mode)
		/// </summary>
		SubscribeSelf = 4,

		/// <summary>
		/// Name explicitly does not exist
		/// </summary>
		ResolvedNone = 5,

		/// <summary>
		/// A flow has been configured but does not have sufficient information
		/// to establish an audio connection.
		/// For example, configuring a template with no associations.
		/// </summary>
		Idle = 0x7,

		/// <summary>
		/// Name has been found and processed; setting up flow. This is an transient state
		/// </summary>
		InProgress = 0x8,

		/// <summary>
		/// Active subscritpion to an automatically configured source flow
		/// </summary>
		Dynamic = 9,

		/// <summary>
		/// Active subscription to a manually configured source flow
		/// </summary>
		Static = 10,

		/// <summary>
		/// Manual flow configuration bypassing the standard subscription process
		/// </summary>
		Manual = 0xE,

		/// <summary>
		/// Error: The name was found but the connection process failed(the receiver could not communicate with the transmitter)
		/// </summary>
		NoConnection = 0xF,

		/// <summary>
		/// Error: Channel formats do not match
		/// </summary>
		ChannelFormat,

		/// <summary>
		/// Error: Flow formats do not match, e.g. Multicast flow with more slots than receiving device can handle
		/// </summary>
		BundleFormat,

		/// <summary>
		/// Error: Receiver is out of resources (e.g. flows)
		/// </summary>
		NoRx,

		/// <summary>
		/// Error: Receiver couldn't set up the flow
		/// </summary>
		RxFail,

		/// <summary>
		/// Error: Transmitter is out of resources (e.g. flows)
		/// </summary>
		NoTx,

		/// <summary>
		/// Error: Transmitter couldn't set up the flow
		/// </summary>
		TxFail,

		/// <summary>
		/// Error: Receiver got a QoS failure (too much data) when setting up the flow.
		/// </summary>
		QosFailRx,

		/// <summary>
		/// Error: Transmitter got a QoS failure (too much data) when setting up the flow.
		/// </summary>
		QosFailTx,

		/// <summary>
		/// Error: TX rejected the address given by rx (usually indicates an arp failure)
		/// </summary>
		TxRejectedAddr,

		/// <summary>
		/// Error: Transmitter rejected the bundle request as invalid
		/// </summary>
		InvalidMsg,

		/// <summary>
		/// Error: TX channel latency higher than maximum supported RX latency
		/// </summary>
		ChannelLatency,

		/// <summary>
		/// Error: TX and RX and in different clock subdomains
		/// </summary>
		ClockDomain,

		/// <summary>
		/// Error: Attempt to use an unsupported feature
		/// </summary>
		Unsupported,

		/// <summary>
		/// Error: All rx links are down
		/// </summary>
		RxLinkDown,

		/// <summary>
		/// Error: All tx links are down
		/// </summary>
		TxLinkDown,

		/// <summary>
		/// Error: can't find suitable protocol for dynamic connection
		/// </summary>
		DynamicProtocol,

		/// <summary>
		/// Channel does not exist (eg no such local channel)
		/// </summary>
		InvalidChannel,

		/// <summary>
		/// Tx Scheduler failure
		/// </summary>
		TxSchedulerFailure,

		/// <summary>
		/// The given subscription to self was disallowed by the device
		/// </summary>
		SubscribeSelfPolicy,

		/// <summary>
		/// Template-based subscription failed: template and subscription device names don't match
		/// </summary>
		TemplateMismatchDevice = 0x40,

		/// <summary>
		/// Template-based subscription failed: flow and channel formats don't match
		/// </summary>
		TemplateMismatchFormat,

		/// <summary>
		/// Template-based subscription failed: the channel is not part of the given multicast flow
		/// </summary>
		TemplateMissingChannel,

		/// <summary>
		/// Template-based subscription failed: something else about the template configuration
		/// made it impossible to complete the subscription using the given flow
		/// </summary>
	    TemplateMismatchConfig,

		/// <summary>
		/// Template-based subscription failed: the unicast template is full
		/// </summary>
		TemplateFull,

		/// <summary>
		/// Error: RX device does not have a supported subscription mode (unicast/multicast) available
		/// </summary>
		RxUnsupportedSubMode,

		/// <summary>
		/// Error: TX device does not have a supported subscription mode (unicast/multicast) available
		/// </summary>
		TxUnsupportedSubMode,

		/// <summary>
		/// Error: TX access control denied the request
		/// </summary>
		TxAccessControlDenied = 0x60,

		/// <summary>
		/// TX access control request is in progress
		/// </summary>
		TxAccessControlPending,

		/// <summary>
		/// Unexpected system failure.
		/// </summary>
		SystemFail = 0xFF
	};

    public class RxChannelInfo
    {
        public int Id { get; }
        public bool IsStale { get; }
        public string Name { get; }
        public string Format { get; }
        public string Latency { get; }
        public bool IsMuted { get; }

        /// <summary>
        /// -1 if it's unset <br/>
        /// -2 if it's invalid/uninitialised
        /// </summary>
        public int Dbu { get; }
        public string Sub { get; }
        public RxStatus Status { get; }
        public string Flow { get; }

        public RxChannelInfo(int id, bool isStale, string name, string format, string latency, bool isMuted, int dbu, string sub, int status, string flow)
        {
            Id = id;
            IsStale = isStale;
            Name = name;
            Format = format;
            Latency = latency;
            IsMuted = isMuted;
            Dbu = dbu switch
            {
                0x7FFF => -1,
                0x7FFE => -2,
                _ => dbu
            };
            Sub = sub;
            Status = (RxStatus)status;
            Flow = flow;
        }
    }
}

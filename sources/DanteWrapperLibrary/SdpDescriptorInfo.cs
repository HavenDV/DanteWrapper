using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct InternalSdpDescriptorGroupInfo
    {
        public string address;
        public ushort port;
        public string id;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct InternalSdpDescriptorInfo
    {
        public string username;
        public string session_name;
        public ulong session_id;
        public string session_originator_address;
        public int is_dante;
        public uint media_clock_offset;
        public byte stream_payload_type;
        public int groups_count;
        public IntPtr groups;
        public string gmid;
        public string sub_domain;
        public uint stream_sample_rate;
        public ushort stream_encoding;
        public ushort stream_num_chans;
        public SdpStreamDirection stream_dir;
    }

    public enum SdpStreamDirection
    {
        Undefined,
        ReceiveOnly,
        SendReceive,
        SendOnly,
    };

    public class SdpDescriptorGroupInfo
    {
        public string Address { get; }
        public ushort Port { get; }
        public string Id { get; }

        public SdpDescriptorGroupInfo(string address, ushort port, string id)
        {
            Address = address;
            Port = port;
            Id = id;
        }
    }

    public class SdpDescriptorInfo
    {
        public string Username { get; }
        public string SessionName { get; }
        public ulong SessionId { get; }
        public string SessionOriginatorAddress { get; }
        public bool IsDante { get; }
        public uint MediaClockOffset { get; }
        public byte StreamPayloadType { get; }
        public IList<SdpDescriptorGroupInfo> Groups { get; }
        public string GMid { get; }
        public string SubDomain { get; }
        public uint StreamSampleRate { get; }
        public ushort StreamEncoding { get; }
        public ushort StreamNumChans { get; }
        public SdpStreamDirection StreamDir { get; }

        public SdpDescriptorInfo(
            string username, 
            string sessionName, 
            ulong sessionId, 
            string sessionOriginatorAddress, 
            bool isDante, 
            uint mediaClockOffset, 
            byte streamPayloadType, 
            IList<SdpDescriptorGroupInfo> groups, 
            string gMid, 
            string subDomain, 
            uint streamSampleRate, 
            ushort streamEncoding, 
            ushort streamNumChans, 
            SdpStreamDirection streamDir)
        {
            Username = username;
            SessionName = sessionName;
            SessionId = sessionId;
            SessionOriginatorAddress = sessionOriginatorAddress;
            IsDante = isDante;
            MediaClockOffset = mediaClockOffset;
            StreamPayloadType = streamPayloadType;
            Groups = groups;
            GMid = gMid;
            SubDomain = subDomain;
            StreamSampleRate = streamSampleRate;
            StreamEncoding = streamEncoding;
            StreamNumChans = streamNumChans;
            StreamDir = streamDir;
        }
    }
}

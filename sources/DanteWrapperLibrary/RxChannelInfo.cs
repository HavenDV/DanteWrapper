using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RxChannelInfo
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
}

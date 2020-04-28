using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    public struct TxChannelInfo
    {
        public uint id;
        public bool stale;
        public string name;
        public string format;
        public bool enabled;
        public bool muted;
        public uint dbu;
    }
}

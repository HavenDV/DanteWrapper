using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    public struct TxChannelInfo
    {
        public ushort id;
        public int stale;
        public string name;
        public string format;
        public int enabled;
        public int muted;
        public short dbu;
    }
}

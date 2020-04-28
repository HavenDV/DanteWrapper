using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct InternalTxChannelInfo
    {
        public ushort id;
        public int stale;
        public string name;
        public string format;
        public int enabled;
        public int muted;
        public short dbu;
    }

    public class TxChannelInfo
    {
        public int Id { get; }
        public bool IsStale { get; }
        public string Name { get; }
        public string Format { get; }
        public bool IsEnabled { get; }
        public bool IsMuted { get; }

        /// <summary>
        /// -1 if it's unset <br/>
        /// -2 if it's invalid/uninitialised
        /// </summary>
        public int Dbu { get; }

        public TxChannelInfo(int id, bool isStale, string name, string format, bool isEnabled, bool isMuted, int dbu)
        {
            Id = id;
            IsStale = isStale;
            Name = name;
            Format = format;
            IsEnabled = isEnabled;
            IsMuted = isMuted;
            Dbu = dbu switch
            {
                0x7FFF => -1,
                0x7FFE => -2,
                _ => dbu
            };
        }
    }
}

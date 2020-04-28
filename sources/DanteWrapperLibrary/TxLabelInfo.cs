using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    public struct TxLabelInfo
    {
        public ushort id;
        public int data_exists;
        public string name;
        public int label_count;
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.AnsiBStr)]
        public string[] labels;
    }
}

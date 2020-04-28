using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct InternalTxLabelInfo
    {
        public ushort id;
        public int is_empty;
        public string name;
        public int label_count;
        public IntPtr labels;
    }

    public class TxLabelInfo
    {
        public int Id { get; }
        public bool IsEmpty { get; }
        public string Name { get; }
        public IList<string> Labels { get; }

        public TxLabelInfo(int id, bool isEmpty, string name, IList<string> labels)
        {
            Id = id;
            IsEmpty = isEmpty;
            Name = name;
            Labels = labels;
        }
    }
}

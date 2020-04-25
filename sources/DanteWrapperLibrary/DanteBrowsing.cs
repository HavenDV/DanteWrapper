using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing
    {
        [DllImport("dante_browsing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetNames(
            out IntPtr stringArray,
            out int count
        );

        public static IList<string> GetNames()
        {
            var result = GetNames(out var ptr, out var count);
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }

            MarshalUnmananagedStrArray2ManagedStrArray
            (
                ptr,
                count,
                out var array
            );

            return array;
        }

        public static void MarshalUnmananagedStrArray2ManagedStrArray
        (
            IntPtr ptr,
            int count,
            out string[] array
        )
        {
            var arrayPtr = new IntPtr[count];
            array = new string[count];

            Marshal.Copy(ptr, arrayPtr, 0, count);

            for (var i = 0; i < count; i++)
            {
                array[i] = Marshal.PtrToStringAnsi(arrayPtr[i]);
                Marshal.FreeCoTaskMem(arrayPtr[i]);
            }

            Marshal.FreeCoTaskMem(ptr);
        }
    }
}

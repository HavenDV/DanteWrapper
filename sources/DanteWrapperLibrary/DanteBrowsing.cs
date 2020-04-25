using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing
    {
        [DllImport("dante_browsing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDeviceNames(
            out IntPtr stringArray,
            out int count
        );

        public static IList<string> GetDeviceNames()
        {
            var result = GetDeviceNames(out var ptr, out var count);
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }

            MarshalUtilities.ToManagedStringArray
            (
                ptr,
                count,
                out var array
            );

            return array;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteRouting
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

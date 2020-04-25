using System;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public static class MarshalUtilities
    {
        public static void ToManagedStringArray
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

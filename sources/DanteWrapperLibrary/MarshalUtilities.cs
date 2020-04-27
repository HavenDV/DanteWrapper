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
            ToManagedArray(ptr, count, out array, Marshal.PtrToStringAnsi);
        }
        
        public static void ToManagedStructureArray<T>
        (
            IntPtr ptr,
            int count,
            out T[] array
        )
        {
            ToManagedArray(ptr, count, out array, Marshal.PtrToStructure<T>);
        }

        public static void ToManagedArray<T>
        (
            IntPtr ptr,
            int count,
            out T[] array,
            Func<IntPtr, T> func
        )
        {
            var arrayPtr = new IntPtr[count];
            array = new T[count];

            Marshal.Copy(ptr, arrayPtr, 0, count);

            for (var i = 0; i < count; i++)
            {
                array[i] = func(arrayPtr[i]);
                Marshal.FreeCoTaskMem(arrayPtr[i]);
            }

            Marshal.FreeCoTaskMem(ptr);
        }
    }
}

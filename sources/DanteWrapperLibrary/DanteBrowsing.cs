using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing
    {
        #region Imports

        [DllImport("dante_browsing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int RunDll(
            int argc,
            string[] argv,
            string input,
            out IntPtr array,
            out int count
        );

        /// <summary>
        /// Entry point to library
        /// </summary>
        /// <param name="array"></param>
        /// <param name="count"></param>
        /// <param name="input"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static void Run(out IntPtr array, out int count, string input)
        {
            var result = RunDll(2, new[] { "DanteBrowsingWrapper", "-conmon" }, input, out array, out count);
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }
        }

        /// <summary>
        /// Entry point to library(when the output is an array of strings)
        /// </summary>
        /// <param name="input"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static IList<string> RunAndGetStringArray(string input)
        {
            Run(out var ptr, out var count, input);
            MarshalUtilities.ToManagedStringArray
            (
                ptr,
                count,
                out var array
            );

            return array;
        }

        #endregion

        public static IList<string> GetDeviceNames()
        {
            return RunAndGetStringArray("r d");
        }
    }
}

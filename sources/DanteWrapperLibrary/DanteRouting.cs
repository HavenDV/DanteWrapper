using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteRouting
    {
        #region Imports

        [DllImport("dante_routing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Run(
            out IntPtr stringArray,
            out int count,
            string name,
            string argument
        );

        private static void Run(string name, string argument)
        {
            var result = Run(out _, out _, name, argument);
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }
        }

        private static IList<string> RunAndGetArray(string name, string argument)
        {
            var result = Run(out var ptr, out var count, name, argument);
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

        #endregion

        #region Methods

        public static void SetRxChannelName(string deviceName, int number, string name)
        {
            Run(deviceName, $"r {number} \"{name}\"");
        }

        public static IList<string> GetRxChannels(string deviceName)
        {
            return RunAndGetArray(deviceName, "r");
        }

        public static IList<string> GetTxChannels(string deviceName)
        {
            return RunAndGetArray(deviceName, "t");
        }

        public static void SetSxChannelName(string deviceName, int number, string name)
        {
            Run(deviceName, $"s {number} \"{name}\"");
        }

        public static IList<string> GetLabels(string deviceName)
        {
            return RunAndGetArray(deviceName, "l");
        }

        public static void AddTxLabel(string deviceName, int number, string name)
        {
            Run(deviceName, $"s {number} \"{name}\" +");
        }

        #endregion
    }
}

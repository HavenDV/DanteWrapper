using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteRouting
    {
        #region Imports

        [DllImport("dante_routing_test.dll", CallingConvention = CallingConvention.Cdecl)]
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
        /// <param name="name"></param>
        /// <param name="input"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static void Run(out IntPtr array, out int count, string name, string input)
        {
            var result = RunDll(2, new []{ "DanteRoutingWrapper", name }, input, out array, out count);
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }
        }

        /// <summary>
        /// Entry point to library(when there is no output)
        /// </summary>
        /// <param name="name"></param>
        /// <param name="argument"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static void Run(string name, string argument)
        {
            Run(out _, out _, name, argument);
        }

        /// <summary>
        /// Entry point to library(when the output is an array of strings)
        /// </summary>
        /// <param name="name"></param>
        /// <param name="argument"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static IList<string> RunAndGetStringArray(string name, string argument)
        {
            Run(out var ptr, out var count, name, argument);
            MarshalUtilities.ToManagedStringArray
            (
                ptr,
                count,
                out var array
            );

            return array;
        }

        /// <summary>
        /// Entry point to library(when the output is an array of structures)
        /// </summary>
        /// <param name="name"></param>
        /// <param name="argument"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static IList<T> RunAndGetStructureArray<T>(string name, string argument)
        {
            Run(out var ptr, out var count, name, argument);
            MarshalUtilities.ToManagedStructureArray<T>
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
            return RunAndGetStringArray(deviceName, "r");
        }

        public static IList<string> GetTxChannels(string deviceName)
        {
            return RunAndGetStringArray(deviceName, "t");
        }

        public static void SetSxChannelName(string deviceName, int number, string name)
        {
            Run(deviceName, $"s {number} \"{name}\"");
        }

        public static IList<string> GetLabels(string deviceName)
        {
            return RunAndGetStringArray(deviceName, "l");
        }

        public static void AddTxLabel(string deviceName, int number, string name)
        {
            Run(deviceName, $"l {number} \"{name}\" +");
        }

        #endregion
    }
}

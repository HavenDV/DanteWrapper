using System;
using System.Collections.Generic;
using System.Linq;
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

        public static IList<RxChannelInfo> GetRxChannels(string deviceName)
        {
            return RunAndGetStructureArray<RxChannelInfo>(deviceName, "r");
        }

        public static IList<TxChannelInfo> GetTxChannels(string deviceName)
        {
            return RunAndGetStructureArray<InternalTxChannelInfo>(deviceName, "t")
                .Select(info => new TxChannelInfo(
                    info.id, 
                    Convert.ToBoolean(info.stale), 
                    info.name, 
                    info.format, 
                    Convert.ToBoolean(info.enabled),
                    Convert.ToBoolean(info.muted),
                    info.dbu))
                .ToArray();
        }

        public static void SetSxChannelName(string deviceName, int number, string name)
        {
            Run(deviceName, $"s {number} \"{name}\"");
        }

        public static IList<TxLabelInfo> GetTxLabels(string deviceName)
        {
            return RunAndGetStructureArray<InternalTxLabelInfo>(deviceName, "l")
                .Select(info =>
                {
                    MarshalUtilities.ToManagedStringArray(info.labels, info.label_count, out var labels);

                    return new TxLabelInfo(info.id, Convert.ToBoolean(info.is_empty), info.name, labels);
                })
                .ToArray();
        }

        public static void AddTxLabel(string deviceName, int number, string name)
        {
            Run(deviceName, $"l {number} \"{name}\" +");
        }

        #endregion
    }
}

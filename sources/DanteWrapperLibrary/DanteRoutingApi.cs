using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteRoutingApi
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

        [DllImport("dante_routing_test.dll", EntryPoint = "open", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Open(
            int argc,
            string[] argv,
            out IntPtr ptr
        );

        [DllImport("dante_routing_test.dll", EntryPoint = "step", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Step(
            ref IntPtr ptr
        );

        internal delegate void EventCallbackDelegate(string value);

        [DllImport("dante_routing_test.dll", EntryPoint = "set_event_callback", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetEventCallback(
            EventCallbackDelegate @delegate
        );

        [DllImport("dante_routing_test.dll", EntryPoint = "process_line", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ProcessLine(
            ref IntPtr ptr,
            string input,
            out IntPtr array,
            out int count
        );

        [DllImport("dante_routing_test.dll", EntryPoint = "close", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Close(
            ref IntPtr ptr
        );

        /// <summary>
        /// Checks result and throws exception if it's not equals 0
        /// </summary>
        /// <param name="result"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        internal static void CheckResult(int result)
        {
            if (result != 0)
            {
                throw new InvalidOperationException($"Bad result: {result}");
            }
        }

        /// <summary>
        /// Opens device and returns pointer
        /// </summary>
        /// <param name="name"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        internal static IntPtr OpenDevice(string name)
        {
            CheckResult(Open(2, new[] { "DanteRoutingWrapper", name }, out var ptr));

            return ptr;
        }

        /// <summary>
        /// Performs next step
        /// </summary>
        /// <param name="ptr"></param>
        /// <returns></returns>
        internal static void PerformNextDeviceStep(IntPtr ptr)
        {
            CheckResult(Step(ref ptr));
        }

        /// <summary>
        /// Performs next step
        /// </summary>
        /// <param name="ptr"></param>
        /// <param name="input"></param>
        /// <param name="array"></param>
        /// <param name="count"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        internal static void ProcessLine(IntPtr ptr, string input, out IntPtr array, out int count)
        {
            if (ptr == IntPtr.Zero)
            {
                throw new InvalidOperationException("Device is not initialized");
            }

            CheckResult(ProcessLine(ref ptr, input, out array, out count));
        }

        /// <summary>
        /// Entry point to library(when there is no output)
        /// </summary>
        /// <param name="ptr"></param>
        /// <param name="argument"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        internal static void ProcessLine(IntPtr ptr, string argument)
        {
            ProcessLine(ptr, argument, out _, out _);
        }

        /// <summary>
        /// Entry point to library(when the output is an array of structures)
        /// </summary>
        /// <param name="ptr"></param>
        /// <param name="argument"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        internal static IList<T> ProcessLineAndGetStructureArray<T>(IntPtr ptr, string argument)
        {
            ProcessLine(ptr, argument, out var arrayPtr, out var count);
            MarshalUtilities.ToManagedStructureArray<T>
            (
                arrayPtr,
                count,
                out var array
            );

            return array;
        }

        /// <summary>
        /// Closes device
        /// </summary>
        /// <param name="ptr"></param>
        /// <returns></returns>
        internal static void CloseDevice(IntPtr ptr)
        {
            try
            {
                Close(ref ptr);
            }
            finally
            {
                Marshal.FreeCoTaskMem(ptr);
            }
        }

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

        #region Events

        public static event EventHandler<string>? EventOccurred;

        private static void OnEventOccurred(string value)
        {
            EventOccurred?.Invoke(null, value);
        }

        #endregion

        #region Methods

        public static void InitializeEvents()
        {
            SetEventCallback(OnEventOccurred);
        }

        public static void SetRxChannelName(string deviceName, int number, string name)
        {
            Run(deviceName, $"r {number} \"{name}\"");
        }

        public static IList<RxChannelInfo> GetRxChannels(string deviceName)
        {
            return RunAndGetStructureArray<InternalRxChannelInfo>(deviceName, "r")
                .Select(info => new RxChannelInfo(
                    info.id,
                    Convert.ToBoolean(info.stale),
                    info.name,
                    info.format,
                    info.latency,
                    Convert.ToBoolean(info.muted),
                    info.dbu,
                    info.sub,
                    info.status,
                    info.flow))
                .ToArray();
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

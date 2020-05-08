using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteRoutingApi
    {
        #region Imports

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

        internal delegate void DomainEventCallbackDelegate(string text);
        internal delegate void DeviceEventCallbackDelegate(string name, string text);

        [DllImport("dante_routing_test.dll", EntryPoint = "set_domain_event_callback", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetDomainEventCallback(
            DomainEventCallbackDelegate @delegate
        );

        [DllImport("dante_routing_test.dll", EntryPoint = "set_device_event_callback", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetDeviceEventCallback(
            DeviceEventCallbackDelegate @delegate
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

        #endregion

        #region Events

        public static event EventHandler<string>? DomainEventOccurred;

        private static void OnDomainEventOccurred(string value)
        {
            DomainEventOccurred?.Invoke(null, value);
        }

        public static event EventHandler<(string name, string text)>? DeviceEventOccurred;

        private static void OnDeviceEventOccurred(string name, string text)
        {
            DeviceEventOccurred?.Invoke(null, (name, text));
        }

        #endregion

        #region Methods

        internal static void InitializeDomainEvents()
        {
            SetDomainEventCallback(OnDomainEventOccurred);
        }

        internal static void InitializeDeviceEvents()
        {
            SetDeviceEventCallback(OnDeviceEventOccurred);
        }


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

        #endregion
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
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

        /// <summary>
        /// Entry point to library(when the output is an array of structures)
        /// </summary>
        /// <param name="input"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <returns></returns>
        private static IList<T> RunAndGetStructureArray<T>(string input)
        {
            Run(out var ptr, out var count, input);
            MarshalUtilities.ToManagedStructureArray<T>
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

        public static IList<SdpDescriptorInfo> GetSdpDescriptors()
        {
            return RunAndGetStructureArray<InternalSdpDescriptorInfo>("p")
                .Select(info =>
                {
                    MarshalUtilities.ToManagedStructureArray<InternalSdpDescriptorGroupInfo>(
                        info.groups, info.groups_count, out var internalGroups);

                    return new SdpDescriptorInfo(
                        info.username,
                        info.session_name,
                        info.session_id,
                        info.session_originator_address,
                        Convert.ToBoolean(info.is_dante),
                        info.media_clock_offset,
                        info.stream_payload_type,
                        internalGroups
                            .Select(group => new SdpDescriptorGroupInfo(group.address, group.port, group.id))
                            .ToArray(),
                        info.gmid,
                        info.sub_domain,
                        info.stream_sample_rate,
                        info.stream_encoding,
                        info.stream_num_chans,
                        info.stream_dir);
                })
                .ToArray();
        }
    }
}

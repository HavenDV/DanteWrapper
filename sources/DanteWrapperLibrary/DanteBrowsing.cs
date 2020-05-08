using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using DanteWrapperLibrary.Utilities;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing : IDisposable
    {
        #region Static methods

        public static async Task<T> RunAsync<T>(Func<DanteBrowsing, T> func, TimeSpan? delay = null, CancellationToken cancellationToken = default)
        {
            using var browsing = new DanteBrowsing();
            browsing.Initialize();

            await Task.Delay(delay ?? TimeSpan.Zero, cancellationToken).ConfigureAwait(false);

            return func(browsing);
        }

        public static async Task<IList<string>> GetDeviceNamesAsync(TimeSpan? delay = null, CancellationToken cancellationToken = default)
        {
            return await RunAsync(browsing => browsing.GetDeviceNames(), delay, cancellationToken).ConfigureAwait(false);
        }

        public static async Task<IList<SdpDescriptorInfo>> GetSdpDescriptorsAsync(TimeSpan? delay = null, CancellationToken cancellationToken = default)
        {
            return await RunAsync(browsing => browsing.GetSdpDescriptors(), delay, cancellationToken).ConfigureAwait(false);
        }

        #endregion

        #region Properties

        private IntPtr IntPtr { get; set; } = IntPtr.Zero;
        private TaskWorker TaskWorker { get; } = new TaskWorker();

        #endregion

        #region Events

        public event EventHandler? StepOccurred;

        private void OnStepOccurred()
        {
            StepOccurred?.Invoke(this, EventArgs.Empty);
        }

        #endregion

        #region Methods

        public void Initialize()
        {
            if (IntPtr != IntPtr.Zero)
            {
                return;
            }

            IntPtr = DanteBrowsingApi.Open();

            TaskWorker.Start(cancellationToken =>
            {
                while (!cancellationToken.IsCancellationRequested && IntPtr != IntPtr.Zero)
                {
                    DanteBrowsingApi.PerformNextDeviceStep(IntPtr);

                    OnStepOccurred();
                }
            });
        }

        public IList<string> GetDeviceNames()
        {
            return DanteBrowsingApi.ProcessLineAndGetStringArray(IntPtr, "r d");
        }

        public IList<SdpDescriptorInfo> GetSdpDescriptors()
        {
            return DanteBrowsingApi.ProcessLineAndGetStructureArray<InternalSdpDescriptorInfo>(IntPtr, "p")
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

        public void Dispose()
        {
            if (IntPtr == IntPtr.Zero)
            {
                return;
            }

            // Waits to complete the last step
            TaskWorker.Dispose();

            try
            {
                DanteBrowsingApi.Close(IntPtr);
            }
            finally
            {
                IntPtr = IntPtr.Zero;
            }
        }

        #endregion
    }
}

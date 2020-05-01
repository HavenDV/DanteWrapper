using System;
using System.Collections.Generic;
using System.Linq;
using DanteWrapperLibrary.Utilities;

namespace DanteWrapperLibrary
{
    public class RoutingDevice : IDisposable
    {
        #region Properties

        public string Name { get; }

        private IntPtr IntPtr { get; set; } = IntPtr.Zero;
        private TaskWorker TaskWorker { get; set; } = new TaskWorker();

        #endregion

        #region Events

        public event EventHandler? StepOccurred;

        private void OnStepOccurred()
        {
            StepOccurred?.Invoke(this, EventArgs.Empty);
        }

        #endregion

        #region Constructors

        public RoutingDevice(string name)
        {
            Name = name ?? throw new ArgumentNullException(nameof(name));
        }

        #endregion

        #region Methods

        public void Initialize()
        {
            if (IntPtr != IntPtr.Zero)
            {
                return;
            }

            IntPtr = DanteRouting.OpenDevice(Name);

            TaskWorker.Start(cancellationToken =>
            {
                while (!cancellationToken.IsCancellationRequested && IntPtr != IntPtr.Zero)
                {
                    DanteRouting.PerformNextDeviceStep(IntPtr);

                    OnStepOccurred();
                }
            });
        }

        public IList<RxChannelInfo> GetRxChannels()
        {
            return DanteRouting.ProcessLineAndGetStructureArray<InternalRxChannelInfo>(IntPtr, "r")
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

        public void SetRxChannelName(int number, string name)
        {
            DanteRouting.ProcessLine(IntPtr, $"r {number} \"{name}\"");
        }

        public IList<TxChannelInfo> GetTxChannels()
        {
            return DanteRouting.ProcessLineAndGetStructureArray<InternalTxChannelInfo>(IntPtr, "t")
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

        public void SetSxChannelName(int number, string name)
        {
            DanteRouting.ProcessLine(IntPtr, $"s {number} \"{name}\"");
        }

        public IList<TxLabelInfo> GetTxLabels()
        {
            return DanteRouting.ProcessLineAndGetStructureArray<InternalTxLabelInfo>(IntPtr, "l")
                .Select(info =>
                {
                    MarshalUtilities.ToManagedStringArray(info.labels, info.label_count, out var labels);

                    return new TxLabelInfo(info.id, Convert.ToBoolean(info.is_empty), info.name, labels);
                })
                .ToArray();
        }

        public void AddTxLabel(int number, string name)
        {
            DanteRouting.ProcessLine(IntPtr, $"l {number} \"{name}\" +");
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
                DanteRouting.CloseDevice(IntPtr);
            }
            finally
            {
                IntPtr = IntPtr.Zero;
            }
        }

        #endregion

        #region Event Handlers



        #endregion
    }
}

using System;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public class DanteRoutingTests
    {
        [TestMethod]
        public void NotInitializedRoutingDeviceTest()
        {
            using var device = new RoutingDevice("DESKTOP-VSC");
        }

        [TestMethod]
        public async Task InitializedRoutingDeviceTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC");
        }

        [TestMethod]
        public async Task RoutingDeviceTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            Console.WriteLine("GetRxChannels:");
            foreach (var info in device.GetRxChannels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }

            Console.WriteLine("GetTxChannels:");
            foreach (var info in device.GetTxChannels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }

            Console.WriteLine("GetTxLabels:");
            foreach (var info in device.GetTxLabels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public async Task MultiDeviceTest()
        {
            using var device1 = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));
            using var device2 = await GetInitializedDeviceAsync("test device", TimeSpan.FromSeconds(3));
        }

        [TestMethod]
        public async Task MultiDeviceTest2()
        {
            {
                using var device1 = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));
            }
            {
                using var device2 = await GetInitializedDeviceAsync("test device", TimeSpan.FromSeconds(3));
            }
        }

        [TestMethod]
        public async Task GetRxChannelsTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            foreach (var info in device.GetRxChannels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public async Task GetTxChannelsTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            foreach (var info in device.GetTxChannels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public async Task GetTxLabelsTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            foreach (var info in device.GetTxLabels())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public async Task AddTxLabelTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            device.AddTxLabel(3, "TEST-LABEL");
        }

        [TestMethod]
        public async Task SetRxChannelNameTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            device.SetRxChannelName(3, "TEST-CHANNEL-NAME");
        }

        [TestMethod]
        public async Task SetSxChannelNameTest()
        {
            using var device = await GetInitializedDeviceAsync("DESKTOP-VSC", TimeSpan.FromSeconds(3));

            device.SetSxChannelName(3, "TEST-CHANNEL-NAME");
        }

        private static async Task<RoutingDevice> GetInitializedDeviceAsync(
            string name,
            TimeSpan? delay = null,
            CancellationToken cancellationToken = default)
        {
            var device = new RoutingDevice(name);
            device.StepOccurred += (_, args) =>
            {
                Console.WriteLine($"{name} StepOccurred");
            };
            device.EventOccurred += (_, text) =>
            {
                Console.WriteLine($"{name} EventOccurred: {text}");
            };
            device.DomainEventOccurred += (_, text) =>
            {
                Console.WriteLine($"{name} DomainEventOccurred: {text}");
            };

            device.Initialize();

            await Task.Delay(delay ?? TimeSpan.Zero, cancellationToken);

            return device;
        }
    }
}
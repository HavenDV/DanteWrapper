using System;
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
        public void InitializedRoutingDeviceTest()
        {
            using var device = new RoutingDevice("DESKTOP-VSC");

            device.Initialize();
        }

        [TestMethod]
        public async Task RoutingDeviceTest()
        {
            DanteRoutingApi.DomainEventOccurred += (_, text) =>
            {
                Console.WriteLine($"DomainEventOccurred: {text}");
            };
            DanteRoutingApi.InitializeDomainEvents();

            using var device = new RoutingDevice("DESKTOP-VSC");
            device.StepOccurred += (sender, args) =>
            {
                Console.WriteLine("StepOccurred");
            };
            device.Initialize();

            await Task.Delay(TimeSpan.FromSeconds(3));

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
        public void GetRxChannelsTest()
        {
            foreach (var info in DanteRoutingApi.GetRxChannels("DESKTOP-VSC"))
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void GetTxChannelsTest()
        {
            foreach (var info in DanteRoutingApi.GetTxChannels("DESKTOP-VSC"))
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void GetTxLabelsTest()
        {
            foreach (var info in DanteRoutingApi.GetTxLabels("DESKTOP-VSC"))
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void AddTxLabelTest()
        {
            DanteRoutingApi.AddTxLabel("DESKTOP-VSC", 3, "TEST-LABEL");
        }

        [TestMethod]
        public void SetRxChannelNameTest()
        {
            DanteRoutingApi.SetRxChannelName("DESKTOP-VSC", 3, "TEST-CHANNEL-NAME");
        }

        [TestMethod]
        public void SetSxChannelNameTest()
        {
            DanteRoutingApi.SetSxChannelName("DESKTOP-VSC", 3, "TEST-CHANNEL-NAME");
        }
    }
}
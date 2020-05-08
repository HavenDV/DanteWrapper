using System;
using System.Collections;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public class DanteBrowsingTests
    {
        [TestMethod]
        public void NotInitializedRoutingDeviceTest()
        {
            using var device = new RoutingDevice("DESKTOP-VSC");
        }

        [TestMethod]
        public async Task RoutingDeviceTest()
        {
            DanteRoutingApi.EventOccurred += (_, text) =>
            {
                Console.WriteLine($"EventOccurred: {text}");
            };
            DanteRoutingApi.InitializeEvents();

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
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"IsStale: {info.IsStale}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"Format: {info.Format}");
                Console.WriteLine($"Latency: {info.Latency}");
                Console.WriteLine($"IsMuted: {info.IsMuted}");
                Console.WriteLine($"SignalReferenceLevel: {info.SignalReferenceLevel}");
                Console.WriteLine($"Subscription: {info.Subscription}");
                Console.WriteLine($"Status: {info.Status}");
                Console.WriteLine($"Flow: {info.Flow}");
                Console.WriteLine();
            }

            Console.WriteLine("GetTxChannels:");
            foreach (var info in device.GetTxChannels())
            {
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"IsStale: {info.IsStale}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"Format: {info.Format}");
                Console.WriteLine($"IsEnabled: {info.IsEnabled}");
                Console.WriteLine($"IsMuted: {info.IsMuted}");
                Console.WriteLine($"SignalReferenceLevel: {info.SignalReferenceLevel}");
                Console.WriteLine();
            }

            Console.WriteLine("GetTxLabels:");
            foreach (var info in device.GetTxLabels())
            {
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"IsEmpty: {info.IsEmpty}");
                Console.WriteLine("Labels:");
                foreach (var label in info.Labels)
                {
                    Console.WriteLine($"-- {label}");
                }
                Console.WriteLine();
            }
        }

        [TestMethod]
        public async Task GetDeviceNamesTest()
        {
            foreach (var name in await DanteBrowsing.GetDeviceNamesAsync())
            {
                ShowProperties(name);
            }
        }

        [TestMethod]
        public async Task GetSdpDescriptorsTest()
        {
            foreach (var info in await DanteBrowsing.GetSdpDescriptorsAsync())
            {
                ShowProperties(info);
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void GetRxChannelsTest()
        {
            foreach (var info in DanteRoutingApi.GetRxChannels("DESKTOP-VSC"))
            {
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"IsStale: {info.IsStale}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"Format: {info.Format}");
                Console.WriteLine($"Latency: {info.Latency}");
                Console.WriteLine($"IsMuted: {info.IsMuted}");
                Console.WriteLine($"SignalReferenceLevel: {info.SignalReferenceLevel}");
                Console.WriteLine($"Subscription: {info.Subscription}");
                Console.WriteLine($"Status: {info.Status}");
                Console.WriteLine($"Flow: {info.Flow}");
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void GetTxChannelsTest()
        {
            foreach (var info in DanteRoutingApi.GetTxChannels("DESKTOP-VSC"))
            {
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"IsStale: {info.IsStale}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"Format: {info.Format}");
                Console.WriteLine($"IsEnabled: {info.IsEnabled}");
                Console.WriteLine($"IsMuted: {info.IsMuted}");
                Console.WriteLine($"SignalReferenceLevel: {info.SignalReferenceLevel}");
                Console.WriteLine();
            }
        }

        [TestMethod]
        public void GetTxLabelsTest()
        {
            foreach (var info in DanteRoutingApi.GetTxLabels("DESKTOP-VSC"))
            {
                Console.WriteLine($"Id: {info.Id}");
                Console.WriteLine($"Name: {info.Name}");
                Console.WriteLine($"IsEmpty: {info.IsEmpty}");
                Console.WriteLine("Labels:");
                foreach (var label in info.Labels)
                {
                    Console.WriteLine($"-- {label}");
                }
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

        #region Utilities

        private static void ShowProperties(object obj, string? prefix = null)
        {
            if (obj is string stringValue)
            {
                Console.WriteLine(stringValue);
                return;
            }

            foreach (var info in obj.GetType().GetProperties())
            {
                var value = info.GetValue(obj);
                if (!(value is string) &&
                    value is IEnumerable enumerable)
                {
                    Console.WriteLine($"{info.Name}:");
                    foreach (var subValue in enumerable)
                    {
                        ShowProperties(subValue, " - ");
                        Console.WriteLine();
                    }
                }
                else
                {
                    Console.WriteLine($"{prefix}{info.Name}: {value}");
                }
            }
        }

        #endregion
    }
}
using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public class DanteBrowsingTests
    {
        [TestMethod]
        public void GetDeviceNamesTest()
        {
            foreach (var name in DanteBrowsing.GetDeviceNames())
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void GetRxChannelsTest()
        {
            foreach (var name in DanteRouting.GetRxChannels("DESKTOP-VSC"))
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void GetTxChannelsTest()
        {
            foreach (var name in DanteRouting.GetTxChannels("DESKTOP-VSC"))
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void GetTxLabelsTest()
        {
            foreach (var info in DanteRouting.GetTxLabels("DESKTOP-VSC"))
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
            DanteRouting.AddTxLabel("DESKTOP-VSC", 3, "TEST-LABEL");
        }

        [TestMethod]
        public void SetRxChannelNameTest()
        {
            DanteRouting.SetRxChannelName("DESKTOP-VSC", 3, "TEST-CHANNEL-NAME");
        }

        [TestMethod]
        public void SetSxChannelNameTest()
        {
            DanteRouting.SetSxChannelName("DESKTOP-VSC", 3, "TEST-CHANNEL-NAME");
        }
    }
}
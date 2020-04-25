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
        public void GetLabelsTest()
        {
            foreach (var name in DanteRouting.GetLabels("DESKTOP-VSC"))
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void AddTxLabelTest()
        {
            DanteRouting.AddTxLabel("DESKTOP-VSC", 3, "TEST-LABEL");
        }
    }
}
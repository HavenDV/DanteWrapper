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
            foreach (var name in DanteRouting.GetRxChannels("WIN-59B9F8QNN58"))
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void GetTxChannelsTest()
        {
            foreach (var name in DanteRouting.GetTxChannels("WIN-59B9F8QNN58"))
            {
                Console.WriteLine(name);
            }
        }

        [TestMethod]
        public void GetLabelsTest()
        {
            foreach (var name in DanteRouting.GetLabels("WIN-59B9F8QNN58"))
            {
                Console.WriteLine(name);
            }
        }
    }
}
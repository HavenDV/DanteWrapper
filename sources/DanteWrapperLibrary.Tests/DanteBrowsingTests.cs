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
            foreach (var name in DanteRouting.GetRxChannels("test"))
            {
                Console.WriteLine(name);
            }
        }
    }
}

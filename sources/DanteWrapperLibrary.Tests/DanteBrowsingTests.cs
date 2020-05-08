using System;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public class DanteBrowsingTests
    {
        [TestMethod]
        public void NotInitializedDanteBrowsingTest()
        {
            using var device = new DanteBrowsing();
        }

        [TestMethod]
        public void InitializedDanteBrowsingTest()
        {
            using var device = new DanteBrowsing();

            device.Initialize();
        }

        [TestMethod]
        public async Task GetDeviceNamesTest()
        {
            foreach (var name in await DanteBrowsing.GetDeviceNamesAsync())
            {
                PrintUtilities.ShowProperties(name);
            }
        }

        [TestMethod]
        public async Task GetSdpDescriptorsTest()
        {
            foreach (var info in await DanteBrowsing.GetSdpDescriptorsAsync())
            {
                PrintUtilities.ShowProperties(info);
                Console.WriteLine();
            }
        }
    }
}
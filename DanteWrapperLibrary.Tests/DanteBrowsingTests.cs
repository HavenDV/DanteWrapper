using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public class DanteBrowsingTests
    {
        [TestMethod]
        public void GetNamesTest()
        {
            foreach (var name in DanteBrowsing.GetNames())
            {
                Console.WriteLine(name);
            }
        }
    }
}

using System;
using DanteWrapperLibrary;

namespace DanteWrapperApp
{
    internal class Program
    {
        private static void Main()
        {
            foreach (var name in DanteRouting.GetRxChannels("DESKTOP-VSC"))
            {
                Console.WriteLine(name);
            }
        }
    }
}

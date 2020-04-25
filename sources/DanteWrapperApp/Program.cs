using System;
using DanteWrapperLibrary;

namespace DanteWrapperApp
{
    internal class Program
    {
        private static void Main()
        {
            foreach (var name in DanteRouting.GetRxChannels("WIN-59B9F8QNN58"))
            {
                Console.WriteLine(name);
            }
        }
    }
}

using System;
using DanteWrapperLibrary;

namespace DanteWrapperApp
{
    internal class Program
    {
        private static void Main()
        {
            foreach (var info in DanteRouting.GetRxChannels("DESKTOP-VSC"))
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
    }
}

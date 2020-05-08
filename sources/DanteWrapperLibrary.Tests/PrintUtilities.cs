using System;
using System.Collections;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DanteWrapperLibrary.Tests
{
    [TestClass]
    public static class PrintUtilities
    {
        public static void ShowProperties(object obj, string? prefix = null)
        {
            if (obj is string stringValue)
            {
                Console.WriteLine($"{prefix}{stringValue}");
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
                    }
                }
                else
                {
                    Console.WriteLine($"{prefix}{info.Name}: {value}");
                }
            }
        }
    }
}
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing
    {
        [DllImport("dante_browsing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetNames(int argc, [In, Out] string[] argv);

        public static IList<string> GetNames()
        {
            var args = new[] { "hello.exe", "-conmon", "", "", "", "", "", "", "", "" };
            GetNames(2, args);

            return args;
        }
    }
}

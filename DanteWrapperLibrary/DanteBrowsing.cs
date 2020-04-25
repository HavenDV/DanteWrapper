using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace DanteWrapperLibrary
{
    public class DanteBrowsing
    {
        [DllImport("../../../../Dependencies/dante_browsing_test.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int test(int argc, [In, Out] string[] argv);

        public static IList<string> GetNames()
        {
            var args = new[] { "hello.exe", "-conmon", "", "", "", "", "", "", "", "" };
            test(2, args);

            return args;
        }
    }
}

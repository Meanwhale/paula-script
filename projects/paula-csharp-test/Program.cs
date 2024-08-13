using System;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("paula-dll.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void printVersion();

    static void Main()
    {
        printVersion();
    }
}
using System;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("paula_debug", CallingConvention = CallingConvention.Cdecl)]
    public static extern void printVersion();
	
	[DllImport("paula_debug", CallingConvention = CallingConvention.Cdecl)]
	public static extern void runSafe(string code);
	
    [DllImport("paula_debug", CallingConvention = CallingConvention.Cdecl)]
    public static extern unsafe int* getPtr(string varName);

	static int GetInt(string varName)
	{
		unsafe
		{
			// TODO check type
			int* ptr = getPtr(varName);
			return *(ptr+3);
		}
	}

    static void Main()
    {
        printVersion();
		runSafe("a:5;print(a)");
		int x = GetInt("a");
		Console.WriteLine("CONSOLE a: " + x);
    }
}
/* paula_cs_example.cs -- C# embedding example for the Paula scripting engine.
 *
 * Demonstrates three integration patterns:
 *   1. Run an inline script string and read back a variable.
 *   2. Register a C# callback and call it from a script.
 *   3. Run a script loaded from a file.
 *
 * Build (from project root, paula-dll.dll must be in the same directory or PATH):
 *   csc projects/paula-cs/paula_cs_example.cs /out:build/paula_cs_example.exe
 *
 * Run (cwd = project root):
 *   build\paula_cs_example.exe
 */

using System;
using System.Runtime.InteropServices;

static class Paula
{
    const string Dll = "paula-dll";

    [DllImport(Dll)] public static extern void paula_print_version();
    [DllImport(Dll)] public static extern void paula_run_safe(string code);
    [DllImport(Dll)] public static extern int  paula_run_file(string path);
    [DllImport(Dll)] public static extern int  paula_get_int(string varName, out int value);
    [DllImport(Dll)] public static extern void paula_add_callback(string name, CallbackFn fn);
    [DllImport(Dll)] public static extern int  paula_args_count(IntPtr args);
    [DllImport(Dll)] public static extern int  paula_args_get_int(IntPtr args, int index, out int value);
    [DllImport(Dll)] public static extern void paula_args_return_int(IntPtr args, int value);

    public delegate int CallbackFn(IntPtr args);
}

class Program
{
    // Stored in a static field to prevent the GC from collecting it while
    // the DLL holds a raw function pointer.
    static Paula.CallbackFn doublerDelegate = Doubler;

    static int Doubler(IntPtr args)
    {
        if (Paula.paula_args_count(args) != 1) return 1;
        if (Paula.paula_args_get_int(args, 0, out int val) == 0) return 1;
        Paula.paula_args_return_int(args, 2 * val);
        return 0;
    }

    static void Main()
    {
        Paula.paula_print_version();

        // --- pattern 1: run inline script, read back a variable ---

        Paula.paula_run_safe("x: 6 * 7");

        if (Paula.paula_get_int("x", out int x) != 0)
            Console.WriteLine("x = " + x);

        // --- pattern 2: register a C# callback, call it from a script ---

        Paula.paula_add_callback("doubler", doublerDelegate);
        Paula.paula_run_safe("result: doubler(21)\nprint (\"moi\")");

        if (Paula.paula_get_int("result", out int result) != 0)
            Console.WriteLine("result = " + result);

        // --- pattern 3: run a script from a file ---

        if (Paula.paula_run_file("projects/test.pa") == 0)
            Console.WriteLine("script file not found");
    }
}

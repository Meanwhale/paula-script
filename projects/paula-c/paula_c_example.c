/* paula_c_example.c -- ANSI-C embedding example for the Paula engine.
 *
 * Demonstrates three integration patterns:
 *   1. Run an inline script string and read back a variable.
 *   2. Register a C callback and call it from a script.
 *   3. Run a script loaded from a file.
 *
 * Build: see CMakeLists.txt target "paula-c-example".
 * Run:   build\Debug\paula-c-example.exe   (cwd = project root)
 */

#include <stdio.h>
#include "paula_c.h"

static int doubler(paula_args_t args)
{
    int value = 0;
    if (paula_args_count(args) != 1)          return 1;
    if (!paula_args_get_int(args, 0, &value)) return 1;
    paula_args_return_int(args, 2 * value);
    return 0;
}

int main(void)
{
    int value = 0;

    paula_print_version();

    /* --- pattern 1: run inline script, read back a variable --- */

    paula_run_safe("x: 6 * 7");

    if (paula_get_int("x", &value))
        printf("x = %d\n", value);

    /* --- pattern 2: register a callback, call it from a script --- */

    paula_add_callback("doubler", doubler);
    paula_run_safe("result: doubler(21)\nprint (\"moi\")");

    if (paula_get_int("result", &value))
        printf("result = %d\n", value);

    /* --- pattern 3: run a script from a file --- */

    if (!paula_run_file("projects/test.pa"))
        printf("script file not found\n");

    return 0;
}

/* paula_c_example.c -- ANSI-C embedding example for the Paula engine.
 *
 * Demonstrates two integration patterns:
 *   1. Run an inline script string and read back a variable.
 *   2. Run a script loaded from a file.
 *
 * Build: see CMakeLists.txt target "paula-c-example".
 * Run:   build\Debug\paula-c-example.exe   (cwd = project root)
 */

#include <stdio.h>
#include "paula_c.h"

int main(void)
{
    int value = 0;

    paula_print_version();

    /* --- pattern 1: run inline script, read back a variable --- */

    paula_run_safe("x: 6 * 7");

    if (paula_get_int("x", &value))
    {
        printf("x = %d\n", value);
    }
    else
    {
        printf("could not read variable x\n");
    }

    /* --- pattern 2: run a script from a file --- */

    if (!paula_run_file("projects/test.pa"))
    {
        printf("script file not found\n");
    }

    return 0;
}

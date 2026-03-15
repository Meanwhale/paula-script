/* paula_c.cpp -- C++ bridge that exports a plain-C API for paula-core.
 *
 * Compiled as C++ (needs access to paula's C++ classes), but all symbols
 * are exported with C linkage so they can be called from ANSI-C code.
 */

#include "paula_c.h"
#include "paula.h"
#include "stream.h"

extern "C"
{

void paula_print_version(void)
{
    paula::printVersion();
}

void paula_run_safe(const char* code)
{
    paula::runSafe(code);
}

int paula_run_file(const char* path)
{
    if (!paula::FileInput::exists(path)) return 0;
    paula::FileInput in(path, /*binary=*/false);
    paula::runSafe(in);
    return 1;
}

int paula_get_int(const char* var_name, int* out)
{
    const paula::INT* ptr = paula::getPtr(var_name);
    if (ptr == nullptr) return 0;
    *out = (int)*ptr;
    return 1;
}

} /* extern "C" */

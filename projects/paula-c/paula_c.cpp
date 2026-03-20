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
    paula::INT value = 0;
    if (!paula::get(var_name).getInt(value)) return 0;
    *out = (int)value;
    return 1;
}

void paula_add_callback(const char* name, paula_callback_fn fn)
{
    // Wrap the C function pointer in a lambda so it can be stored as
    // std::function<const Error*(Args&)> inside paula-core.
    // The Args reference is passed as a void* (paula_args_t) to the C side;
    // the C callback uses paula_args_* helpers to read arguments and set the
    // return value without ever seeing the C++ type.
    paula::addCallback(name, [fn](paula::Args& args) -> const paula::Error*
    {
        return fn(static_cast<void*>(&args)) == 0 ? NO_ERROR : &paula::CALLBACK_ERROR;
    });
}

int paula_args_count(paula_args_t args_ptr)
{
    return (int)static_cast<paula::Args*>(args_ptr)->count();
}

int paula_args_get_int(paula_args_t args_ptr, int index, int* out)
{
    paula::INT value = 0;
    if (static_cast<paula::Args*>(args_ptr)->get((paula::INT)index).getInt(value))
    {
        *out = (int)value;
        return 1;
    }
    return 0;
}

void paula_args_return_int(paula_args_t args_ptr, int value)
{
    static_cast<paula::Args*>(args_ptr)->returnInt((paula::INT)value);
}

} /* extern "C" */

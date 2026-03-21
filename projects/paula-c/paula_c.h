/* paula_c.h -- Plain C API for embedding the Paula scripting engine.
 *
 * Compile paula_c.cpp (C++ bridge) alongside your C code and link
 * against paula-core (static library).  Paula itself is C++; this header
 * exposes the subset needed for common embedding tasks.
 */

#ifndef PAULA_C_H
#define PAULA_C_H

/* ── Export/import macro ──────────────────────────────────────────── */
/* Build the DLL:   -DPAULA_DLL_BUILD                                  */
/* Use the DLL:     -DPAULA_DLL_USE                                    */
/* Static linking:  neither (PAULA_API expands to nothing)             */

#if defined(PAULA_DLL_BUILD) || defined(PAULA_DLL_USE)
#  if defined(_WIN32)
#    if defined(PAULA_DLL_BUILD)
#      define PAULA_API __declspec(dllexport)
#    else
#      define PAULA_API __declspec(dllimport)
#    endif
#  else
#    define PAULA_API __attribute__((visibility("default")))
#  endif
#else
#  define PAULA_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Script execution ─────────────────────────────────────────────── */

/* Print the Paula version string to stdout. */
PAULA_API void paula_print_version(void);

/* Run a script string.  Errors are printed to stderr; execution continues. */
PAULA_API void paula_run_safe(const char* code);

/* Run a script from a file.
 * Returns 1 on success, 0 if the file is not found. */
PAULA_API int  paula_run_file(const char* path);

/* ── Variable access ──────────────────────────────────────────────── */

/* Read an integer variable by name into *out.
 * Returns 1 if the variable exists and holds an integer, 0 otherwise. */
PAULA_API int  paula_get_int(const char* var_name, int* out);

/* ── Callbacks ────────────────────────────────────────────────────── */

/* Opaque handle to the argument list passed to a callback. */
typedef void* paula_args_t;

/* C callback type.  Return 0 for success, non-zero to signal an error. */
typedef int (*paula_callback_fn)(paula_args_t args);

/* Register a C callback by name. */
PAULA_API void paula_add_callback(const char* name, paula_callback_fn fn);

/* Number of arguments passed to the callback, e.g. 2 for f(a, b). */
PAULA_API int  paula_args_count(paula_args_t args);

/* Read argument at index into *out.
 * Returns 1 if the argument exists and is an integer, 0 otherwise. */
PAULA_API int  paula_args_get_int(paula_args_t args, int index, int* out);

/* Set the integer return value of the callback. */
PAULA_API void paula_args_return_int(paula_args_t args, int value);

#ifdef __cplusplus
}
#endif

#endif /* PAULA_C_H */

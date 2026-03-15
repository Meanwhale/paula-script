/* paula_c.h -- Plain C API for embedding the Paula scripting engine.
 *
 * Compile paula_c.cpp (C++ bridge) alongside your C code and link
 * against paula-core (static library).  Paula itself is C++; this header
 * exposes the subset needed for common embedding tasks.
 */

#ifndef PAULA_C_H
#define PAULA_C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Print the Paula version string to stdout. */
void paula_print_version(void);

/* Run a script string.  Errors are printed to stderr; execution continues. */
void paula_run_safe(const char* code);

/* Run a script from a file.
 * Returns 1 on success, 0 if the file is not found. */
int  paula_run_file(const char* path);

/* Read an integer variable by name into *out.
 * Returns 1 if the variable exists and holds an integer, 0 otherwise. */
int  paula_get_int(const char* var_name, int* out);

#ifdef __cplusplus
}
#endif

#endif /* PAULA_C_H */

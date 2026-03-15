> _AI-generated quick-reference. For full details see [README.md](README.md)._

# Paula Script — Overview

Lightweight scripting language written in C++. No runtime memory allocation; everything runs in pre-allocated buffers. Stand-alone (built-in parser, minimal dependencies). Non-blocking line-by-line execution. Version 0.2. Targets embedded/memory-constrained use.

What you can build and run:
- **paula-cli** — run `.paula` scripts or pre-compiled bytecode from the command line
- **paula-cli (mini)** — size-optimized CLI variant; runs bytecode only, no script compiler
- **paula-dll** — embed the engine in another app via a C DLL (Windows)
- **paula-test** — unit test suite (Debug only)
- **paula-example** — developer scratchpad for trying engine features (Debug only)
- **paula-c-example** — ANSI-C embedding demo: inline run/get and file input (Debug only)

---

## Management files

| File | Purpose |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | CMake build definition — all targets, flags, per-config defines |
| [CMakePresets.json](CMakePresets.json) | Configure preset: `default` (Ninja Multi-Config, Ninja path from `%VSINSTALLDIR%`); build presets `debug` / `release` / `mini` |
| [Makefile](Makefile) | Linux-only shorthand over g++ — no CMake needed |
| [.vscode/launch.json](.vscode/launch.json) | VSCode run & debug configurations (three entries) |
| [.vscode/tasks.json](.vscode/tasks.json) | VSCode build task: `build-debug` (invokes CMake preset) |
| [.vscode/settings.json](.vscode/settings.json) | VSCode workspace settings |

## Project structure

```
src/                          paula-core static library (engine, parser, stack, utils…)
projects/
  paula-cli/paula-cli.cpp     CLI entry point
  paula-vs/paula-vs.cpp       Unit tests entry point  → calls testAll()
  paula-example/              Developer scratchpad (embedding / feature trials)
  paula-c/                    ANSI-C embedding demo
    paula_c.h                   Plain-C API header
    paula_c.cpp                 C++ bridge (extern "C" wrappers)
    paula_c_example.c           ANSI-C example: inline run/get + file input
  paula-dll/dllmain.cpp       Windows DLL — exports printVersion, runSafe, getPtr
  test.pa / test.bytecode     Sample script and pre-compiled bytecode
build/                        CMake Ninja Multi-Config output (gitignored)
```

---

## Building and running

### Windows — VSCode (recommended)

Prerequisite: Visual Studio 2022 with CMake and Ninja components. Open the project folder; CMake Tools picks up `CMakePresets.json` automatically.

| Action | How |
|---|---|
| Build Debug (all targets) | `Ctrl+Shift+B` or `cmake --build build --preset debug` |
| Build Release CLI | `cmake --build build --preset release` |
| Build Mini CLI | `cmake --build build --preset mini` |
| **Debug: paula-cli** | Run & Debug panel (`F5`) → _Debug: paula-cli_ |
| **Debug: paula-test** | Run & Debug panel → _Debug: paula-test_ |
| **Debug: paula-example** | Run & Debug panel → _Debug: paula-example_ |
| **Debug: paula-c-example** | Run & Debug panel → _Debug: paula-c-example_ |

Outputs: `build/Debug/`, `build/Release/`, `build/MinSizeRel/`.

To pass CLI arguments when launching from VSCode, edit the `"args"` array in [.vscode/launch.json](.vscode/launch.json), e.g.:
```json
"args": ["-f", "projects/test.pa"]
```
Or run the Debug executable directly from the VSCode terminal:
```
build\Debug\paula-cli.exe -f projects/test.pa
```

### Windows — command line (Developer Command Prompt for VS 2022)

```bat
cmake --preset default           :: configure once
cmake --build build --preset debug
cmake --build build --preset release
cmake --build build --preset mini
```

### Windows — Visual Studio 2022 (native IDE)

Open VS → **Open a local folder** → project root. VS detects `CMakePresets.json` and offers the same three build configurations. No `.sln` file — CMake is the build system.

### Linux — CMake

```bash
cmake --preset default
cmake --build build --preset debug    # build/Debug/paula-cli, paula-test, paula-example
cmake --build build --preset release  # build/Release/paula-cli
```

### Linux — Makefile (g++ shorthand)

```bash
make release   # → bin/paula         CLI release build
make debug     # → bin/pauladbg      Debug + ASan/LSan; runs unit tests on launch
make mini      # → bin/paulamini     Minimal CLI (bytecode input only)
make example   # → bin/paulaexample  Dev scratchpad
```

---

## CLI usage (paula-cli)

```
paula -f script.pa               # run a script from a file
paula -i                         # run script from stdin
paula -c script.pa out.bytecode  # compile script to a bytecode file
paula -b script.bytecode         # run bytecode from a file
```

**Mini build** only accepts bytecode from stdin:
```
paulamini < script.bytecode
```

`PAULA_DIR` environment variable is a fallback directory searched when a file path is not found directly.

---

## Target / config matrix

| Target | Debug | Release | MinSizeRel (mini) | Platform |
|---|---|---|---|---|
| paula-core | ✓ | ✓ | ✓ | all |
| paula-cli | ✓ | ✓ | ✓ | all |
| paula-test | ✓ | — | — | all |
| paula-example | ✓ | — | — | all |
| paula-c-example | ✓ | — | — | all |
| paula-dll | ✓ | ✓ | — | Windows only |

`paula-test` and `paula-example` are Debug-only because they depend on `test.h` functions compiled out (`#ifndef PAULA_RELEASE`) in Release and MinSizeRel. `paula-c-example` has no such dependency but is kept Debug-only to match the other dev-tool targets. The `release` and `mini` build presets target only `paula-cli`.

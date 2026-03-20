
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

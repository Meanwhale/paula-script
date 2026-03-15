# ── Paula Script Build ────────────────────────────────────────────────
#
# Targets:
#   paula-core     Static library – the interpreter engine (src/)
#   paula-cli      Command-line interpreter – reads and runs .paula scripts
#   paula-test     Unit tests – exercises core via test.h              (Debug only)
#   paula-example  Demo application – shows embedding the engine       (Debug only)
#   paula-dll      Windows DLL – exposes the engine as a shared lib    (Windows only)
#
# Build presets (see CMakePresets.json):
#   debug    – all targets,  defines PAULA_DEBUG
#   release  – cli only,     defines PAULA_RELEASE
#   mini     – cli only,     defines PAULA_RELEASE + PAULA_MINI, size-optimized
#
# VSCode run & debug (Run and Debug panel, F5):
#   "Debug: paula-cli"      Launch the CLI interpreter (Debug build)
#   "Debug: paula-test"     Run unit tests             (Debug build)
#   "Debug: paula-example"  Run the demo application   (Debug build)
#   All configurations build via the "build-debug" task before launching.
#
# Prerequisites:
#   Windows  – Visual Studio 2022 (MSVC, CMake, and Ninja included)
#   Linux    – CMake 3.21+, Ninja, and a C++17 compiler (g++ or clang++)
#
# ──────────────────────────────────────────────────────────────────────

cmake_minimum_required(VERSION 3.21)
project(paula-script LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Global MSVC settings ──────────────────────────────────────────────

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
if(WIN32 AND NOT MSVC)
    message(FATAL_ERROR "On Windows, only MSVC (cl.exe) is supported. "
        "Found: ${CMAKE_CXX_COMPILER_ID}. "
        "Launch VS Code from a Developer Command Prompt for VS 2022.")
endif()
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W3 /permissive-)
    set(MINI_COMPILE_FLAGS /Os /Ob0 /Gy /GF /GL /GS-)
    set(MINI_LINK_FLAGS /LTCG /OPT:REF /OPT:ICF)
endif()

# ── paula-core (static library) ───────────────────────────────────────

add_library(paula-core STATIC
    src/args.cpp
    src/byteautomata.cpp
    src/command.cpp
    src/defs.cpp
    src/engine.cpp
    src/paula.cpp
    src/stack.cpp
    src/stream.cpp
    src/test.cpp
    src/tree.cpp
    src/utils.cpp
    # Headers listed for IDE file navigation
    src/args.h
    src/array.h
    src/byteautomata.h
    src/command.h
    src/defs.h
    src/engine.h
    src/node_types.h
    src/paula.h
    src/scanner.h
    src/stack.h
    src/stream.h
    src/test.h
    src/tree.h
    src/utils.h
)

target_include_directories(paula-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Per-configuration definitions, propagated to all consumers via PUBLIC so
# that header declarations (paula.h, test.h, etc.) match the compiled library.
target_compile_definitions(paula-core PUBLIC
    $<$<CONFIG:Debug>:PAULA_DEBUG>
    $<$<CONFIG:Release>:PAULA_RELEASE>
    $<$<CONFIG:MinSizeRel>:PAULA_RELEASE;PAULA_MINI>
)

if(MSVC)
    # MinSizeRel mirrors the original "Mini" VS configuration
    target_compile_options(paula-core PRIVATE $<$<CONFIG:MinSizeRel>:${MINI_COMPILE_FLAGS}>)
    target_link_options(paula-core INTERFACE $<$<CONFIG:MinSizeRel>:${MINI_LINK_FLAGS}>)
endif()

# ── paula-test (unit tests, Debug only) ───────────────────────────────
# test.h/test.cpp are entirely guarded by #ifndef PAULA_RELEASE, so
# paula-test can only build in Debug. The Release/Mini build presets
# target only the shipping executables (paula-cli, paula-dll).

add_executable(paula-test projects/paula-vs/paula-vs.cpp)
target_link_libraries(paula-test PRIVATE paula-core)

target_compile_definitions(paula-test PRIVATE
    $<$<CONFIG:Debug>:VERBOSE>
)

# ── paula-cli (command-line interface) ────────────────────────────────

add_executable(paula-cli projects/paula-cli/paula-cli.cpp)
target_link_libraries(paula-cli PRIVATE paula-core)

if(MSVC)
    # MinSizeRel: aggressive size optimization; /GR- also disables RTTI
    target_compile_options(paula-cli PRIVATE $<$<CONFIG:MinSizeRel>:${MINI_COMPILE_FLAGS} /GR->)
    target_link_options(paula-cli PRIVATE $<$<CONFIG:MinSizeRel>:${MINI_LINK_FLAGS}>)
endif()

# ── paula-example (demo application, Debug only) ─────────────────────
# Calls test functions from test.h (guarded by #ifndef PAULA_RELEASE),
# so this target also only builds in Debug.

add_executable(paula-example projects/paula-example/paula-example.cpp)
target_link_libraries(paula-example PRIVATE paula-core)

target_compile_definitions(paula-example PRIVATE
    $<$<CONFIG:Debug>:VERBOSE>
)

# ── paula-dll (dynamic library, Windows only) ────────────────────────

if(WIN32)
    add_library(paula-dll SHARED projects/paula-dll/dllmain.cpp)
    target_link_libraries(paula-dll PRIVATE paula-core)

    target_compile_definitions(paula-dll PRIVATE PAULADLL_EXPORTS)

    # Output name varies by configuration (matches original VS setup)
    set_target_properties(paula-dll PROPERTIES
        OUTPUT_NAME_DEBUG       "paula_debug"
        OUTPUT_NAME_RELEASE     "paula_release"
        OUTPUT_NAME_MINSIZEREL  "paula_mini"
    )
endif()

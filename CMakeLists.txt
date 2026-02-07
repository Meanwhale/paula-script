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

# Per-configuration definitions (match original Visual Studio setup).
# NOTE: Each consumer target sets its own definitions independently to match
# how the original .vcxproj files were configured. This means some targets
# may see different header declarations than the library compiles — this
# reproduces the original VS behavior.
target_compile_definitions(paula-core PRIVATE
    $<$<CONFIG:Debug>:PAULA_DEBUG>
    $<$<CONFIG:Release>:PAULA_RELEASE>
    $<$<CONFIG:MinSizeRel>:PAULA_RELEASE;PAULA_MINI>
)

if(MSVC)
    # MinSizeRel mirrors the original "Mini" VS configuration
    target_compile_options(paula-core PRIVATE
        $<$<CONFIG:MinSizeRel>:/Os /Ob0 /Gy /GF /GL /GS->
    )
    target_link_options(paula-core INTERFACE
        $<$<CONFIG:MinSizeRel>:/LTCG /OPT:REF /OPT:ICF>
    )
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

target_compile_definitions(paula-cli PRIVATE
    $<$<CONFIG:Debug>:PAULA_DEBUG>
    $<$<CONFIG:Release>:PAULA_RELEASE>
    $<$<CONFIG:MinSizeRel>:PAULA_RELEASE;PAULA_MINI>
)

if(MSVC)
    # MinSizeRel: aggressive size optimization (mirrors "Mini" config)
    target_compile_options(paula-cli PRIVATE
        $<$<CONFIG:MinSizeRel>:/Os /Ob0 /GR- /Gy /GF /GL /GS->
    )
    target_link_options(paula-cli PRIVATE
        $<$<CONFIG:MinSizeRel>:/LTCG /OPT:REF /OPT:ICF>
    )
endif()

# ── paula-example (demo application, Debug only) ─────────────────────
# Calls test functions from test.h (guarded by #ifndef PAULA_RELEASE),
# so this target also only builds in Debug.

add_executable(paula-example projects/paula-example/paula-example.cpp)
target_link_libraries(paula-example PRIVATE paula-core)

target_compile_definitions(paula-example PRIVATE
    $<$<CONFIG:Release>:VERBOSE>
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

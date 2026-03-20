cmake_minimum_required(VERSION 3.21)
project(paula-script)

set(CMAKE_CXX_STANDARD 17)

file(GLOB CORE_SOURCES "src/*.cpp")

add_library(paula-core STATIC ${CORE_SOURCES})
target_include_directories(paula-core PUBLIC src)
target_compile_definitions(paula-core PRIVATE PAULA_DEBUG)

add_executable(paula-cli projects/paula-cli/paula-cli.cpp)
target_link_libraries(paula-cli PRIVATE paula-core)
target_compile_definitions(paula-cli PRIVATE PAULA_DEBUG)

add_executable(paula-example projects/paula-example/paula-example.cpp)
target_link_libraries(paula-example PRIVATE paula-core)
target_compile_definitions(paula-example PRIVATE PAULA_DEBUG)

add_library(paula-dll SHARED
    ${CORE_SOURCES}
    projects/paula-dll/dllmain.cpp
    projects/paula-c/paula_c.cpp
)
target_include_directories(paula-dll PRIVATE src projects/paula-c)
target_compile_definitions(paula-dll PRIVATE PAULA_DLL_BUILD PAULA_RELEASE)

add_executable(paula-c-example projects/paula-c/paula_c_example.c)
target_include_directories(paula-c-example PRIVATE projects/paula-c)
target_link_libraries(paula-c-example PRIVATE paula-dll)
target_compile_definitions(paula-c-example PRIVATE PAULA_DLL_USE)

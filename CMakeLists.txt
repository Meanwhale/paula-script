cmake_minimum_required(VERSION 3.21)
project(paula-script)

set(CMAKE_CXX_STANDARD 17)

file(GLOB CORE_SOURCES "src/*.cpp")
add_library(paula-core STATIC ${CORE_SOURCES})
target_include_directories(paula-core PUBLIC src)
target_compile_definitions(paula-core PUBLIC PAULA_DEBUG)

add_executable(paula-cli projects/paula-cli/paula-cli.cpp)
target_link_libraries(paula-cli PRIVATE paula-core)

add_executable(paula-example projects/paula-example/paula-example.cpp)
target_link_libraries(paula-example PRIVATE paula-core)

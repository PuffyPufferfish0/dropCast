cmake_minimum_required(VERSION 3.15)
project(DropCastClientAndroid LANGUAGES CXX C)

set(PLATFORM "Android" CACHE STRING "Target platform for Raylib" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

# Fetch Raylib
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# Compile the Client Source
add_library(main SHARED ../Client/src/main.cpp)

target_link_options(main PRIVATE "-Wl,-u,ANativeActivity_onCreate")
target_link_libraries(main PRIVATE raylib log android EGL GLESv2 OpenSLES m)
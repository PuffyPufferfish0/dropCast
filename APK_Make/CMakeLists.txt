cmake_minimum_required(VERSION 3.15)
project(DropCastAndroid)

# Tell Raylib to build for Android, NOT Desktop Linux!
set(PLATFORM "Android" CACHE STRING "Target platform for Raylib" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Android requires compiling Raylib for ARM64 from source
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# Compile the Host code as an Android shared library (libmain.so)
add_library(main SHARED ../Host/src/main.cpp)

# CRITICAL FIX: Stop the C++ compiler from deleting the Android entry point!
target_link_options(main PRIVATE "-Wl,-u,ANativeActivity_onCreate")

# Link Android specific libraries alongside Raylib
target_link_libraries(main PRIVATE raylib log android EGL GLESv2 OpenSLES m)

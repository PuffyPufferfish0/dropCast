cmake_minimum_required(VERSION 3.15)
project(DropCastAndroid LANGUAGES CXX C) # Added C language

set(PLATFORM "Android" CACHE STRING "Target platform for Raylib" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

# 1. Fetch Raylib
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch Lua Engine for Android
FetchContent_Declare(
    lua_src
    URL https://www.lua.org/ftp/lua-5.4.6.tar.gz
)
FetchContent_MakeAvailable(lua_src)
file(GLOB LUA_SOURCES "${lua_src_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES "${lua_src_SOURCE_DIR}/src/lua.c" "${lua_src_SOURCE_DIR}/src/luac.c")
add_library(lua_lib STATIC ${LUA_SOURCES})
target_include_directories(lua_lib PUBLIC ${lua_src_SOURCE_DIR}/src)

# Compile Host
add_library(main SHARED ../Host/src/main.cpp)

target_link_options(main PRIVATE "-Wl,-u,ANativeActivity_onCreate")
# Link Lua alongside Raylib
target_link_libraries(main PRIVATE raylib lua_lib log android EGL GLESv2 OpenSLES m)
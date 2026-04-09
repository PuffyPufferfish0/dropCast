cmake_minimum_required(VERSION 3.20)
# CRITICAL: Added 'C' so CMake knows how to compile the Lua engine!
project(DropCast VERSION 1.0 LANGUAGES CXX C) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Fetch & Build Lua 5.4 Engine ---
include(FetchContent)
FetchContent_Declare(
    lua_src
    URL https://www.lua.org/ftp/lua-5.4.6.tar.gz
)
FetchContent_MakeAvailable(lua_src)
# Grab all Lua C files except the standalone console runners
file(GLOB LUA_SOURCES "${lua_src_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES "${lua_src_SOURCE_DIR}/src/lua.c" "${lua_src_SOURCE_DIR}/src/luac.c")
add_library(lua_lib STATIC ${LUA_SOURCES})
target_include_directories(lua_lib PUBLIC ${lua_src_SOURCE_DIR}/src)

# --- 2. Find Raylib ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

add_subdirectory(Host)
add_subdirectory(Client)
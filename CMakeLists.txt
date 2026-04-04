cmake_minimum_required(VERSION 3.20)
project(DropCast VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Raylib via PkgConfig (This uses the fast Nix Linux version)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Tell CMake to look in these two folders next
add_subdirectory(Host)
add_subdirectory(Client)
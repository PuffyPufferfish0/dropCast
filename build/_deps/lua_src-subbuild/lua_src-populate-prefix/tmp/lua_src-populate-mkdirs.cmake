# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/puffy/dropCast/build/_deps/lua_src-src")
  file(MAKE_DIRECTORY "/home/puffy/dropCast/build/_deps/lua_src-src")
endif()
file(MAKE_DIRECTORY
  "/home/puffy/dropCast/build/_deps/lua_src-build"
  "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix"
  "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/tmp"
  "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/src/lua_src-populate-stamp"
  "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/src"
  "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/src/lua_src-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/src/lua_src-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/puffy/dropCast/build/_deps/lua_src-subbuild/lua_src-populate-prefix/src/lua_src-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

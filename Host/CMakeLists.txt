project(DropCast_Host)

add_executable(HostApp src/main.cpp)

target_include_directories(HostApp PRIVATE ${RAYLIB_INCLUDE_DIRS})
# CRITICAL: Link the new lua_lib to the Host
target_link_libraries(HostApp PRIVATE ${RAYLIB_LIBRARIES} lua_lib) 
target_compile_options(HostApp PRIVATE ${RAYLIB_CFLAGS_OTHER})
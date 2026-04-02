project(DropCast_Client)

add_executable(ClientApp src/main.cpp)

target_include_directories(ClientApp PRIVATE ${RAYLIB_INCLUDE_DIRS})
target_link_libraries(ClientApp PRIVATE ${RAYLIB_LIBRARIES})
target_compile_options(ClientApp PRIVATE ${RAYLIB_CFLAGS_OTHER})
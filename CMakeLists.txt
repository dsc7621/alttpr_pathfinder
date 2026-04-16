cmake_minimum_required(VERSION 4.2)
project(alttpr_pathfinder C)

set(CMAKE_C_STANDARD 17)

add_executable(alttpr_pathfinder src/main.c
        src/world.h
        src/world.c
        src/locations.h
        src/util/file_util.h
        src/util/file_util.c
        src/locations.c
        src/util/string_util.h
        src/util/string_util.c
        src/util/macros.h
        src/deque.h
        src/deque.c
        src/world_ops.c
        src/world_impl.h
        src/world_impl.c
)

target_compile_definitions(alttpr_pathfinder PRIVATE
        $<$<CONFIG:Debug>:DEBUG=0>
)

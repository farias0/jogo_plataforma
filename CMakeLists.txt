# https://github.com/raysan5/raylib/wiki/Working-with-CMake


cmake_minimum_required(VERSION 3.15) # required by raylib

project(jogo_plataforma)

add_subdirectory(raylib)

set(CMAKE_C_STANDARD 11) # required by raylib

add_executable(${PROJECT_NAME} src/game.c src/level/player.c src/linked_list.c src/level/enemy.c
    src/level/level.c src/camera.c src/core.c src/render.c src/input.c src/editor.c src/assets.c
    src/overworld.c src/level/block.c src/files.c src/persistence.c src/level/powerups.c)

set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

# required by raylib
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

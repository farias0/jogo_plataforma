# https://github.com/raysan5/raylib/wiki/Working-with-CMake


cmake_minimum_required(VERSION 3.15) # required by raylib

project(jogo_plataforma)

add_subdirectory(raylib)

set(CMAKE_C_STANDARD 11) # required by raylib
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} src/game.cpp src/level/player.cpp src/linked_list.cpp src/level/enemy.cpp
    src/level/level.cpp src/camera.cpp src/core.cpp src/render.cpp src/input.cpp src/editor.cpp src/assets.cpp
    src/overworld.cpp src/level/block.cpp src/files.cpp src/persistence.cpp src/level/powerups.cpp src/debug.cpp
    src/text_bank.cpp src/sounds.cpp src/level/grappling_hook.cpp src/animation.cpp)

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

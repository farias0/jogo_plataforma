# https://github.com/raysan5/raylib/wiki/Working-with-CMake


cmake_minimum_required(VERSION 3.15) # required by raylib

project(jogo_plataforma)

find_package(raylib 4.5 REQUIRED)

set(CMAKE_C_STANDARD 11) # required by raylib

add_executable(${PROJECT_NAME} src/game.c src/entities/player.c src/entities/entity.c src/entities/enemy.c
    src/entities/level.c src/entities/camera.c src/global.c src/render.c src/input.c src/editor.c src/assets.c)

set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib m) # 'm' is libm

# required by raylib
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
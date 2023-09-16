#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entities/entity.h"
#include "entities/player.h"
#include "entities/enemy.h"
#include "entities/level.h"
#include "entities/camera.h"
#include "render.h"
#include "input.h"

void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    STATE = MemAlloc(sizeof(GameState));
    ENTITIES = 0;
    PLAYER = 0;
    CAMERA = 0;

    // debug
    STATE->gamepadIdx = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    SetTargetFPS(60);

    ResetGameState();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            if (IsKeyDown(KEY_BACKSPACE)) {
                int __dummy; __dummy++; // DEBUG: Put breakpoint in here
            }


            /*
                IMPORTANT: Input handling must be done before ticking -- otheriwise
                collision checking, which is done in the tick functions, might break. 
            */

            HandleInput();
            if (STATE->isPaused) goto render;

            TickAllEntities(ENTITIES, PLAYER);
            if (STATE->isPaused) goto render;

            updateWindowTitle();
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);

            RenderBackground();

            RenderAllEntities();

            RenderHUD();

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
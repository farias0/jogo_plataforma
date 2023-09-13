#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "level.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "collision.h"

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


            HandleInput();
            if (STATE->isPaused) goto render;

            CheckForCollisions();
            if (STATE->isPaused) goto render;

            TickAllEntities(ENTITIES, PLAYER);

            updateWindowTitle();
        }

render:
        { // Game Render
            BeginDrawing();

            ClearBackground(BLACK);

            RenderAllEntities();

            RenderHUD();

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
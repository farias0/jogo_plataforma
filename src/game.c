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
#include "assets.h"


void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    // debug
    int gamepadIdx = 0;
    if (IsGamepadAvailable(gamepadIdx))
        TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);


    SetTraceLogLevel(LOG_DEBUG);

    InitWindow(SCREEN_WIDTH_FULL, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    InitializeAssets();

    InitializeGameState();

    InitializeOverworld();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            /*
                IMPORTANT: Input handling must be done before ticking -- otheriwise
                collision checking, which is done in the tick functions, might break. 
            */

            HandleInput();
            if (STATE->isPaused) goto render;

            TickAllEntities(ENTITIES_HEAD, PLAYER);
            if (STATE->isPaused) goto render;

            updateWindowTitle();
        }

render:
        {
            BeginDrawing();

            Render();

            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
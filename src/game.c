#include <raylib.h>
#include <stdio.h>

#include "global.h"
#include "inlevel/player.h"
#include "inlevel/enemy.h"
#include "inlevel/level.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "assets.h"
#include "overworld.h"


void updateWindowTitle() {
    char title[50];
    sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());
    SetWindowTitle(title);
}

int main(int argc, char **argv)
{
    SetTraceLogLevel(LOG_DEBUG);

    // debug
    int gamepadIdx = 0;
    if (IsGamepadAvailable(gamepadIdx))
        TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    InitializeAssets();

    InitializeGameState();

    CameraInitialize();

    OverworldInitialize();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            /*
                IMPORTANT: Input handling must be done before ticking -- otheriwise
                collision checking, which is done in the tick functions, might break. 
            */

            HandleInput();
            if (STATE->isPaused) goto render;

            if (STATE->mode == MODE_IN_LEVEL) LevelTick();
            else if (STATE->mode == MODE_OVERWORLD) OverworldTick();
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

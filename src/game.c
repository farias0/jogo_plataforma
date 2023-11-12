#include <raylib.h>
#include <stdio.h>

#include "core.h"
#include "level/level.h"
#include "camera.h"
#include "render.h"
#include "input.h"
#include "assets.h"
#include "overworld.h"


int main() {

    SetTraceLogLevel(LOG_DEBUG);

    // debug
    int gamepadIdx = 0;
    if (IsGamepadAvailable(gamepadIdx))
        TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    AssetsInitialize();

    GameStateInitialize();

    CameraInitialize();

    RenderInitialize();

    OverworldInitialize();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        { // Game Update

            /*
                IMPORTANT: Input handling must be done before ticking -- otheriwise
                collision checking, which is done in the tick functions, might break. 
            */

            InputHandle();
            if (STATE->isPaused) goto render;

            if (STATE->mode == MODE_IN_LEVEL) LevelTick();
            else if (STATE->mode == MODE_OVERWORLD) OverworldTick();
            if (STATE->isPaused) goto render;

            WindowTitleUpdate();
        }

render:
        Render();
    }

    CloseWindow();
    return 0;
}

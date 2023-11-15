#include <raylib.h>

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
        InputHandle();

        if (STATE->mode == MODE_IN_LEVEL) LevelTick();
        else if (STATE->mode == MODE_OVERWORLD) OverworldTick();

        WindowTitleUpdate();

        Render();
    }

    CloseWindow();
    return 0;
}

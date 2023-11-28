#include <raylib.h>

#include "core.h"
#include "render.h"
#include "input.h"
#include "overworld.h"

int main() {

    SetTraceLogLevel(LOG_DEBUG);

    // debug
    int gamepadIdx = 0;
    if (IsGamepadAvailable(gamepadIdx))
        TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    SystemsInitialize();

    GameStateReset();

    OverworldInitialize();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        InputHandle();

        GameUpdate();

        Render();
    }

    CloseWindow();
    return 0;
}

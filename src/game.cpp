#include <raylib.h>

#include "core.hpp"
#include "render.hpp"
#include "input.hpp"
#include "overworld.hpp"

int main() {

    SetTraceLogLevel(LOG_DEBUG);

    // debug
    TraceLog(LOG_DEBUG, "Searching gamepads...");
    for (int gamepadIdx = 0; gamepadIdx < 10000; gamepadIdx++) {
        if (IsGamepadAvailable(gamepadIdx))
            TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    SystemsInitialize();

    GameStateReset();

    OverworldLoad();

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Input::Handle();

        if (GetKeyPressed()) {
            TraceLog(LOG_DEBUG, "Key pressed=%i.", GetKeyPressed());
        }

        GameUpdate();

        Render();
    }

    CloseWindow();
    return 0;
}

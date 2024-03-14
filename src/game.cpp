#include <raylib.h>

#include "core.hpp"
#include "render.hpp"
#include "input.hpp"
#include "overworld.hpp"

int main() {

    SetTraceLogLevel(LOG_DEBUG);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");
    
    SetTargetFPS(60);

    SystemsInitialize();

    GameStateReset();

    OverworldLoad();

    // debug
    TraceLog(LOG_DEBUG, "Searching gamepads...");
    for (int gamepadIdx = 0; gamepadIdx < 10000; gamepadIdx++) {
        if (IsGamepadAvailable(gamepadIdx))
            TraceLog(LOG_DEBUG, "Gamepad detected. Index=%i.", gamepadIdx);
    }

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        int keyPressed = GetGamepadButtonPressed();
        if (keyPressed) {
            TraceLog(LOG_INFO, "GPPressed=%i.", keyPressed);
        }
        keyPressed = GetKeyPressed();
        if (keyPressed) {
            TraceLog(LOG_INFO, "KeyPressed=%i.", keyPressed);
        }

        Input::Handle();

        int a = GetGamepadButtonPressed(); 
        if (a) {
            TraceLog(LOG_DEBUG, "Key pressed=%i.", a);
        }

        GameUpdate();

        Render();
    }

    CloseWindow();
    return 0;
}

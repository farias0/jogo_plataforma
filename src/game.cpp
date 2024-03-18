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

    TraceLog(LOG_INFO, "Gamepad name::");
    TraceLog(LOG_INFO, GetGamepadName(0));


    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        int keyPressed = GetGamepadButtonPressed();
        if (keyPressed) {
            TraceLog(LOG_INFO, "GPPressed=%i.", keyPressed);
        }
        keyPressed = GetKeyPressed();
        if (keyPressed) {
            TraceLog(LOG_INFO, "KeyPressed=%i.", keyPressed);
            if (keyPressed == 32) {
                TraceLog(LOG_INFO, "DEBUGGIGN");
            }
        }

        Input::Handle();

        GameUpdate();

        Render();
    }

    CloseWindow();
    return 0;
}

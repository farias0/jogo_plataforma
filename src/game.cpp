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

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Input::Handle();

        GameUpdate();

        Render::Render();
    }

    CloseWindow();
    return 0;
}

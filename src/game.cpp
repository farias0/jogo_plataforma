#include <raylib.h>

#include "core.hpp"
#include "render.hpp"
#include "input.hpp"
#include "overworld.hpp"

void initWindow() {

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo de plataforma");

    /*
        Windows for some reason tries to resize the window to slightly lower than 1080p (vertical)
        in my 1080p monitor in windowed mode.

        After entering and then exiting fullscreen we just have a image that's cropped in the
        bottom, which is ok and fixable with only SetWindowMinSize(), if desired.
        
        But when the game first launches in windowed mode, for some reason,
        the rendering is dislocated upwards, putting it off sync with the game internal logic and
        breaking the mouse interactions.

        This combination of commands fixes this. 
    */
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main() {

    SetTraceLogLevel(LOG_DEBUG);

    initWindow();

    SetExitKey(KEY_NULL); 

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

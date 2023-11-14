#include "math.h"
#include <stdio.h>

#include "core.h"
#include "overworld.h"
#include "camera.h"
#include "level/level.h"
#include "editor.h"
#include "string.h"
#include "render.h"


GameState *STATE = 0;

void GameStateInitialize() {
    STATE = MemAlloc(sizeof(GameState));

    // Debug
    STATE->showDebugHUD = false;
    STATE->showBackground = false;

    EditorDisable();

    TraceLog(LOG_INFO, "Game state initialized.");
}

void GameStateReset() {
    strcpy(STATE->loadedLevel, "");
    STATE->isPaused = false;
    STATE->mode = MODE_IN_LEVEL;

    TraceLog(LOG_DEBUG, "Game state reset.");
}

void PausedGameToggle() {
    
    if (STATE->isPaused) {

        if (STATE->mode == MODE_IN_LEVEL) {

            if (LEVEL_PLAYER_STATE) {
                if (LEVEL_PLAYER_STATE->isDead) LevelPlayerContinue();
            } else {
                TraceLog(LOG_ERROR, "Pause toggle in level has no reference to player's state.");
            }
        }

        STATE->isPaused = false;

    } else {
        STATE->isPaused = true;
    }
}

ListNode *GetEntityListHead() {

    if (STATE->mode == MODE_IN_LEVEL) return LEVEL_LIST_HEAD;
    else if (STATE->mode == MODE_OVERWORLD) return OW_LIST_HEAD;
    else return 0;
}

void WindowTitleUpdate() {
    char title[LEVEL_NAME_BUFFER_SIZE + 20];

    if (STATE->loadedLevel[0] == '\0')
        sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());    
    else
        sprintf(title, "%s - %d FPS", STATE->loadedLevel, GetFPS());

    SetWindowTitle(title);
}

void DebugHudEnable() {

    STATE->showDebugHUD = true;
    ShowCursor();

    TraceLog(LOG_TRACE, "Debug hud enabled.");
}

void DebugHudDisable() {

    STATE->showDebugHUD = false;
    
    HideCursor();

    CameraPanningReset();

    TraceLog(LOG_TRACE, "Debug hud disabled.");
}

void DebugHudToggle() {

    if (STATE->showDebugHUD) DebugHudDisable();
    else DebugHudEnable();
}

bool IsInPlayArea(Vector2 pos) {
    return pos.x >= 0 &&
            pos.x <= SCREEN_WIDTH &&
            pos.y >= 0 &&
            pos.y <= SCREEN_HEIGHT;
}

float SnapToGrid(float value, float length) {

    if (value >= 0) {
        return value - fmod(value, length);
    } else {
        return value - length - fmod(value, length);
    }
}

float PushOnGrid(float value, float length) {

    if (value >= 0) {
        return length - fmod(value, length);
    } else {
        return - length - fmod(value, length);
    }
}

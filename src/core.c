#include "math.h"

#include "core.h"
#include "overworld.h"
#include "camera.h"
#include "level/level.h"
#include "editor.h"
#include "string.h"


GameState *STATE = 0;

void GameStateInitialize() {
    STATE = MemAlloc(sizeof(GameState));

    STATE->isEditorEnabled = true;
    EditorEnabledToggle(); // To hide mouse cursor

    // Debug
    STATE->showDebugHUD = false;
    STATE->showBackground = false;

    TraceLog(LOG_INFO, "Game state initialized.");
}

void GameStateReset() {
    strcpy(STATE->loadedLevel, "");
    STATE->isPaused = false;
    STATE->isPlayerDead = false;
    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;
    STATE->mode = MODE_IN_LEVEL;

    TraceLog(LOG_DEBUG, "Game state reset.");
}

void PausedGameToggle() {
    if (STATE->isPaused) {

        if (STATE->isPlayerDead) {
            LevelPlayerContinue();
        } else {
            STATE->isPaused = false;
        }

    } else {
        STATE->isPaused = true;
    }
}

void EditorEnabledToggle() {
    STATE->isEditorEnabled = !STATE->isEditorEnabled;

    if (STATE->isEditorEnabled) {
        SetWindowSize(SCREEN_WIDTH_W_EDITOR, SCREEN_HEIGHT);
        ShowCursor();
        TraceLog(LOG_TRACE, "Editor enabled.");
    } else {
        SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        HideCursor();
        TraceLog(LOG_TRACE, "Editor disabled.");
    }
}

ListNode *GetEntityListHead() {

    if (STATE->mode == MODE_IN_LEVEL) return LEVEL_LIST_HEAD;
    else if (STATE->mode == MODE_OVERWORLD) return OW_LIST_HEAD;
    else return 0;
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

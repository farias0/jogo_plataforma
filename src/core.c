#include "math.h"

#include "core.h"
#include "overworld.h"
#include "camera.h"
#include "inlevel/level.h"
#include "editor.h"


GameState *STATE = 0;

void GameStateInitialize() {
    STATE = MemAlloc(sizeof(GameState));

    STATE->isEditorEnabled = false;

    // Debug
    STATE->showDebugHUD = false;
    STATE->showBackground = false;

    TraceLog(LOG_INFO, "Game state initialized.");
}

void GameStateReset() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;
    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;
    STATE->mode = MODE_IN_LEVEL;

    TraceLog(LOG_INFO, "Game state reset.");
}

void GameModeToggle() {

    switch (STATE->mode)
    {
    case MODE_IN_LEVEL:
        OverworldInitialize();
        break;
    
    case MODE_OVERWORLD:
        LevelInitialize();
        break;

    default:
        TraceLog(LOG_WARNING, "No code to handle toggle game mode for mode %d.", STATE->mode);
    }
}

void StateInLevelToggle() {
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
        TraceLog(LOG_INFO, "Editor enabled.");
    } else {
        SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        TraceLog(LOG_INFO, "Editor disabled.");
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

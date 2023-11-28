#include "math.h"
#include <stdio.h>

#include "core.h"
#include "overworld.h"
#include "camera.h"
#include "level/level.h"
#include "level/player.h"
#include "editor.h"
#include "string.h"
#include "render.h"


GameState *GAME_STATE = 0;

static size_t mouseEnabledReferences = 0;


static inline float snapToGrid(float value, float length) {

    if (value >= 0) {
        return value - fmod(value, length);
    } else {
        return value - length - fmod(value, length);
    }
}

static inline float distanceFromGrid(float value, float length) {

    if (value >= 0) {
        return length - fmod(value, length);
    } else {
        return - length - fmod(value, length);
    }
}

void windowTitleUpdate() {

    char title[LEVEL_NAME_BUFFER_SIZE + 20];

    if (GAME_STATE->loadedLevel[0] == '\0')
        sprintf(title, "Jogo de Plataforma - %d FPS", GetFPS());    
    else
        sprintf(title, "%s - %d FPS", GAME_STATE->loadedLevel, GetFPS());

    SetWindowTitle(title);
}

void GameStateInitialize() {

    GAME_STATE = MemAlloc(sizeof(GameState));

    GAME_STATE->showBackground = false;
    GAME_STATE->showDebugHUD = false;

    EditorDisable();

    TraceLog(LOG_INFO, "Game state initialized.");
}

void GameStateReset() {

    strcpy(GAME_STATE->loadedLevel, "");
    GAME_STATE->isPaused = false;
    GAME_STATE->mode = MODE_IN_LEVEL;

    TraceLog(LOG_DEBUG, "Game state reset.");
}

void SystemsInitialize() {

    AssetsInitialize();
    GameStateInitialize();
    CameraInitialize();
    EditorInitialize();
    RenderInitialize();
}

void GameUpdate() {

    if (GAME_STATE->mode == MODE_IN_LEVEL)
        LevelTick();
    else if (GAME_STATE->mode == MODE_OVERWORLD)
        OverworldTick();

    if (EDITOR_STATE->isEnabled)
        EditorTick();

    windowTitleUpdate();
}

void PausedGameToggle() {
    
    if (GAME_STATE->isPaused) {

        if (GAME_STATE->mode == MODE_IN_LEVEL) {

            if (PLAYER_ENTITY) {
                if (PLAYER_ENTITY->isDead) PlayerContinue();
            } else {
                TraceLog(LOG_ERROR, "Pause toggle in level has no reference to player.");
            }
        }

        GAME_STATE->isPaused = false;

    } else {
        GAME_STATE->isPaused = true;
    }
}

ListNode *GetEntityListHead() {

    if (GAME_STATE->mode == MODE_IN_LEVEL) return LEVEL_STATE->listHead;
    else if (GAME_STATE->mode == MODE_OVERWORLD) return OW_LIST_HEAD;
    else return 0;
}

void DebugHudEnable() {

    GAME_STATE->showDebugHUD = true;
    MouseCursorEnable();
    TraceLog(LOG_TRACE, "Debug hud enabled.");
}

void DebugHudDisable() {

    GAME_STATE->showDebugHUD = false;
    MouseCursorDisable();
    CameraPanningReset();
    TraceLog(LOG_TRACE, "Debug hud disabled.");
}

void MouseCursorDisable() {

    if (mouseEnabledReferences > 0) mouseEnabledReferences--;
    if (mouseEnabledReferences == 0) HideCursor();
    TraceLog(LOG_TRACE, "Mouse enabled references down to %d.", mouseEnabledReferences);
}

void MouseCursorEnable() {

    mouseEnabledReferences++;
    ShowCursor();
    TraceLog(LOG_TRACE, "Mouse enabled references increased to %d.", mouseEnabledReferences);
}

void DebugHudToggle() {

    if (GAME_STATE->showDebugHUD) {
        RenderDebugEntityStopAll();
        DebugHudDisable();
    }
    else {
        DebugHudEnable();
    }
}

bool IsInPlayArea(Vector2 pos) {
    
    return pos.x >= 0 &&
            pos.x <= SCREEN_WIDTH &&
            pos.y >= 0 &&
            pos.y <= SCREEN_HEIGHT;
}

Vector2 SnapToGrid(Vector2 coords, Dimensions grid) {

    return (Vector2) {
        snapToGrid(coords.x, grid.width),
        snapToGrid(coords.y, grid.height)
    };
}

Vector2 DistanceFromGrid(Vector2 coords, Dimensions grid) {

    return (Vector2) {
        distanceFromGrid(coords.x, grid.width),
        distanceFromGrid(coords.y, grid.height)
    };
}

Vector2 RectangleGetPos(Rectangle rect) {
    return (Vector2){ rect.x, rect.y };
}

void RectangleSetPos(Rectangle *rect, Vector2 pos) {
    rect->x = pos.x;
    rect->y = pos.y;
}

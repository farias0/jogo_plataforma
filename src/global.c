#include "math.h"

#include "global.h"
#include "overworld.h"
#include "entities/entity.h"
#include "entities/camera.h"
#include "entities/level.h"
#include "editor.h"


GameState *STATE = 0;
Entity *ENTITIES_HEAD = 0;
Entity *PLAYER = 0;
Entity *CAMERA = 0;


// Continue game after dying
void playerContinue() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    SetEntityPosition(PLAYER, GetPlayerStartingPosition());

    TraceLog(LOG_INFO, "Player continue.");
}

void InitializeGameState() {
    STATE = MemAlloc(sizeof(GameState));

    STATE->isEditorEnabled = false;

    // Debug
    STATE->showDebugHUD = false;
    STATE->showBackground = false;

    TraceLog(LOG_INFO, "Game state initialized.");
}

void ResetGameState() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;
    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;
    STATE->mode = InLevel;

    TraceLog(LOG_INFO, "Game state reset.");
}

void InitializeLevel() {

    ResetGameState();
    STATE->mode = InLevel;

    ReloadEntityList();
    ENTITIES_HEAD = InitializePlayer(ENTITIES_HEAD, &PLAYER);
    ENTITIES_HEAD = LoadLevel(ENTITIES_HEAD);
    
    SetEntityPosition(PLAYER, GetPlayerStartingPosition());

    SyncEditor();

    TraceLog(LOG_INFO, "Initialized level.");
}

void InitializeOverworld() {

    ResetGameState();
    STATE->mode = Overworld;

    PLAYER = 0;
    ReloadEntityList();
    LoadOverworld();

    SyncEditor();

    TraceLog(LOG_INFO, "Initialized overworld.");
}

void ToggleGameMode() {

    switch (STATE->mode)
    {
    case InLevel:
        InitializeOverworld();
        break;
    
    case Overworld:
        InitializeLevel();
        break;

    default:
        TraceLog(LOG_WARNING, "No code to handle toggle game mode for mode %d.", STATE->mode);
    }
}

void ToggleInLevelState() {
    if (STATE->isPaused) {

        if (STATE->isPlayerDead) {
            playerContinue();
        } else {
            STATE->isPaused = false;
        }

    } else {
        STATE->isPaused = true;
    }
}

void ToggleEditorEnabled() {
    STATE->isEditorEnabled = !STATE->isEditorEnabled;

    if (STATE->isEditorEnabled) {
        SetWindowSize(SCREEN_WIDTH_FULL, SCREEN_HEIGHT);
        TraceLog(LOG_INFO, "Editor enabled.");
    } else {
        SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        TraceLog(LOG_INFO, "Editor disabled.");
    }
}

bool IsInPlayArea(Vector2 pos) {
    return pos.x >= 0 &&
            pos.x <= SCREEN_WIDTH &&
            pos.y >= 0 &&
            pos.y <= SCREEN_HEIGHT;
}

Vector2 PosInScreenToScene(Vector2 pos) {
    return (Vector2){
        pos.x + CAMERA->hitbox.x,
        pos.y + CAMERA->hitbox.y
    };
}

Vector2 PosInSceneToScreen(Vector2 pos) {
    return (Vector2){
        pos.x - CAMERA->hitbox.x,
        pos.y - CAMERA->hitbox.y
    };
}

float RoundValue(float value, float period) {

    if (value >= 0) {
        return period - fmod(value, period);
    } else {
        return - period - fmod(value, period);
    }
}
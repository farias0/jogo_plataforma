#include "global.h"
#include "overworld.h"
#include "entities/entity.h"
#include "entities/camera.h"
#include "entities/level.h"


GameState *STATE = 0;
Entity *ENTITIES_HEAD = 0;
Entity *PLAYER = 0;
Entity *CAMERA = 0;

void InitializeGameState() {
    STATE = MemAlloc(sizeof(GameState));

    STATE->editorSelectedItem = Block;

    TraceLog(LOG_INFO, "Game state initialized.");
}

void ResetGameState() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;
    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;
    STATE->mode = InLevel;

    TraceLog(LOG_INFO, "Game state reset.");
}

void PlayerContinue() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    SetEntityPosition(PLAYER, GetPlayerStartingPosition());

    TraceLog(LOG_INFO, "Player continue.");
}

void InitializeLevel() {

    ResetGameState();
    STATE->mode = InLevel;

    ReloadEntityList();
    ENTITIES_HEAD = InitializePlayer(ENTITIES_HEAD, &PLAYER);
    ENTITIES_HEAD = LoadLevel(ENTITIES_HEAD);
    
    SetEntityPosition(PLAYER, GetPlayerStartingPosition());

    TraceLog(LOG_INFO, "Initialized level.");
}

void InitializeOverworld() {

    ResetGameState();
    STATE->mode = Overworld;

    PLAYER = 0;
    ReloadEntityList();
    LoadOverworld();

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

bool IsInPlayArea(Vector2 pos) {
    return pos.x >= 0 &&
            pos.x <= SCREEN_WIDTH &&
            pos.y >= 0 &&
            pos.y <= SCREEN_HEIGHT;
}
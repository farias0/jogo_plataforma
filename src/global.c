#include "global.h"
#include "entity.h"
#include "camera.h"
#include "level.h"

GameState *STATE;
Entity *ENTITIES;
Entity *PLAYER;
Entity *CAMERA;


void ResetGameState() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;

    STATE->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    DestroyAllEntities(ENTITIES);
    PLAYER = InitializePlayer(0);
    ENTITIES = PLAYER;
    CAMERA = InitializeCamera(ENTITIES);
    InitializeLevel(ENTITIES);
}
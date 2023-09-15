#include "global.h"
#include "entities/entity.h"
#include "entities/camera.h"
#include "entities/level.h"

GameState *STATE;
Entity *ENTITIES;
Entity *PLAYER;
Entity *CAMERA;


void ResetGameState() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    DestroyAllEntities(ENTITIES);
    PLAYER = InitializePlayer(0);
    ENTITIES = PLAYER;
    CAMERA = InitializeCamera(ENTITIES);
    InitializeLevel(ENTITIES);
}
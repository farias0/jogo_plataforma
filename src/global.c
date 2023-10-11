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
    ENTITIES = InitializePlayer(0, &PLAYER);
    ENTITIES = InitializeCamera(ENTITIES, &CAMERA);
    ENTITIES = InitializeLevel(ENTITIES);
}

bool IsInPlayArea(Vector2 pos) {
    if (pos.x >= 0 && pos.x <= SCREEN_WIDTH &&
        pos.y >= 0 && pos.y <= SCREEN_HEIGHT) {

            return true;
        }

    return false;
}
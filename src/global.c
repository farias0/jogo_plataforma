#include "global.h"
#include "entities/entity.h"
#include "entities/camera.h"
#include "entities/level.h"

GameState *STATE;
Entity *ENTITIES_HEAD;
Entity *PLAYER;
Entity *CAMERA;


void ResetGameState() {
    STATE->isPaused = false;
    STATE->isPlayerDead = false;
    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    DestroyAllEntities(ENTITIES_HEAD);
    ENTITIES_HEAD = InitializePlayer(0, &PLAYER);
    ENTITIES_HEAD = InitializeCamera(ENTITIES_HEAD, &CAMERA);
    ENTITIES_HEAD = InitializeLevel(ENTITIES_HEAD);

    TraceLog(LOG_DEBUG, "Game state reset.");
}

bool IsInPlayArea(Vector2 pos) {
    if (pos.x >= 0 && pos.x <= SCREEN_WIDTH &&
        pos.y >= 0 && pos.y <= SCREEN_HEIGHT) {

            return true;
        }

    return false;
}
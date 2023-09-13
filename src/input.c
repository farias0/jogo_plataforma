#include <raylib.h>

#include "input.h"
#include "global.h"
#include "entities/level.h"
#include "entities/enemy.h"


#define CAMERA_SPEED 8.0f;

void handleGeneralInput() {
    if (STATE->isPaused) {
        if (IsKeyPressed(KEY_ENTER)) {
            if (STATE->isPlayerDead) {
                ResetGameState();
            } else {
                STATE->isPaused = false;
            }
        }
    }
    else if (IsKeyPressed(KEY_ENTER)) {
        STATE->isPaused = true;
    }
}

void handlePlayerInput() {

    Vector2 playerDelta = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_Z)) STATE->playerMovementType = PLAYER_MOVEMENT_RUNNING;
    else STATE->playerMovementType = PLAYER_MOVEMENT_DEFAULT;

    if (IsKeyDown(KEY_RIGHT))
        MovePlayer(PLAYER, STATE->playerMovementType, PLAYER_MOVEMENT_RIGHT);
    if (IsKeyDown(KEY_LEFT) && PLAYER->hitbox.x > 0)
        MovePlayer(PLAYER, STATE->playerMovementType, PLAYER_MOVEMENT_LEFT);

    if (IsKeyPressed(KEY_X)) PlayerStartJump(PLAYER);
}

void handleEditorInput() {

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // TODO use a timer to not keep checking it every frame

        Vector2 mousePos = GetMousePosition();
        Vector2 blockPos = {
            mousePos.x + CAMERA->hitbox.x,
            mousePos.y + CAMERA->hitbox.y
        };
        AddBlockToLevel(ENTITIES, blockPos);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

        Vector2 mousePos = GetMousePosition();
        Vector2 enemyPos = {
            mousePos.x + CAMERA->hitbox.x,
            mousePos.y + CAMERA->hitbox.y
        };
        bool didPosition = AddEnemyToLevel(ENTITIES, enemyPos);
    }
}

void handleCameraInput() {

    if (IsKeyDown(KEY_A)) CAMERA->hitbox.x -= CAMERA_SPEED;
    if (IsKeyDown(KEY_D)) CAMERA->hitbox.x += CAMERA_SPEED;
    if (IsKeyDown(KEY_W)) CAMERA->hitbox.y -= CAMERA_SPEED;
    if (IsKeyDown(KEY_S)) CAMERA->hitbox.y += CAMERA_SPEED;
}

void HandleInput() {

    handleGeneralInput();
    handlePlayerInput();
    handleEditorInput();
    handleCameraInput(); // debug
}
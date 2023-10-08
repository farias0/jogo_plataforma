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

    if (IsKeyDown(KEY_Z)) STATE->playerMovementSpeed = PLAYER_MOVEMENT_RUNNING;
    else STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    if (IsKeyDown(KEY_RIGHT))
        UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))
        UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_LEFT);
    else
        UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_STOP);

    if (IsKeyPressed(KEY_X)) PlayerStartJump(PLAYER);
}

void handleEditorInput() {

    Vector2 mousePosAbsolute = GetMousePosition();
    if (IsInPlayArea(mousePosAbsolute)) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // TODO use a timer to not keep checking it every frame

            Vector2 mousePos = {
                    mousePosAbsolute.x + CAMERA->hitbox.x,
                    mousePosAbsolute.y + CAMERA->hitbox.y
                };

            switch (STATE->editorSelectedItem)  {

            case Block:
                AddBlockToLevel(ENTITIES, mousePos);
                break;

            case Enemy:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // so holding doesn't keep placing
                    bool didPosition = AddEnemyToLevel(ENTITIES, mousePos);
                }

            default:
                // TODO log error
                break;
            }
        }
    }
}

void handleCameraInput() {

    // TODO move camera code to camera.c
    if (IsKeyDown(KEY_A)) CAMERA->hitbox.x -= CAMERA_SPEED;
    if (IsKeyDown(KEY_D)) CAMERA->hitbox.x += CAMERA_SPEED;
    if (IsKeyDown(KEY_W)) CAMERA->hitbox.y -= CAMERA_SPEED;
    if (IsKeyDown(KEY_S)) CAMERA->hitbox.y += CAMERA_SPEED;
}

void HandleInput() {

    handleGeneralInput();
    if (STATE->isPaused) return;
    
    handlePlayerInput();
    handleEditorInput();
    handleCameraInput(); // debug
}
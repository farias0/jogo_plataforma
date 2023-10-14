#include <raylib.h>

#include "input.h"
#include "global.h"
#include "entities/level.h"
#include "entities/enemy.h"
#include "overworld.h"


#define CAMERA_SPEED 8.0f;

void handleGeneralInput() {
    if (STATE->isPaused) {
        if (IsKeyPressed(KEY_ENTER)) {
            if (STATE->isPlayerDead) {
                PlayerContinue();
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

    if      (IsKeyDown(KEY_Z))              STATE->playerMovementSpeed = PLAYER_MOVEMENT_RUNNING;
    else                                    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    if      (IsKeyDown(KEY_RIGHT))          UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))           UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_LEFT);
    else                                    UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_STOP);

    if      (IsKeyPressed(KEY_X))           PlayerStartJump(PLAYER);

    if      (IsKeyPressed(KEY_BACKSPACE))   InitializeOverworld();
}

void handleOverworldInput() {

    if      (IsKeyPressed(KEY_X))           SelectLevel();

    if      (IsKeyPressed(KEY_UP))          OverworldMoveCursor(UP);
    else if (IsKeyPressed(KEY_DOWN))        OverworldMoveCursor(DOWN);
    else if (IsKeyPressed(KEY_LEFT))        OverworldMoveCursor(LEFT);
    else if (IsKeyPressed(KEY_RIGHT))       OverworldMoveCursor(RIGHT);
}

void handleEditorInput() {

    Vector2 mousePosInScreen = GetMousePosition();
    if (IsInPlayArea(mousePosInScreen)) {

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // TODO use a timer to not keep checking it every frame

            Vector2 mousePosInScene = {
                    mousePosInScreen.x + CAMERA->hitbox.x,
                    mousePosInScreen.y + CAMERA->hitbox.y
                };

            switch (STATE->editorSelectedItem)  {

            case Block:
                ENTITIES_HEAD = AddBlockToLevel(ENTITIES_HEAD, mousePosInScene);
                break;

            case Enemy:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { // so holding doesn't keep placing
                    ENTITIES_HEAD = AddEnemyToLevel(ENTITIES_HEAD, mousePosInScene);
                }
                break;

            case Eraser:
                ENTITIES_HEAD = DestroyEntityOn(ENTITIES_HEAD, mousePosInScene);
                break;

            default:
                TraceLog(LOG_WARNING, "No code to handle selected item %d.", STATE->editorSelectedItem);
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
    
    if (STATE->mode == InLevel) {
        handlePlayerInput();
    }

    else if (STATE->mode == Overworld) {
        handleOverworldInput();
    }

    handleEditorInput();
    handleCameraInput(); // debug
}
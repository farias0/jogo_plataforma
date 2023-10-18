#include <raylib.h>

#include "input.h"
#include "global.h"
#include "entities/level.h"
#include "entities/enemy.h"
#include "overworld.h"


#define CAMERA_SPEED 8.0f;


void handleInLevelInput() {

    if      (IsKeyPressed(KEY_F5))          STATE->showBackground = !STATE->showBackground;

    if      (IsKeyPressed(KEY_BACKSPACE))   { InitializeOverworld(); return; }

    if      (IsKeyPressed(KEY_ENTER))       { ToggleInLevelState(); return; }


    if (STATE->isPlayerDead) return;


    if      (IsKeyDown(KEY_Z))              STATE->playerMovementSpeed = PLAYER_MOVEMENT_RUNNING;
    else                                    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    if      (IsKeyDown(KEY_RIGHT))          UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))           UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_LEFT);
    else                                    UpdatePlayerHorizontalMovement(PLAYER_MOVEMENT_STOP);

    if      (IsKeyPressed(KEY_X))           PlayerStartJump(PLAYER);
}

void handleOverworldInput() {

    if      (IsKeyPressed(KEY_X))           { SelectLevel(); return; };

    if      (IsKeyPressed(KEY_UP))          OverworldMoveCursor(UP);
    else if (IsKeyPressed(KEY_DOWN))        OverworldMoveCursor(DOWN);
    else if (IsKeyPressed(KEY_LEFT))        OverworldMoveCursor(LEFT);
    else if (IsKeyPressed(KEY_RIGHT))       OverworldMoveCursor(RIGHT);
}

void handleEditorInput() {

    if      (IsKeyPressed(KEY_F1))           { ToggleEditorEnabled(); return; }

    // Debug
    if      (IsKeyPressed(KEY_F2))          STATE->showDebugHUD = !STATE->showDebugHUD;
    if      (IsKeyPressed(KEY_F3))          STATE->showDebugGrid = !STATE->showDebugGrid;

    if (!STATE->isEditorEnabled) return;


    Vector2 mousePosInScreen = GetMousePosition();

    if (!IsInPlayArea(mousePosInScreen)) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // TODO use a timer to not keep checking it every frame

        if (STATE->editorSelectedItem == 0) return;

        if (STATE->editorSelectedItem->handler == 0) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                TraceLog(LOG_WARNING, "No code to handle selected editor item.");
            return;
        }

        // so holding doesn't keep activating the item
        if (STATE->editorSelectedItem->interaction == Click &&
            !IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                return;


        STATE->editorSelectedItem->handler(
            PosInScreenToScene(mousePosInScreen));
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

    handleEditorInput();

    if (STATE->mode == InLevel) {
        handleInLevelInput();
        if (STATE->isPaused) return;
    }
    else if (STATE->mode == Overworld) {
        handleOverworldInput();
    }

    handleCameraInput(); // debug
}

void ClickOnEditorItem(EditorItem *item) {

    STATE->editorSelectedItem = item;
}

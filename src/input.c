#include <raylib.h>
#include <string.h>
#include <stdio.h>

#include "input.h"
#include "core.h"
#include "level/level.h"
#include "overworld.h"
#include "camera.h"
#include "persistence.h"
#include "render.h"


#define CAMERA_SPEED 8.0f;


void handleInLevelInput() {

    if      (IsKeyPressed(KEY_F5))          STATE->showBackground = !STATE->showBackground;

    if      (IsKeyPressed(KEY_BACKSPACE))   { OverworldInitialize(); return; }

    if      (IsKeyPressed(KEY_ENTER))       { PausedGameToggle(); return; }


    if (STATE->isPlayerDead) return;
    if (!LEVEL_PLAYER) return;


    if      (IsKeyDown(KEY_Z))              STATE->playerMovementSpeed = PLAYER_MOVEMENT_RUNNING;
    else                                    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    if      (IsKeyDown(KEY_RIGHT))          LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))           LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_LEFT);
    else                                    LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_STOP);

    if      (IsKeyPressed(KEY_X))           LevelPlayerJump();
}

void handleOverworldInput() {

    if      (IsKeyPressed(KEY_X))           { OverworldLevelSelect(); return; };

    if      (IsKeyPressed(KEY_UP))          OverworldCursorMove(OW_CURSOR_UP);
    else if (IsKeyPressed(KEY_DOWN))        OverworldCursorMove(OW_CURSOR_DOWN);
    else if (IsKeyPressed(KEY_LEFT))        OverworldCursorMove(OW_CURSOR_LEFT);
    else if (IsKeyPressed(KEY_RIGHT))       OverworldCursorMove(OW_CURSOR_RIGHT);
}

void handleEditorInput() {

    if      (IsKeyPressed(KEY_F1))           { EditorEnabledToggle(); return; }

    // Debug
    if      (IsKeyPressed(KEY_F2))          STATE->showDebugHUD = !STATE->showDebugHUD;
    if      (IsKeyPressed(KEY_F3))          STATE->showDebugGrid = !STATE->showDebugGrid;

    if (!STATE->isEditorEnabled) return;


    Vector2 mousePosInScreen = GetMousePosition();

    if (!IsInPlayArea(mousePosInScreen)) return;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // TODO use a timer to not keep checking it every frame

        if (STATE->editorSelectedEntity == 0) return;

        if (STATE->editorSelectedEntity->handler == 0) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                TraceLog(LOG_WARNING, "No code to handle selected editor entity.");
            return;
        }

        // so holding doesn't keep activating the item
        if (STATE->editorSelectedEntity->interaction == EDITOR_INTERACTION_CLICK &&
            !IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                return;


        STATE->editorSelectedEntity->handler(
            PosInScreenToScene(mousePosInScreen));
    }
}

void handleCameraInput() {

    // TODO move camera code to camera.c
    if (IsKeyDown(KEY_A)) CAMERA->pos.x -= CAMERA_SPEED;
    if (IsKeyDown(KEY_D)) CAMERA->pos.x += CAMERA_SPEED;
    if (IsKeyDown(KEY_W)) CAMERA->pos.y -= CAMERA_SPEED;
    if (IsKeyDown(KEY_S)) CAMERA->pos.y += CAMERA_SPEED;
}

void handleDroppedFile() {

    char *levelName = MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);
    
    if (PersistenceGetDroppedLevelName(levelName)) {
        
        LevelInitialize(levelName);
        
        if (STATE->expectingLevelAssociation) {

            strcpy(STATE->tileUnderCursor->levelName, levelName);

            TraceLog(LOG_INFO, "Dot on x=%.1f, y=%.1f associated with level %s.",
                        STATE->tileUnderCursor->gridPos.x, STATE->tileUnderCursor->gridPos.y, levelName);
            
            char *sysMsg = MemAlloc(sizeof(char) * SYS_MSG_BUFFER_SIZE);
            sprintf(sysMsg, "Associada fase %s", levelName);
            RenderPrintSysMessage(sysMsg);
            MemFree(sysMsg);

            STATE->expectingLevelAssociation = false;
        }
    }

    MemFree(levelName);
}

void InputHandle() {

    handleEditorInput();

    if (IsFileDropped()) {
        handleDroppedFile();
        return;
    }

    if (STATE->mode == MODE_IN_LEVEL) {
        handleInLevelInput();
        if (STATE->isPaused) return;
    }
    else if (STATE->mode == MODE_OVERWORLD) {
        handleOverworldInput();
    }

    handleCameraInput(); // debug
}

void InputEditorEntitySelect(EditorEntityItem *item) {

    STATE->editorSelectedEntity = item;
}

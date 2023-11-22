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


void handleInLevelInput() {

    if      (IsKeyPressed(KEY_F5))          STATE->showBackground = !STATE->showBackground;


    if (STATE->isEditorEnabled) return;


    if      (IsKeyPressed(KEY_BACKSPACE))   { LevelGoToOverworld(); return; }

    if      (IsKeyPressed(KEY_ENTER))       { PausedGameToggle(); return; }


    if (STATE->isPaused || !LEVEL_PLAYER || LEVEL_PLAYER->isDead) return;


    if      (IsKeyDown(KEY_Z))              LevelPlayerStartRunning();
    else                                    LevelPlayerStopRunning();

    if      (IsKeyDown(KEY_RIGHT))          LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))           LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_LEFT);
    else                                    LevelPlayerMoveHorizontal(PLAYER_MOVEMENT_STOP);

    if      (IsKeyPressed(KEY_X))           LevelPlayerJump();

    // For debugging
    if      (IsKeyPressed(KEY_R))           LevelPlayerSetRespawn();
    if      (IsKeyPressed(KEY_ONE))         LevelPlayerSetMode(PLAYER_MODE_DEFAULT);
    if      (IsKeyPressed(KEY_TWO))         LevelPlayerSetMode(PLAYER_MODE_GLIDE);
}

void handleOverworldInput() {

    if (STATE->isEditorEnabled) return;
    

    if      (IsKeyPressed(KEY_X))           { OverworldLevelSelect(); return; };

    if      (IsKeyPressed(KEY_UP))          OverworldCursorMove(OW_CURSOR_UP);
    else if (IsKeyPressed(KEY_DOWN))        OverworldCursorMove(OW_CURSOR_DOWN);
    else if (IsKeyPressed(KEY_LEFT))        OverworldCursorMove(OW_CURSOR_LEFT);
    else if (IsKeyPressed(KEY_RIGHT))       OverworldCursorMove(OW_CURSOR_RIGHT);
}

void handleDevInput() {

    if      (IsKeyPressed(KEY_F1))          { EditorEnabledToggle(); return; }
    if      (IsKeyPressed(KEY_F2))          DebugHudToggle();
    if      (IsKeyPressed(KEY_F3))          STATE->showDebugGrid = !STATE->showDebugGrid;


    /* Mouse functionalities */

    if (IsCursorHidden()) return;

    Vector2 mousePosInScreen = GetMousePosition();
    Vector2 mousePosInScene = PosInScreenToScene(mousePosInScreen);

    if      (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))         CameraPanningMove(mousePosInScreen);
    if      (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))     CameraPanningStop();
    if      (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))        CameraPanningReset();


    // Entitiy selection
    if (STATE->isEditorEnabled && IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        EditorSelectEntities(mousePosInScene);
        return;
    }
    else if (STATE->isEditorEnabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && EDITOR_ENTITY_SELECTION) {

        // Actions available when entities are selected

        if (STATE->editorButtonToggled &&
            STATE->editorButtonToggled->type == EDITOR_ENTITY_ERASER &&
            IsInPlayArea(mousePosInScreen))
                    goto skip_to_button_handler;

        EditorSelectionCancel();
    }


    if      (!IsInPlayArea(mousePosInScreen)) return;


    if (STATE->showDebugHUD || STATE->isEditorEnabled) {

        if (IsKeyDown(KEY_SPACE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && LEVEL_PLAYER) {
            LevelPlayerCheckAndSetPos(mousePosInScene);
            return;
        }
    }

    if (STATE->showDebugHUD) {

        if  (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            RenderDebugEntityToggle(LevelEntityGetAt(mousePosInScene));
            return;
        }
    }

    if (STATE->isEditorEnabled) {

        if (IsKeyDown(KEY_O) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && LEVEL_PLAYER) {
            LevelPlayerCheckAndSetOrigin(mousePosInScene);
            return;
        }

skip_to_button_handler:
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

            // TODO use a timer to not keep checking it every frame

            if (!STATE->editorButtonToggled) return;

            if (!STATE->editorButtonToggled->handler) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    TraceLog(LOG_WARNING, "No code to handle selected editor entity.");
                return;
            }

            // so holding doesn't keep activating the item
            if (STATE->editorButtonToggled->interactionType == EDITOR_INTERACTION_CLICK &&
                !IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    return;


            STATE->editorButtonToggled->handler(mousePosInScene);
            return;
        }
    }
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

    handleDevInput();

    if (IsFileDropped()) {
        handleDroppedFile();
        return;
    }

    if (STATE->mode == MODE_IN_LEVEL) {
        handleInLevelInput();
    }
    else if (STATE->mode == MODE_OVERWORLD) {
        handleOverworldInput();
    }
}

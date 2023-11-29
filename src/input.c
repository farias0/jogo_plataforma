#include <raylib.h>
#include <string.h>
#include <stdio.h>

#include "input.h"
#include "core.h"
#include "level/level.h"
#include "level/player.h"
#include "overworld.h"
#include "camera.h"
#include "persistence.h"
#include "render.h"
#include "editor.h"


void handleInLevelInput() {

    if      (IsKeyPressed(KEY_F5))          GAME_STATE->showBackground = !GAME_STATE->showBackground;


    if (EDITOR_STATE->isEnabled) return;


    if      (IsKeyPressed(KEY_BACKSPACE))   { LevelGoToOverworld(); return; }

    if      (IsKeyPressed(KEY_ENTER))       { LevelPauseToggle(); return; }


    if (LEVEL_STATE->isPaused || !PLAYER_ENTITY || PLAYER_ENTITY->isDead) return;


    if      (IsKeyDown(KEY_Z))              PlayerStartRunning();
    else                                    PlayerStopRunning();

    if      (IsKeyDown(KEY_RIGHT))          PlayerMoveHorizontal(PLAYER_MOVEMENT_RIGHT);
    else if (IsKeyDown(KEY_LEFT))           PlayerMoveHorizontal(PLAYER_MOVEMENT_LEFT);
    else                                    PlayerMoveHorizontal(PLAYER_MOVEMENT_STOP);

    if      (IsKeyPressed(KEY_X))           PlayerJump();

    if      (IsKeyPressed(KEY_C))           PlayerSetCheckpoint();

    // For debugging
    if      (IsKeyPressed(KEY_ONE))         PlayerSetMode(PLAYER_MODE_DEFAULT);
    if      (IsKeyPressed(KEY_TWO))         PlayerSetMode(PLAYER_MODE_GLIDE);
}

void handleOverworldInput() {

    if (EDITOR_STATE->isEnabled) return;
    

    if      (IsKeyPressed(KEY_X))           { OverworldLevelSelect(); return; };

    if      (IsKeyPressed(KEY_UP))          OverworldCursorMove(OW_CURSOR_UP);
    else if (IsKeyPressed(KEY_DOWN))        OverworldCursorMove(OW_CURSOR_DOWN);
    else if (IsKeyPressed(KEY_LEFT))        OverworldCursorMove(OW_CURSOR_LEFT);
    else if (IsKeyPressed(KEY_RIGHT))       OverworldCursorMove(OW_CURSOR_RIGHT);
}

void handleDevInput() {

    if      (IsKeyPressed(KEY_F1))          { EditorEnabledToggle(); return; }
    if      (IsKeyPressed(KEY_F2))          DebugHudToggle();
    if      (IsKeyPressed(KEY_F3))          GAME_STATE->showDebugGrid = !GAME_STATE->showDebugGrid;


    /* Mouse functionalities */

    if (IsCursorHidden()) return;

    Vector2 mousePosInScreen = GetMousePosition();
    Vector2 mousePosInScene = PosInScreenToScene(mousePosInScreen);

    if      (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))         CameraPanningMove(mousePosInScreen);
    if      (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))     CameraPanningStop();
    if      (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))        CameraPanningReset();


    // Entitiy selection
    if (EDITOR_STATE->isEnabled && IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        EditorSelectEntities(mousePosInScene);
        return;
    }
    else if (EDITOR_STATE->isEnabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && EDITOR_STATE->selectedEntities) {

        if (!IsInPlayArea(mousePosInScreen)) goto skip_selected_entities_actions;

        // Actions available when entities are selected

        if (EDITOR_STATE->toggledEntityButton &&
            EDITOR_STATE->toggledEntityButton->type == EDITOR_ENTITY_ERASER)
                    goto skip_to_button_handler;

        if (EditorSelectedEntitiesMove(mousePosInScene)) return;

        EditorSelectionCancel();
    }
skip_selected_entities_actions:


    if      (!IsInPlayArea(mousePosInScreen)) return;


    if (GAME_STATE->showDebugHUD || EDITOR_STATE->isEnabled) {

        if (IsKeyDown(KEY_SPACE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlayerCheckAndSetPos(mousePosInScene);
            return;
        }
    }

    if (GAME_STATE->showDebugHUD) {

        if  (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            RenderDebugEntityToggle(LevelEntityGetAt(mousePosInScene));
            return;
        }
    }

    if (EDITOR_STATE->isEnabled) {

        if (IsKeyDown(KEY_O) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlayerCheckAndSetOrigin(mousePosInScene);
            return;
        }

skip_to_button_handler:
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

            // TODO use a timer to not keep checking it every frame

            if (!EDITOR_STATE->toggledEntityButton) return;

            if (!EDITOR_STATE->toggledEntityButton->handler) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    TraceLog(LOG_WARNING, "No code to handle selected editor entity.");
                return;
            }

            // so holding doesn't keep activating the item
            if (EDITOR_STATE->toggledEntityButton->interactionType == EDITOR_INTERACTION_CLICK &&
                !IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    return;


            EDITOR_STATE->toggledEntityButton->handler(mousePosInScene);
            return;
        }
    }
}

void handleDroppedFile() {

    char *levelName = MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);
    
    if (PersistenceGetDroppedLevelName(levelName)) {
        
        LevelLoad(levelName);
        
        if (LEVEL_STATE->awaitingAssociation) {

            strcpy(OW_STATE->tileUnderCursor->levelName, levelName);

            TraceLog(LOG_INFO, "Dot on x=%.1f, y=%.1f associated with level %s.",
                        OW_STATE->tileUnderCursor->gridPos.x, OW_STATE->tileUnderCursor->gridPos.y, levelName);
            
            char *sysMsg = MemAlloc(sizeof(char) * SYS_MSG_BUFFER_SIZE);
            sprintf(sysMsg, "Associada fase %s", levelName);
            RenderPrintSysMessage(sysMsg);
            MemFree(sysMsg);
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

    if (GAME_STATE->mode == MODE_IN_LEVEL) {
        handleInLevelInput();
    }
    else if (GAME_STATE->mode == MODE_OVERWORLD) {
        handleOverworldInput();
    }
}

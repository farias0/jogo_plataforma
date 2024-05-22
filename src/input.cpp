#include <raylib.h>
#include <string.h>
#include <stdio.h>

#include "input.hpp"
#include "core.hpp"
#include "level/level.hpp"
#include "level/player.hpp"
#include "overworld.hpp"
#include "camera.hpp"
#include "persistence.hpp"
#include "render.hpp"
#include "editor.hpp"
#include "debug.hpp"
#include "menu.hpp"


namespace Input {

InputState STATE;


bool isGamepadPressed(int button) {
    return IsGamepadButtonPressed(GAME_STATE->gamepadIdx, button);
}

bool isGamepadDown(int button) {
    return IsGamepadButtonDown(GAME_STATE->gamepadIdx, button);
}

bool isGamepadAnalogDown(AnalogStickDigitalDirection dir) {
    return (STATE.leftStickCurrentState & dir) == dir;
}

bool isGamePadAnalogPressed(AnalogStickDigitalDirection dir) {
    return isGamepadAnalogDown(dir) && (STATE.leftStickPreviousState & dir) != dir;
}

void updateLeftStickState() {

    STATE.leftStickPreviousState = STATE.leftStickCurrentState;

    char horizontal = 0, vertical = 0;
    float reading;

    reading = GetGamepadAxisMovement(GAME_STATE->gamepadIdx, 0);
    if (reading > ANALOG_STICK_DIGITAL_THRESHOLD) horizontal = ANALOG_RIGHT;
    else if (reading < ANALOG_STICK_DIGITAL_THRESHOLD * -1) horizontal = ANALOG_LEFT;

    reading = GetGamepadAxisMovement(GAME_STATE->gamepadIdx, 1);
    if (reading > ANALOG_STICK_DIGITAL_THRESHOLD) vertical = ANALOG_DOWN;
    else if (reading < ANALOG_STICK_DIGITAL_THRESHOLD * -1) vertical = ANALOG_UP;

    STATE.leftStickCurrentState = horizontal | vertical; 
}

void updateInputStates() {

    updateLeftStickState();
}

void handleMenuInput() {

    Menu *menu = GAME_STATE->menu;
    if (!menu) return;

    if (IsKeyPressed(KEY_DOWN) || isGamepadPressed(GP_DOWN) || isGamePadAnalogPressed(ANALOG_DOWN))
        { menu->Down(); return; }

    if (IsKeyPressed(KEY_UP) || isGamepadPressed(GP_UP) || isGamePadAnalogPressed(ANALOG_UP))
        { menu->Up(); return; }

    if (IsKeyPressed(KEY_X) || isGamepadPressed(GP_A))
        { menu->Select(); return; }

}

void handleInLevelInput() {

    if (IsKeyPressed(KEY_F7))
        GAME_STATE->showBackground = !GAME_STATE->showBackground;


    if (EDITOR_STATE->isEnabled) return;


    if (IsKeyPressed(KEY_BACKSPACE) || isGamepadPressed(GP_SELECT))
        { Level::GoToOverworld(); return; }


    if (IsKeyPressed(KEY_ENTER) || isGamepadPressed(GP_START))
        { Level::PauseToggle(); return; }


    if (Level::STATE->isPaused || !PLAYER || PLAYER->isDead) return;


    STATE.isHoldingRun = IsKeyDown(KEY_Z) || isGamepadDown(GP_X);


    if (IsKeyDown(KEY_RIGHT) || isGamepadDown(GP_RIGHT) || isGamepadAnalogDown(ANALOG_RIGHT))
        STATE.playerMoveDirection = PLAYER_DIRECTION_RIGHT; 

    else if (IsKeyDown(KEY_LEFT) || isGamepadDown(GP_LEFT) || isGamepadAnalogDown(ANALOG_LEFT))
        STATE.playerMoveDirection = PLAYER_DIRECTION_LEFT;

    else
        STATE.playerMoveDirection = PLAYER_DIRECTION_STOP;


    if (IsKeyPressed(KEY_X) || isGamepadPressed(GP_A))
        PLAYER->Jump();

    if (IsKeyPressed(KEY_C) || isGamepadPressed(GP_Y))
        PLAYER->SetCheckpoint();

    if (IsKeyPressed(KEY_A) || isGamepadPressed(GP_R1))
        PLAYER->LaunchGrapplingHook();


    // For debugging
    if (IsKeyPressed(KEY_ONE))
        PLAYER->SetMode(PLAYER_MODE_DEFAULT);

    if (IsKeyPressed(KEY_TWO))
        PLAYER->SetMode(PLAYER_MODE_GLIDE);
}

void handleOverworldInput() {

    if (EDITOR_STATE->isEnabled) return;


    if (isGamepadPressed(GP_SELECT))
        GameExit();


    if (IsKeyPressed(KEY_X) || isGamepadPressed(GP_A))
        { OverworldLevelSelect(); return; };


    if (IsKeyPressed(KEY_UP) || isGamepadPressed(GP_UP) || isGamePadAnalogPressed(ANALOG_UP))
        OverworldCursorMove(OW_CURSOR_UP);

    else if (IsKeyPressed(KEY_DOWN) || isGamepadPressed(GP_DOWN) || isGamePadAnalogPressed(ANALOG_DOWN))
        OverworldCursorMove(OW_CURSOR_DOWN);

    else if (IsKeyPressed(KEY_LEFT) || isGamepadPressed(GP_LEFT) || isGamePadAnalogPressed(ANALOG_LEFT))
        OverworldCursorMove(OW_CURSOR_LEFT);
        
    else if (IsKeyPressed(KEY_RIGHT) || isGamepadPressed(GP_RIGHT) || isGamePadAnalogPressed(ANALOG_RIGHT))
        OverworldCursorMove(OW_CURSOR_RIGHT);
}

void handleDevInput() {

    if      (IsKeyPressed(KEY_F1))          { EditorEnabledToggle(); return; }
    if      (IsKeyPressed(KEY_F2))          DebugHudToggle();
    if      (IsKeyPressed(KEY_F3))          GAME_STATE->showDebugGrid = !GAME_STATE->showDebugGrid;
    if      (IsKeyPressed(KEY_F5))          AssetsHotReload();
    if      (IsKeyPressed(KEY_F11))         Render::FullscreenToggle();


    /* Mouse functionalities */

    if (IsCursorHidden()) return;

    Vector2 mousePosInScreen = GetMousePosition();
    Vector2 mousePosInScene = PosInScreenToScene(mousePosInScreen);

    if      (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))         CameraPanningMove(mousePosInScreen);
    if      (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))     CameraPanningStop();
    if      (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))        CameraPanningReset();

    // Zoom
    if      (GetMouseWheelMove() > 0) CameraZoomIn();
    else if (GetMouseWheelMove() < 0) CameraZoomOut();

    // Entitiy selection
    if (EDITOR_STATE->isEnabled && IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        EditorSelectEntities(mousePosInScene);
        return;
    }
    else if (EDITOR_STATE->isEnabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !EDITOR_STATE->selectedEntities.empty()) {

        if (!IsInMouseArea(mousePosInScreen)) goto skip_selected_entities_actions;

        // Actions available when entities are selected

        if (EDITOR_STATE->toggledEntityButton &&
            EDITOR_STATE->toggledEntityButton->type == EDITOR_ENTITY_ERASER)
                    goto skip_to_button_handler;

        if (EditorSelectedEntitiesMove(mousePosInScene)) return;

        EditorSelectionCancel();
    }
skip_selected_entities_actions:


    if      (!IsInMouseArea(mousePosInScreen)) return;


    if (GAME_STATE->showDebugHUD || EDITOR_STATE->isEnabled) {

        if (IsKeyDown(KEY_SPACE) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PLAYER->CheckAndSetPos(mousePosInScene);
            return;
        }
    }

    if (GAME_STATE->showDebugHUD) {

        if  (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            DebugEntityToggle(mousePosInScene);
            return;
        }
    }

    if (EDITOR_STATE->isEnabled) {

        if (IsKeyDown(KEY_O) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PLAYER->CheckAndSetOrigin(mousePosInScene);
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


            int tags = 0;
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) tags += EDITOR_INTERACTION_CLICK;
            else tags += EDITOR_INTERACTION_HOLD;

            if (IsKeyDown(KEY_LEFT_ALT)) tags += EDITOR_INTERACTION_ALT;


            EDITOR_STATE->toggledEntityButton->handler(mousePosInScene, tags);
            return;
        }
    }

}

void handleDroppedFile() {

    char *levelName = (char *) MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);
    
    if (PersistenceGetDroppedLevelName(levelName)) {
        
        Level::Load(levelName);
        
        if (Level::STATE->awaitingAssociation) {

            strcpy(OW_STATE->tileUnderCursor->levelName, levelName);

            TraceLog(LOG_INFO, "Dot on x=%.1f, y=%.1f associated with level %s.",
                        OW_STATE->tileUnderCursor->gridPos.x, OW_STATE->tileUnderCursor->gridPos.y, levelName);
            
            Render::PrintSysMessage("Associada fase " + std::string(levelName));
        }
    }

    MemFree(levelName);
}

void handleTextInput() {
  
    int keyPressed = -1;
    while (keyPressed) {

        keyPressed = GetKeyPressed();

        if (keyPressed >= ' ' && keyPressed <= '~') {

            STATE.textInputed += (char) keyPressed;

        }
        else if (IsKeyDown(KEY_BACKSPACE) && !STATE.textInputed.empty()) {
            
            STATE.textInputed.pop_back();

        }
        else if (keyPressed == KEY_ENTER) {

            // Finishes text input
            STATE.textInputCallback->operator()(STATE.textInputed);

            TraceLog(LOG_TRACE, "Text input finished: %s.", STATE.textInputed.c_str());

            GAME_STATE->waitingForTextInput = false;
            STATE.textInputed.clear();

            return;

        }
    }
}

void Initialize() {

    STATE = InputState();

    // For some reason IsGamepadAvailable only works from the second frame onwards,
    // I have no idea why, and I didn't feel like debugging it
    TraceLog(LOG_INFO, "Gamepad 0 name: %s", GetGamepadName(0));

    TraceLog(LOG_INFO, "Input initialized.");
}

void Handle() {

    updateInputStates();

    if (GAME_STATE->waitingForTextInput) {
        handleTextInput();
        return;
    }

    handleDevInput();

    if (IsFileDropped()) {
        handleDroppedFile();
        return;
    }

    if (!EDITOR_STATE->isEnabled) {
        handleMenuInput();
    }

    if (GAME_STATE->mode == MODE_IN_LEVEL) {
        handleInLevelInput();
    }
    else if (GAME_STATE->mode == MODE_OVERWORLD) {
        handleOverworldInput();
    }
}

void GetTextInput(TextInputCallback *callback) {

    GAME_STATE->waitingForTextInput = true;
    
    STATE.textInputCallback = callback;

    TraceLog(LOG_TRACE, "Text input started.");
}

}

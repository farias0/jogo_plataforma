#ifndef _CORE_H_INCLUDED_
#define _CORE_H_INCLUDED_


#include "level/level.h"
#include "editor.h"
#include "linked_list.h"

// Currently this represents the play space (not including editor)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SCREEN_WIDTH_W_EDITOR   SCREEN_WIDTH + EDITOR_BAR_WIDTH

#define FLOOR_DEATH_HEIGHT 800 // Below this y in level the player dies


typedef enum GameMode {
    MODE_IN_LEVEL,
    MODE_OVERWORLD
} GameMode;

typedef struct GameState {
    GameMode mode;

    bool isPaused;
    bool isPlayerDead;

    PlayerMovementSpeed playerMovementSpeed;

    EditorItem *editorSelectedItem;
    bool isEditorEnabled;

    bool showDebugHUD;
    bool showDebugGrid;
    bool showBackground;
} GameState;


extern GameState *STATE;


void GameStateInitialize();

void GameStateReset();

// Toggles between level and overworld
void GameModeToggle();

// Toggles between paused and unpaused game
void StateInLevelToggle();

// Enables and disables editor
void EditorEnabledToggle();

// True if the given position is inside the game screen (doesn't include the editor)
bool IsInPlayArea(Vector2 pos);

// Returns the entity list's head for the current selected game mode
ListNode *GetEntityListHead();

// Snaps a coordinate (x or y) to a grid.
float SnapToGrid(float value, float length);

// Pushes a coordinate (x or y) according to a grid.
float PushOnGrid(float value, float length);


#endif // _CORE_H_INCLUDED_

#ifndef _GLOBAL_H_INCLUDED_
#define _GLOBAL_H_INCLUDED_


#include "inlevel/player.h"
#include "editor.h"
#include "linked_list.h"

// Currently this represents the play space (not including editor)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define EDITOR_BAR_WIDTH        200
#define SCREEN_WIDTH_W_EDITOR   SCREEN_WIDTH + EDITOR_BAR_WIDTH

#define FLOOR_HEIGHT 600
#define FLOOR_DEATH_HEIGHT 800 // Below this y the player dies


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


void InitializeGameState();

void ResetGameState();

// Toggles between level and overworld
void ToggleGameMode();

// Toggles between paused and unpaused game
void ToggleInLevelState();

// True if the given position is inside the play space on the screen (doesn't include the editor)
bool IsInPlayArea(Vector2 pos);

// Enables and disables editor
void ToggleEditorEnabled();

// Returns the entity list's head for the current selected game mode
ListNode *GetListHead();

// Converts position from the screen coordinates to in game coordinates
Vector2 PosInScreenToScene(Vector2 pos);

// Converts position from in game coordinates to the screen coordinates
Vector2 PosInSceneToScreen(Vector2 pos);

// Snaps a coordinate (x or y) to a grid.
float SnapToGrid(float value, float length);

// Pushes a coordinate (x or y) according to a grid.
float PushOnGrid(float value, float length);


#endif // _GLOBAL_H_INCLUDED_

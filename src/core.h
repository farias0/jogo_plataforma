#ifndef _CORE_H_INCLUDED_
#define _CORE_H_INCLUDED_

#include <raylib.h>
#include "linked_list.h"
#include "persistence.h"

// Currently this represents the play space (not including editor)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SCREEN_WIDTH_W_EDITOR   SCREEN_WIDTH + EDITOR_BAR_WIDTH

#define FLOOR_DEATH_HEIGHT 800 // Below this y in level the player dies


struct EditorEntityButton;
struct OverworldEntity;

typedef struct Dimensions {
    float width;
    float height;
} Dimensions;

typedef struct Trajectory {
    Vector2 start;
    Vector2 end;
} Trajectory;

typedef enum GameMode {
    MODE_IN_LEVEL,
    MODE_OVERWORLD
} GameMode;

typedef struct GameState {

    GameMode mode;

    char loadedLevel[LEVEL_NAME_BUFFER_SIZE];

    bool isPaused;

    // What entity button is currently selected in the editor
    struct EditorEntityButton *editorButtonToggled;
    bool isEditorEnabled;

    struct OverworldEntity *tileUnderCursor;
    
    // If selected a dot in the OW without an associated level
    bool expectingLevelAssociation;

    bool showDebugHUD;
    bool showDebugGrid;
    bool showDebugEntity;
    bool showBackground;
} GameState;


extern GameState *STATE;


void GameStateInitialize();

void GameStateReset();

// Initialize the game's core systems
void SystemsInitialize();

// Updates the logic of the game. To be called once every frame.
void GameUpdate();

// Toggles between paused and unpaused game
void PausedGameToggle();

// True if the given position is inside the game screen (doesn't include the editor)
bool IsInPlayArea(Vector2 pos);

// Returns the entity list's head for the current selected game mode
ListNode *GetEntityListHead();

// Enables the debug HUD
void DebugHudEnable();

// Disables the debug HUD
void DebugHudDisable();

// Disables mouse cursor, if not in use
void MouseCursorDisable();

// Enables the mouse cursor
void MouseCursorEnable();

// Toggles the debug HUD between 'enabled' and 'disabled'
void DebugHudToggle();

// Snaps a point to its place (top-left) in a grid.
Vector2 SnapToGrid(Vector2 coords, Dimensions grid);

// The distance between a point and the closest (bottom-right) point of a grid.
Vector2 DistanceFromGrid(Vector2 coords, Dimensions grid);


#endif // _CORE_H_INCLUDED_

#ifndef _CORE_H_INCLUDED_
#define _CORE_H_INCLUDED_

#include <raylib.h>
#include "linked_list.h"


// Currently this represents the play space (not including editor)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SCREEN_WIDTH_W_EDITOR   SCREEN_WIDTH + EDITOR_PANEL_RECT.width


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

    bool showDebugHUD;
    bool showDebugGrid;
    bool showBackground;
} GameState;


extern GameState *GAME_STATE;


void GameStateInitialize();

void GameStateReset();

// Initialize the game's core systems
void SystemsInitialize();

// Updates the logic of the game. To be called once every frame.
void GameUpdate();

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

// Returns the x and y of a rectangle
Vector2 RectangleGetPos(Rectangle rect);

// Sets the x and y of a rectangle according to a Vector
void RectangleSetPos(Rectangle *rect, Vector2 pos);

// Sets the width and height of a rectangle according to a Dimensions
void RectangleSetDimensions(Rectangle *rect, Dimensions dims);


#endif // _CORE_H_INCLUDED_

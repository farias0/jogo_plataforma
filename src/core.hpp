#ifndef _CORE_H_INCLUDED_
#define _CORE_H_INCLUDED_

#include <raylib.h>

#include "linked_list.hpp"


// Represents the play area and the initial resolution
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


class Menu;

class Dimensions {
public:
    float width;
    float height;

    Dimensions() {}

    Dimensions(float width, float height):
        width(width), height(height) {}

};

// An OOP version of raylib's Rectangle
class Rect {
public:
    float x;
    float y;
    float width;
    float height;

    Rect() {}

    Rect(float x, float y, float width, float height):
        x(x), y(y), width(width), height(height) {}

    operator Rectangle() {
        return { x, y, width, height };
    }
};

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

    // The game is waiting for the user to input a text.
    // The whole game freezes during this time.
    bool waitingForTextInput;

    bool showDebugHUD;
    bool showDebugGrid;
    bool showBackground;

    bool showDevTextbox;

    int gamepadIdx = 0;

    // The opened menu, if it exists;
    Menu *menu;

    int coinsCollected;

} GameState;


extern GameState *GAME_STATE;


void GameStateInitialize();

void GameStateReset();

// Initialize the game's core systems
void SystemsInitialize();

// Updates the logic of the game. To be called once every frame.
void GameUpdate();

void GameExit();

// The area of the screen that's interactable
bool IsInMouseArea(Vector2 pos);

// Returns the entity list's head for the current selected game mode
LinkedList::Node *GetEntityListHead();

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

void ToggleDevTextbox();

bool IsDevTextboxEnabled();

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

void ExitGame();


#endif // _CORE_H_INCLUDED_

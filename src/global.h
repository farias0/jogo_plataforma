#ifndef _GLOBAL_H_INCLUDED_
#define _GLOBAL_H_INCLUDED_


#include "entities/entity.h"
#include "entities/player.h"
#include "editor.h"

// Currently this represents the play space (not including editor)
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define EDITOR_BAR_WIDTH    200
#define SCREEN_WIDTH_FULL   SCREEN_WIDTH + EDITOR_BAR_WIDTH // TODO rename this constant

#define FLOOR_HEIGHT 600
#define FLOOR_DEATH_HEIGHT 800 // Below this y the player dies


typedef enum GameMode {
    InLevel,
    Overworld
} GameMode;

typedef struct GameState {
    GameMode mode;

    bool isPaused;
    bool isPlayerDead;

    PlayerMovementSpeed playerMovementSpeed;

    EditorItem *editorSelectedItem;
} GameState;


extern GameState *STATE;

// The head of the linked list of all the entities
extern Entity *ENTITIES_HEAD;

// The entity "player" in the linked list of all the entities
extern Entity *PLAYER;

// The entity "camera" in the linked list of all the entities
extern Entity *CAMERA;


void InitializeGameState();

void ResetGameState();

// Player resumes playing after dying. // TODO rename this function
void PlayerContinue();

// Goes to level
void InitializeLevel();

// Goes to overworld
void InitializeOverworld();

// Toggles between level and overworld
void ToggleGameMode();

// True if the given position is inside the play space on the screen (doesn't include the editor)
bool IsInPlayArea(Vector2 pos);


#endif // _GLOBAL_H_INCLUDED_

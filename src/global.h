#ifndef _GLOBAL_H_INCLUDED_
#define _GLOBAL_H_INCLUDED_


#include "entities/entity.h"
#include "entities/player.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define FLOOR_HEIGHT 600
#define FLOOR_DEATH_HEIGHT 800 // Below this y the player dies


typedef struct GameState {
    bool isPaused;
    bool isPlayerDead;

    PlayerMovementSpeed playerMovementSpeed;

    // debug
    int gamepadIdx;
} GameState;


extern GameState *STATE;

// An unspecified node in the linked list of all the entities
extern Entity *ENTITIES;

// The entity "player" in the linked list of all the entities
extern Entity *PLAYER;

// The entity "camera" in the linked list of all the entities
extern Entity *CAMERA;


void ResetGameState();

#endif // _GLOBAL_H_INCLUDED_
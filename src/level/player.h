#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

#include "level.h"


typedef enum PlayerMovementSpeed {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementSpeed;

typedef enum PlayerHorizontalMovementType {
    PLAYER_MOVEMENT_STOP,
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerHorizontalMovementType;

typedef enum PlayerMode {
    PLAYER_MODE_DEFAULT,
    PLAYER_MODE_GLIDE
} PlayerMode;

typedef struct PlayerState {

    // The ground beneath the player, updated every frame, or 0 if there's no ground beneath
    LevelEntity *groundBeneath;

    // TODO maybe add multiple hitboxes support for every entity
    Rectangle upperbody, lowerbody;

    // If the player is on the "going up" phase of the jump
    bool isAscending;

    // If the player is on mode 'GLIDE' and is actively gliding
    bool isGliding;

    float yVelocity;
    float yVelocityTarget;
    float xVelocity;

    PlayerMovementSpeed speed;

    PlayerMode mode;

    LevelEntity *checkpoint;
    int checkpointsLeft;

    // timestamps, for jump buffers
    double lastPressedJump;
    double lastGroundBeneath;
} PlayerState;


// Reference to the player's LevelEntity, part of the level entity list
extern LevelEntity *PLAYER_ENTITY; 

extern PlayerState *PLAYER_STATE;


// Initializes and adds the player to the level in the given origin
void PlayerInitialize(Vector2 origin);

// Sets the player's origin based on pos, if there aren't other things there already.
void PlayerCheckAndSetOrigin(Vector2 pos);

// Moves the player to pos, if there aren't other things there already.
void PlayerCheckAndSetPos(Vector2 pos);

void PlayerSetMode(PlayerMode mode);

void PlayerMoveHorizontal(PlayerHorizontalMovementType direction);

void PlayerStartRunning();

void PlayerStopRunning();

void PlayerJump();

void PlayerTick();

// Continues the game after dying.
void PlayerContinue();

void PlayerSetCheckpoint();


#endif // _PLAYER_H_INCLUDED_

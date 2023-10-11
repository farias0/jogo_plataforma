#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

typedef enum PlayerMovementSpeed {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementSpeed;

typedef enum PlayerHorizontalMovementType {
    PLAYER_MOVEMENT_STOP,
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerHorizontalMovementType;

/*
    TODO: Currently only the player has more than one hitbox,
        but eventually every entity should support multiple hitboxes.
*/
extern Rectangle playersUpperbody, playersLowebody;

// Initializes a player and returns the list's head.
Entity *InitializePlayer(Entity *head, Entity **newPlayer);
void UpdatePlayerHorizontalMovement(PlayerHorizontalMovementType direction);
void PlayerStartJump(Entity *player);
void PlayerTick(Entity *player);


#endif // _PLAYER_H_INCLUDED_
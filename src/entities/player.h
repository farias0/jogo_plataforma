#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

typedef enum PlayerMovementType {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementType;

typedef enum PlayerMovementDirection {
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerMovementDirection;

/*
    TODO: Currently only the player has more than one hitbox,
        but eventually every entity should support multiple hitboxes.
*/
extern Rectangle playersUpperbody, playersLowebody;

Entity *InitializePlayer(Entity *listItem);
void MovePlayer(Entity *player, PlayerMovementType type, PlayerMovementDirection direction);
void PlayerStartJump(Entity *player);
void PlayerTick(Entity *player);

/*
    I didn't want to make this a part of the Entity struct, but it's not a good
    idea to be doing getters.
    TODO: Consider using a PlayerState.
*/
bool IsPlayerJumping();


#endif // _PLAYER_H_INCLUDED_
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
void DrawPlayer(Entity *player);

/*
    TODO Replace this with a PlayerState object accessible by the game
*/
void PlayerFallsDead(Entity *player); // Start "fell down into a pit" animation



#endif // _PLAYER_H_INCLUDED_
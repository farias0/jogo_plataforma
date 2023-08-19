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

Entity *InitializePlayer(Entity *listItem);
void MovePlayer(Entity *player, PlayerMovementType type, PlayerMovementDirection direction);
void PlayerStartJump(Entity *player);
void PlayerTick(Entity *player);
void DrawPlayer(Entity *player);


#endif // _PLAYER_H_INCLUDED_
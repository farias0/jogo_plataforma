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


extern LevelEntity *LEVEL_PLAYER; 


void LevelPlayerInitialize(Vector2 pos);
void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction);
void LevelPlayerJump();
void LevelPlayerTick();
// Continues the game after dying.
void LevelPlayerContinue();


#endif // _PLAYER_H_INCLUDED_

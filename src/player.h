#ifndef _PLAYER_H_INCLUDED_
#define _PLAYER_H_INCLUDED_


#include <raylib.h>

typedef enum PlayerMovementType {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementType;

typedef enum PlayerMovementDirection {
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerMovementDirection;

extern Rectangle playerHitbox;

void InitializePlayer();
void SetPlayerPosition(Vector2 pos);
void MovePlayer(PlayerMovementType type, PlayerMovementDirection direction);
void DrawPlayer();


#endif // _PLAYER_H_INCLUDED_
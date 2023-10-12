#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>
#include "entity.h"

Entity *LoadLevel(Entity *head);
Entity *AddBlockToLevel(Entity *head, Vector2 pos);
// Returns the player's starting position for the currently loaded level;
Vector2 GetPlayerStartingPosition();

#endif // _LEVEL_H_INCLUDED_
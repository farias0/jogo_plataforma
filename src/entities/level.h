#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>
#include "entity.h"

Entity *LoadLevel(Entity *head);

// Initializes and adds a block to the level in the given pos,
// and returns the entities list's head.
Entity *AddBlockToLevel(Vector2 pos);

// Returns the player's starting position for the currently loaded level
Vector2 GetPlayerStartingPosition();

#endif // _LEVEL_H_INCLUDED_
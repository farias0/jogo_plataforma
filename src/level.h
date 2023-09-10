#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>
#include "entity.h"

Entity *InitializeLevel(Entity *entitiesItem);
void AddBlockToLevel(Entity *entitiesItem, Vector2 pos);


#endif // _LEVEL_H_INCLUDED_
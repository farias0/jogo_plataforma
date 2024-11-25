#ifndef _POWERUPS_H_INCLUDED_
#define _POWERUPS_H_INCLUDED_

#include <raylib.h>

#include "level.hpp"

#define GLIDE_PICKUP_ENTITY_ID         "glide_pickup"


Level::Entity *GlideAddFromPersistence();

Level::Entity *GlideAdd(Vector2 origin);

void GlideAddFromEditor(Vector2 origin, int interactionTags);


#endif // _POWERUPS_H_INCLUDED_

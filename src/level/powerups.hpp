#ifndef _POWERUPS_H_INCLUDED_
#define _POWERUPS_H_INCLUDED_

#include <raylib.h>

#include "level.hpp"


Level::Entity *GlideAdd();

Level::Entity *GlideAdd(Vector2 origin);

void GlideCheckAndAdd(Vector2 origin);


#endif // _POWERUPS_H_INCLUDED_

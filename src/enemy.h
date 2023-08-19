#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "entity.h"


Entity *InitializeEnemy(Entity *listItem);
void EnemyTick(Entity *enemy, Entity *player);
void DrawEnemy(Entity *enemy);


#endif // _ENEMY_H_INCLUDED_
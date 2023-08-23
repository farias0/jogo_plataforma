#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

// X and Y above which the enemy will be spawn
Entity *InitializeEnemy(Entity *listItem, int x, int y);
void EnemyTick(Entity *enemy, Entity *player);
void DrawEnemy(Entity *enemy);


#endif // _ENEMY_H_INCLUDED_
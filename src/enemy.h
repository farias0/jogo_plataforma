#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

// X and Y above which the enemy will be spawn
Entity *InitializeEnemy(Entity *listItem, int x, int y);
void EnemyTick(Entity *enemy, Entity *player);
// InitializeEnemy wrapped. # TODO stardardize this operation
bool AddEnemyToLevel(Entity *listItem, Vector2 pos);

#endif // _ENEMY_H_INCLUDED_
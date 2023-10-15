#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "entity.h"

// X and Y above which the enemy will be spawn. Initializes an enemy and returns the list's head.
Entity *InitializeEnemy(Entity *head, int x, int y);

void EnemyTick(Entity *enemy, Entity *player);

// Initializes and adds an enemy to the level in the given pos,
// and returns the entities list's head.
Entity *AddEnemyToLevel(Vector2 pos);

#endif // _ENEMY_H_INCLUDED_
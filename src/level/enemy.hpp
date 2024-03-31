#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"


// Initializes and adds an enemy to the level in the given origin
void EnemyAdd(Vector2 origin);

// Initializes and adds an enemy to the level in the given origin,
// if there are not other elements there already
void EnemyCheckAndAdd(Vector2 origin);

// Runs the update routine of a given enemy
void EnemyTick(LinkedList::ListNode *enemyNode);

// Kills a given enemy
void EnemyKill(Level::Entity *entity);


#endif // _ENEMY_H_INCLUDED_

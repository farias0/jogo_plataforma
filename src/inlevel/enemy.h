#ifndef _ENEMY_H_INCLUDED_
#define _ENEMY_H_INCLUDED_


#include "raylib.h"

#include "../linked_list.h"


// X and Y above which the enemy will be spawn. Initializes an enemy and returns the list's head.
void LevelEnemyAdd(int x, int y);

// Initializes and adds an enemy to the level in the given pos,
// if there are not other elements there already.
void LevelEnemyCheckAndAdd(Vector2 pos);

void LevelEnemyTick(ListNode *enemyNode);


#endif // _ENEMY_H_INCLUDED_

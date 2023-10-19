#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>

#include "../linked_list.h"
#include "../assets.h"


typedef enum LevelEntityComponent {

    LEVEL_IS_PLAYER         = 1,
    LEVEL_IS_ENEMY          = 2,
    LEVEL_IS_SCENARIO       = 4,

} LevelEntityComponent;

typedef struct LevelEntity {

    unsigned long int components;
    Rectangle hitbox;

    Sprite sprite;
    int layer;

    bool isFacingRight;
    bool isFallingDown;
    
} LevelEntity;


// The head of the linked list of all the level entities
extern ListNode *LEVEL_LIST_HEAD;

extern const Dimensions LEVEL_GRID;


void LevelInitialize();

// Initializes and adds a block to the level in the given pos,
// if there are no other blocks there already.
void LevelBlockCheckAndAdd(Vector2 pos);

// Returns the player's starting position for the currently loaded level
Vector2 LevelGetPlayerStartingPosition();

// The ground beneath the entity, or 0 if not on the ground
LevelEntity *LevelGetGroundBeneath(LevelEntity *entity);

// Searches for level entity in position;
// removes it from the list and destroys it,
// if found and if allowed.
void LevelEntityRemoveAt(Vector2 pos);

void LevelTick();


#endif // _LEVEL_H_INCLUDED_

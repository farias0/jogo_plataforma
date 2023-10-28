#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>

#include "../linked_list.h"
#include "../assets.h"


#define LEVEL_GRID (Dimensions){ 32, 32 }


typedef enum PlayerMovementSpeed {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementSpeed;

typedef enum PlayerHorizontalMovementType {
    PLAYER_MOVEMENT_STOP,
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerHorizontalMovementType;

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

extern LevelEntity *LEVEL_PLAYER; 


void LevelInitialize();

// Returns the player's starting position for the currently loaded level
Vector2 LevelGetPlayerStartingPosition();

// The ground beneath the entity, or 0 if not on the ground
LevelEntity *LevelGetGroundBeneath(LevelEntity *entity);

// Searches for level entity in position;
// removes it from the list and destroys it,
// if found and if allowed.
void LevelEntityRemoveAt(Vector2 pos);

void LevelTick();

// Saves to file the current loaded level's data
void LevelSave();


void LevelPlayerInitialize(Vector2 pos);

void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction);

void LevelPlayerJump();

void LevelPlayerTick();

// Continues the game after dying.
void LevelPlayerContinue();

// TODO remove it and use entity origin instead
void LevelPlayerSetStartingPos(Vector2 pos);


// Initializes and adds an enemy to the level in the given pos
void LevelEnemyAdd(Vector2 pos);

// Initializes and adds an enemy to the level in the given pos,
// if there are not other elements there already.
void LevelEnemyCheckAndAdd(Vector2 pos);

void LevelEnemyTick(ListNode *enemyNode);


void LevelBlockAdd(Vector2 pos);

// Initializes and adds a block to the level in the given pos,
// if there are no other blocks there already.
void LevelBlockCheckAndAdd(Vector2 pos);


#endif // _LEVEL_H_INCLUDED_

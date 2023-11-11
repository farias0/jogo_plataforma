#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>

#include "../linked_list.h"
#include "../assets.h"


#define NEW_LEVEL_NAME "default_new_level.lvl"

#define LEVEL_GRID (Dimensions){ 32, 32 }


typedef enum LevelEntityComponent {
    LEVEL_IS_PLAYER         = 1,
    LEVEL_IS_ENEMY          = 2,
    LEVEL_IS_SCENARIO       = 4,
    LEVEL_IS_EXIT           = 8,
    LEVEL_IS_GROUND         = 16
} LevelEntityComponent;

typedef struct LevelEntity {

    unsigned long int components;
    Rectangle hitbox;

    Sprite sprite;
    int layer;

    bool isFacingRight;
    bool isFallingDown;
    
} LevelEntity;

typedef enum PlayerMovementSpeed {
    PLAYER_MOVEMENT_DEFAULT,
    PLAYER_MOVEMENT_RUNNING
} PlayerMovementSpeed;

typedef enum PlayerHorizontalMovementType {
    PLAYER_MOVEMENT_STOP,
    PLAYER_MOVEMENT_LEFT,
    PLAYER_MOVEMENT_RIGHT
} PlayerHorizontalMovementType;

typedef struct PlayerState {

    // The ground beneath the player, updated every frame, or 0 if there's no ground beneath
    LevelEntity* groundBeneath;

    // TODO maybe add multiple hitboxes support for every entity
    Rectangle upperbody, lowerbody;

    // If the player is on the ascension phase of the jump
    bool isJumping;

    float yVelocity;
    float yVelocityTarget;
    float xVelocity;

    PlayerMovementSpeed speed;

    bool isDead;

} PlayerState;


// The head of the linked list of all the level entities
extern ListNode *LEVEL_LIST_HEAD;

extern LevelEntity *LEVEL_PLAYER; 

extern PlayerState *LEVEL_PLAYER_STATE;

// How long ago, in seconds, the level concluded, or -1 if it's not concluded
extern double levelConcludedAgo;


void LevelInitialize(char *levelName);

// Starts "go to Overworld" routine
void LevelGoToOverworld();

void LevelExitAdd(Vector2 pos);

void LevelExitCheckAndAdd(Vector2 pos);

// Returns the player's starting position for the currently loaded level
Vector2 LevelGetPlayerStartingPosition();

// The ground beneath the entity, or 0 if not on the ground.
// Gives priority to the tile in the direction the player is looking at.
LevelEntity *LevelGetGroundBeneath(LevelEntity *entity);

// Searches for level entity in position;
// removes it from the list and destroys it,
// if found and if allowed.
void LevelEntityRemoveAt(Vector2 pos);

void LevelTick();

// Saves to file the current loaded level's data
void LevelSave();

// Loads a new, default level
void LevelLoadNew();


void LevelPlayerInitialize(Vector2 pos);

void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction);

void LevelPlayStartRunning();

void LevelPlayerStopRunning();

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

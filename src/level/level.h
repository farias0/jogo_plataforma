#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>

#include "../linked_list.h"
#include "../assets.h"
#include "../core.h"


#define NEW_LEVEL_NAME "default_new_level.lvl"

#define LEVEL_GRID (Dimensions){ 32, 32 }


typedef enum LevelEntityComponent {
    LEVEL_IS_PLAYER         = 1,
    LEVEL_IS_ENEMY          = 2,
    LEVEL_IS_SCENARIO       = 4,
    LEVEL_IS_EXIT           = 8,
    LEVEL_IS_GROUND         = 16,
    LEVEL_IS_DANGER         = 32,
    LEVEL_IS_GLIDE          = 64,
    LEVEL_IS_CHECKPOINT     = 128
} LevelEntityComponent;

typedef struct LevelEntity {

    unsigned long int components;
    Vector2 origin;
    Rectangle hitbox;

    Sprite sprite;
    int layer;

    bool isDead;

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

typedef enum PlayerMode {
    PLAYER_MODE_DEFAULT,
    PLAYER_MODE_GLIDE
} PlayerMode;

typedef struct PlayerState {

    // The ground beneath the player, updated every frame, or 0 if there's no ground beneath
    LevelEntity *groundBeneath;

    // TODO maybe add multiple hitboxes support for every entity
    Rectangle upperbody, lowerbody;

    // If the player is on the "going up" phase of the jump
    bool isAscending;

    // If the player is on mode 'GLIDE' and is actively gliding
    bool isGliding;

    float yVelocity;
    float yVelocityTarget;
    float xVelocity;

    PlayerMovementSpeed speed;

    PlayerMode mode;

    LevelEntity *checkpoint;
    int checkpointsLeft;

    // timestamps, for jump buffers
    double lastPressedJump;
    double lastGroundBeneath;
} PlayerState;


// The head of the linked list of all the level entities
extern ListNode *LEVEL_LIST_HEAD;

extern LevelEntity *LEVEL_PLAYER; 

extern PlayerState *PLAYER_STATE;

// How long ago, in seconds, the level concluded, or -1 if it's not concluded
extern double levelConcludedAgo;


void LevelInitialize(char *levelName);

// Starts "go to Overworld" routine
void LevelGoToOverworld();

void LevelExitAdd(Vector2 pos);

void LevelExitCheckAndAdd(Vector2 pos);

// The ground beneath the entity, or 0 if not on the ground.
// Gives priority to the tile in the direction the player is looking at.
LevelEntity *LevelGetGroundBeneath(LevelEntity *entity);

// The ground beneath a hitbox, or 0 if not on the ground.
LevelEntity *LevelGetGroundBeneathHitbox(Rectangle hitbox);

void LevelEntityDestroy(ListNode *node);

// Searches for level entity in position
LevelEntity *LevelEntityGetAt(Vector2 pos);

// Removes level entity in position from the
// list and destroys it, if found and if allowed.
void LevelEntityRemoveAt(Vector2 pos);

void LevelTick();

// Saves to file the current loaded level's data
void LevelSave();

// Loads a new, default level
void LevelLoadNew();

// Get this entity's origin hitbox, based on the current hitbox.
Rectangle LevelEntityOriginHitbox(LevelEntity *entity);


void LevelPlayerInitialize(Vector2 origin);

// Sets the player's origin based on pos, if there aren't other things there already.
void LevelPlayerCheckAndSetOrigin(Vector2 pos);

// Moves the player to pos, if there aren't other things there already.
void LevelPlayerCheckAndSetPos(Vector2 pos);

void LevelPlayerSetMode(PlayerMode mode);

void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction);

void LevelPlayerStartRunning();

void LevelPlayerStopRunning();

void LevelPlayerJump();

void LevelPlayerTick();

// Continues the game after dying.
void LevelPlayerContinue();

void LevelPlayerSetCheckpoint();


// Initializes and adds an enemy to the level in the given origin
void LevelEnemyAdd(Vector2 origin);

// Initializes and adds an enemy to the level in the given origin,
// if there are not other elements there already.
void LevelEnemyCheckAndAdd(Vector2 origin);

void LevelEnemyTick(ListNode *enemyNode);

// Checks for collision between a rectangle and any 
// living entity in the level.
bool LevelCheckCollisionWithAnyEntity(Rectangle hitbox);

// Checks for collision between a rectangle and any 
// living entity in the level, including their origins.
bool LevelCheckCollisionWithAnything(Rectangle hitbox);

void LevelEnemyKill(LevelEntity *entity);

void LevelBlockAdd(Vector2 origin);

// Initializes and adds a block to the level in the given origin,
// if there are no other blocks there already.
void LevelBlockCheckAndAdd(Vector2 origin);

void LevelAcidAdd(Vector2 origin);

// Initializes and adds an acid block to the level in the given origin,
// if there are no other blocks there already.
void LevelAcidCheckAndAdd(Vector2 origin);

void LevelGlideAdd(Vector2 origin);

void LevelGlideCheckAndAdd(Vector2 origin);

LevelEntity *LevelCheckpointAdd(Vector2 pos);


#endif // _LEVEL_H_INCLUDED_

#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>

#include "../linked_list.hpp"
#include "../assets.hpp"
#include "../core.hpp"
#include "../persistence.hpp"


// The level used as a basis for new levels
#define LEVEL_BLUEPRINT_NAME    "_blueprint.lvl"

// The size of cells that make up the level grid
#define LEVEL_GRID              Dimensions(32, 32)

// Below this y entities die
#define FLOOR_DEATH_HEIGHT      800


// TODO rename from 'component' to 'tag'
typedef enum LevelEntityComponent {
    LEVEL_IS_PLAYER         = 1,
    LEVEL_IS_ENEMY          = 2,
    LEVEL_IS_SCENARIO       = 4, // the meaning of this component isnt clear
    LEVEL_IS_EXIT           = 8,
    LEVEL_IS_GROUND         = 16,
    LEVEL_IS_DANGER         = 32,
    LEVEL_IS_GLIDE          = 64,
    LEVEL_IS_CHECKPOINT     = 128,
    LEVEL_IS_TEXTBOX        = 256,
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

typedef struct LevelState {

    // The head of the linked list of all the level entities
    ListNode *listHead;

    // The current loaded level's name
    char levelName[LEVEL_NAME_BUFFER_SIZE];

    // If the selected dot has no associated level, and so the level scene
    // is waiting for a level file to dropped so it can be associated
    bool awaitingAssociation;

    bool isPaused;

    // How long ago, in seconds, the level concluded, or -1 if it's not concluded
    double concludedAgo;

    // Reference to the level exit in the level entity's list
    ListNode *exitNode;

    // Reference to the current checkpoint in the level entity's list
    LevelEntity *checkpoint;

    // How many checkpoints the player has left
    int checkpointsLeft;

} LevelState;


extern LevelState *LEVEL_STATE;


// Initialize the level system
void LevelInitialize();

// Loads and goes to the given level
void LevelLoad(char *levelName);

// Starts "go to Overworld" routine
void LevelGoToOverworld();

// Initializes and adds an exit to the level
void LevelExitAdd(Vector2 pos);

// Initializes and adds an exit to the level in the given origin,
// if there are no other entities there already
void LevelExitCheckAndAdd(Vector2 pos);

// Initializes and adds a textbox button to the level
void LevelTextboxAdd(Vector2 pos);

// Initializes and adds a a textbox button to the level in the
// given origin, if there are no other entities there already
void LevelTextboxCheckAndAdd(Vector2 pos);

// The ground beneath the entity, or 0 if not on the ground.
// Gives priority to the tile in the direction the player is looking at.
LevelEntity *LevelGetGroundBeneath(LevelEntity *entity);

// The ground beneath a hitbox, or 0 if not on the ground.
LevelEntity *LevelGetGroundBeneathHitbox(Rectangle hitbox);

// Destroys a LevelEntity
void LevelEntityDestroy(ListNode *node);

// Searches for level entity in position
LevelEntity *LevelEntityGetAt(Vector2 pos);

// Removes level entity in position from the
// list and destroys it, if found and if allowed.
void LevelEntityRemoveAt(Vector2 pos);

// Toggles between paused and unpaused game
void LevelPauseToggle();

// Runs the update routine of the level's entities
void LevelTick();

// Saves to file the current loaded level's data
void LevelSave();

// Loads a new, default level
void LevelLoadNew();

// Get this entity's origin hitbox, based on the current hitbox.
Rectangle LevelEntityOriginHitbox(LevelEntity *entity);

// Checks for collision between a rectangle and any 
// living entity in the level.
bool LevelCheckCollisionWithAnyEntity(Rectangle hitbox);

// Checks for collision between a rectangle and any 
// living entity in the level, including their origins.
bool LevelCheckCollisionWithAnything(Rectangle hitbox);

// Checks for collision between a rectangle and any living entity in the level,
// including their origins, as long as it's NOT present in a given entity list.
bool LevelCheckCollisionWithAnythingElse(Rectangle hitbox, ListNode *entityListHead);

// Adds a level checkpoint in the given position
LevelEntity *LevelCheckpointAdd(Vector2 pos);


#endif // _LEVEL_H_INCLUDED_

#ifndef _LEVEL_H_INCLUDED_
#define _LEVEL_H_INCLUDED_


#include <raylib.h>
#include <string>
#include <vector>

#include "../linked_list.hpp"
#include "../assets.hpp"
#include "../core.hpp"
#include "../persistence.hpp"
#include "../render.hpp"


// The level used as a basis for new levels
#define LEVEL_BLUEPRINT_NAME    "_blueprint.lvl"

// The size of cells that make up the level grid
#define LEVEL_GRID              Dimensions(32, 32)

// Below this y entities die
#define FLOOR_DEATH_HEIGHT      800


namespace Level {


typedef enum {
    IS_PLAYER       = 1,
    IS_ENEMY        = 2,
    IS_SCENARIO     = 4, // the meaning of this component isnt clear
    IS_EXIT         = 8,
    IS_GROUND       = 16,
    IS_DANGER       = 32,
    IS_GLIDE        = 64,
    IS_CHECKPOINT   = 128,
    IS_TEXTBOX      = 256,
} EntityTag;

class Entity : public LinkedList::Node, public Render::IDrawable {

public:
    unsigned long int tags;
    Vector2 origin;
    Rectangle hitbox;

    Sprite sprite;
    int layer;

    bool isDead;

    bool isFacingRight;
    bool isFallingDown;

    // Used by Textbox Buttons
    int textId;

    void Draw();
};

typedef struct LevelState {

    // The head of the linked list of all the level entities
    LinkedList::Node *listHead;

    // The current loaded level's name
    char levelName[LEVEL_NAME_BUFFER_SIZE];

    // If the selected dot has no associated level, and so the level scene
    // is waiting for a level file to dropped so it can be associated
    bool awaitingAssociation;

    bool isPaused;

    // How long ago, in seconds, the level concluded, or -1 if it's not concluded
    double concludedAgo;

    // Reference to the level exit in the level entity's list
    LinkedList::Node *exitNode;

    // Reference to the current checkpoint in the level entity's list
    Entity *checkpoint;

    // How many checkpoints the player has left
    int checkpointsLeft;

} LevelState;


extern LevelState *STATE;


// Initialize the level system
void Initialize();

// Loads and goes to the given level
void Load(char *levelName);

// Starts "go to Overworld" routine
void GoToOverworld();

// Adds a level checkpoint in the given position
Entity *CheckpointAdd(Vector2 pos);

// Initializes and adds an exit to the level
void ExitAdd(Vector2 pos);

// Initializes and adds an exit to the level in the given origin,
// if there are no other entities there already
void ExitCheckAndAdd(Vector2 pos);

// Initializes and adds a textbox button to the level
void TextboxAdd(Vector2 pos, int textId);

// Initializes and adds a a textbox button to the level in the
// given origin, if there are no other entities there already
void TextboxCheckAndAdd(Vector2 pos);

// The ground beneath the entity, or 0 if not on the ground.
// Gives priority to the tile in the direction the player is looking at.
Entity *GetGroundBeneath(Entity *entity);

// The ground beneath a hitbox, or 0 if not on the ground.
Entity *GetGroundBeneathHitbox(Rectangle hitbox);

// Destroys a Entity
void EntityDestroy(LinkedList::Node *node);

// Searches for level entity in position
Entity *EntityGetAt(Vector2 pos);

// Removes level entity in position from the
// list and destroys it, if found and if allowed.
void EntityRemoveAt(Vector2 pos);

// Toggles between paused and unpaused game
void PauseToggle();

// Runs the update routine of the level's entities
void Tick();

// Saves to file the current loaded level's data
void Save();

// Loads a new, default level
void LoadNew();

// Get this entity's origin hitbox, based on the current hitbox.
Rectangle EntityOriginHitbox(Entity *entity);

// Checks for collision between a rectangle and any 
// living entity in the level.
bool CheckCollisionWithAnyEntity(Rectangle hitbox);

// Checks for collision between a rectangle and any 
// living entity in the level, including their origins.
bool CheckCollisionWithAnything(Rectangle hitbox);

// Checks for collision between a rectangle and any living entity in the level,
// including their origins, as long as it's NOT present in a given entity vector.
bool CheckCollisionWithAnythingElse(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore);


} // namespace


#endif // _LEVEL_H_INCLUDED_

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
#define FLOOR_DEATH_HEIGHT      1400

#define EXIT_PERSISTENCE_ID     "lvl_exit"


namespace Level {


typedef enum {
    IS_PLAYER               = 1,
    IS_ENEMY                = 2,
    IS_COLLIDE_WALL         = 4,
    IS_EXIT                 = 8,
    IS_GROUND               = 16,
    IS_COLLIDE_DANGER       = 32,
    IS_GLIDE_PICKUP         = 64,
    //IS_CHECKPOINT           = 128,
    IS_TEXTBOX              = 256,
    //IS_HOOK                 = 512,
    IS_HOOKABLE             = 1024,
    IS_CHECKPOINT_PICKUP    = 2048,
    IS_MOVING_PLATFORM      = 4096,
    IS_ANCHOR               = 8192,
    IS_PERSISTABLE          = 16384,
    IS_GRIDLOCKED           = 32768
} EntityTag;


class Entity : public LinkedList::Node, public Render::IDrawable, public IPersistable {

public:
    unsigned long int tags;
    Vector2 origin;
    Rectangle hitbox;

    Sprite *sprite;
    int layer;

    bool isDead;

    bool isFacingRight;
    bool isFallingDown;

    // It's a variable so it supports entity types that simply instantiates Entity (i.e. not a subclass)
    std::string persistanceEntityID = "!!!_unknown_level_entity";
    

    virtual Rectangle GetOriginHitbox() {
        return {
            origin.x, origin.y,
            hitbox.width, hitbox.height
        };
    }

    virtual void SetHitboxPos(Vector2 pos) {
        RectangleSetPos(&hitbox, pos);
    }

    virtual void SetOrigin(Vector2 origin) {
        this->origin = origin;
    }    

    virtual void Tick();

    virtual std::string GetEntityDebugString();

    void Draw();
    void DrawMoveGhost();

    virtual std::string PersistanceSerialize();
    virtual void PersistenceParse(const std::string &data);

    std::string PersistanceEntityID() {
        return persistanceEntityID;
    }

    // If the entity's hitbox should be ignored when it's dead.
    bool IsADeadEnemy() {
        return (tags & IS_ENEMY) && isDead;
    }
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
    Entity *exit;

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

// Adds a level checkpoint flag in the given position
Entity *CheckpointFlagAdd(Vector2 pos);

// Initializes and adds an exit to the level
Entity *ExitAdd();

// Initializes and adds an exit to the level in the given origin
Entity *ExitAdd(Vector2 pos);

// Initializes and adds an exit to the level in the given origin,
// if there are no other entities there already
void ExitCheckAndAdd(Vector2 pos);

// The ground beneath the entity, or 0 if not on the ground.
// Gives priority to the tile in the direction the player is looking at.
Entity *GetGroundBeneath(Entity *entity);

// The ground beneath a hitbox, or 0 if not on the ground.
Entity *GetGroundBeneathHitbox(Rectangle hitbox);

// Destroys an Entity
void EntityDestroy(Entity *entity);

// Searches for any level entity in the given position
Entity *EntityGetAt(Vector2 pos);

// Searches for a removeable entity in the given position,
// or for its ghost.
Entity *EntityGetRemoveableAt(Vector2 pos);

// Toggles between paused and unpaused game
void PauseToggle();

// Runs the update routine of the level's entities
void Tick();

// Saves to file the current loaded level's data
void Save();

// Loads a new, default level
void LoadNew();

// Checks for collision between a rectangle and any 
// living entity in the level.
Entity *CheckCollisionWithAnyEntity(Rectangle hitbox);

// Checks for collision between a rectangle and any 
// living entity in the level, including their origins.
Entity *CheckCollisionWithAnything(Rectangle hitbox);

// Checks for collision between a rectangle and any living entity in the level,
// including their origins, as long as it's NOT present in a given entity vector.
Entity *CheckCollisionWithAnythingElse(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore);


} // namespace


#endif // _LEVEL_H_INCLUDED_

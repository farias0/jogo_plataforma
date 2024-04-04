#ifndef _OVERWORLD_H_INCLUDED_
#define _OVERWORLD_H_INCLUDED_

#include <vector>

#include "assets.hpp"
#include "linked_list.hpp"
#include "persistence.hpp"
#include "core.hpp"


// The size of cells that make up the overworld grid
#define OW_GRID         Dimensions(64, 64)


typedef enum OverworldCursorDirection {
    OW_CURSOR_UP,
    OW_CURSOR_DOWN,
    OW_CURSOR_LEFT,
    OW_CURSOR_RIGHT
} OverworldCursorDirection;

typedef enum OverworldTileType {
    OW_NOT_TILE,
    OW_LEVEL_DOT,
    OW_STRAIGHT_PATH,
    OW_JOIN_PATH,
    OW_PATH_IN_L
} OverworldTileType;

typedef enum OverworldEntityTag {
    OW_IS_CURSOR            = 1,
    OW_IS_LEVEL_DOT         = 2,
    OW_IS_PATH              = 4,
    OW_IS_ROTATABLE         = 8,
} OverworldEntityTag;

class OverworldEntity : public LinkedList::Node {

public:
    unsigned long int tags;
    OverworldTileType tileType;
    
    Vector2 gridPos;
    Sprite sprite;
    int layer; // currently, only used to put the cursor in front of the tiles
    
    char *levelName;

};

typedef struct OverworldState {

    // The head of the linked list of all the overworld entities
    LinkedList::Node *listHead;

    // Reference to the tile the cursor is over, part of the overworld entity list 
    OverworldEntity *tileUnderCursor;

} OverworldState;


extern OverworldState *OW_STATE;


// Initialize the overworld system
void OverworldInitialize();

// Goes to the overworld
void OverworldLoad();

// Initializes and adds a new tile to the overworld,
// and returns it.
OverworldEntity *OverworldTileAdd(Vector2 pos, OverworldTileType type, int degrees);

// Searches for an overworld entity at a given position
OverworldEntity *OverworldEntityGetAt(Vector2 pos);

// If the current tile contains a level, starts 'go to level' routine and select it
void OverworldLevelSelect();

void OverworldCursorMove(OverworldCursorDirection direction);

// Adds tile to overworld in according to the item selected in the editor,
// or interacts with it if it's already present.
void OverworldTileAddOrInteract(Vector2 pos);

// Removes tile from overworld and destroys it, if present and if allowed.
void OverworldTileRemoveAt(Vector2 pos);

// Checks for collision between a hitbox and any tile in the overworld.
// Returns the collided entity, or 0 if none.
OverworldEntity *OverworldCheckCollisionWithAnyTile(Rectangle hitbox);

// Checks for collision between a hitbox and any tile in the overworld
// that's NOT present in a given vector. Returns the collided entity, or 0 if none.
OverworldEntity *OverworldCheckCollisionWithAnyTileExcept(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore);

void OverworldTick();

// Saves overworld's state to persistence.
void OverworldSave();

// Returns the grid square for a given entity
Rectangle OverworldEntitySquare(OverworldEntity *entity);


#endif // _OVERWORLD_H_INCLUDED_

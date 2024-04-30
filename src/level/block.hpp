#ifndef _BLOCK_H_INCLUDED_
#define _BLOCK_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"

#define BLOCK_PERSISTENCE_ID        "block"
#define ACID_BLOCK_PERSISTENCE_ID   "acid_block"


// Adds a block to the level
Level::Entity *BlockAdd();

// Adds a block to the level in the given position
Level::Entity *BlockAdd(Vector2 origin);

// Initializes and adds a block to the level in the given origin,
// if there are no other blocks there already
void BlockCheckAndAdd(Vector2 origin);

// Adds an acid block to the level
Level::Entity *AcidAdd();

// Adds an acid block to the level in the given position
Level::Entity *AcidAdd(Vector2 origin);

// Initializes and adds an acid block to the level in the given origin,
// if there are no other blocks there already
void AcidCheckAndAdd(Vector2 origin);


#endif // _BLOCK_H_INCLUDED_

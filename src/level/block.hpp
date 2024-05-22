#ifndef _BLOCK_H_INCLUDED_
#define _BLOCK_H_INCLUDED_


#include <raylib.h>

#include "level.hpp"

#define BLOCK_PERSISTENCE_ID        "block"
#define ACID_BLOCK_PERSISTENCE_ID   "acid_block"


class Block : public Level::Entity {

public:
    
    // Adds a block to the level
    static Block *Add();

    // Adds a block to the level in the given position
    static Block *Add(Vector2 origin);

    // Initializes and adds a block to the level in the given origin,
    // if there are no other blocks there already
    static void CheckAndAdd(Vector2 origin);

};

class AcidBlock : public Level::Entity {

public:
    
    // Adds an acid block to the level
    static AcidBlock *Add();

    // Adds an acid block to the level in the given position
    static AcidBlock *Add(Vector2 origin);

    // Initializes and adds an acid block to the level in the given origin,
    // if there are no other blocks there already
    static void CheckAndAdd(Vector2 origin);

};

#endif // _BLOCK_H_INCLUDED_

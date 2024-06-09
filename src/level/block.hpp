#ifndef _BLOCK_H_INCLUDED_
#define _BLOCK_H_INCLUDED_


#include <raylib.h>
#include <map>

#include "level.hpp"
#include "../assets.hpp"

#define BLOCK_PERSISTENCE_ID        "block"
#define ACID_BLOCK_PERSISTENCE_ID   "acid_block"


class Block : public Level::Entity {

public:

    static void InitializeTileMap();
    
    // Adds a block to the level
    static Block *Add();

    // Adds a block to the level in the given position
    static Block *Add(Vector2 origin);

    // Initializes and adds a block to the level in the given origin,
    // or interacts with an existing block if present in this position
    static void AddOrInteract(Vector2 origin, int interactionTags);

    void TileTypeSet(const std::string &tileTypeId);

    void TileTypeNext();

    // Identifies and set to the correct tile type based on the surrounding blocks
    void TileAutoAdjust();

    void TileRotate();

    void Draw() override;

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;
    
    std::string EntityTypeID() {
        return BLOCK_PERSISTENCE_ID;
    }
    
private:

    // Defines the different block tile types and which sprite to use for each of them
    static std::map<std::string, Sprite*> tileSpriteMap;

    int rotation;

    std::string tileTypeId;

};

class AcidBlock : public Level::Entity {

public:
    
    // Adds an acid block to the level
    static AcidBlock *Add();

    // Adds an acid block to the level in the given position
    static AcidBlock *Add(Vector2 origin);

    // Initializes and adds an acid block to the level in the given origin,
    // if there are no other blocks there already
    static void CheckAndAdd(Vector2 origin, int interactionTags);

};

#endif // _BLOCK_H_INCLUDED_

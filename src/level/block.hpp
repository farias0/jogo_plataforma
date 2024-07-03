#ifndef _BLOCK_H_INCLUDED_
#define _BLOCK_H_INCLUDED_


#include <raylib.h>
#include <map>

#include "level.hpp"
#include "../assets.hpp"

#define BLOCK_ENTITY_ID        "block"
#define ACID_BLOCK_ENTITY_ID   "acid_block"


class Block : public Level::Entity {

public:

    static void InitializeTileMap();
    
    // Adds a block to the level
    static Block *AddFromPersistence();

    // Adds a block to the level in the given position
    static Block *Add(Vector2 origin);

    // Initializes and adds a block to the level in the given origin,
    // or interacts with an existing block if present in this position
    static void AddFromEditor(Vector2 origin, int interactionTags);

    void TileTypeSet(const std::string &tileTypeId);

    void TileTypeNext();

    /*
        Identifies and set to the correct tile type based on the surrounding blocks

        @param treatAsSlope: If the block should be treated as a slope or as square block block if applicable
    */
    void TileAutoAdjust(bool treatAsSlope);

    void TileRotate();

    // If the block is a slope, returns true if the slope slopes to the right
    bool IsSlopeToTheRight();

    void Draw() override;

    std::string PersistanceSerialize() override;
    
    void PersistenceParse(const std::string &data) override;
    
private:

    // Defines the different block tile types and which sprite to use for each of them
    static std::map<std::string, Sprite*> tileSpriteMap;

    int rotation;

    std::string tileTypeId;

};

class AcidBlock : public Level::Entity {

public:
    
    // Adds an acid block to the level
    static AcidBlock *AddFromPersistence();

    // Adds an acid block to the level in the given position
    static AcidBlock *Add(Vector2 origin);

    // Initializes and adds an acid block to the level in the given origin,
    // if there are no other blocks there already
    static void AddFromEditor(Vector2 origin, int interactionTags);

};

#endif // _BLOCK_H_INCLUDED_

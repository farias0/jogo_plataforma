#include <raylib.h>
#include <map>
#include <stdexcept>

#include "block.hpp"
#include "level.hpp"
#include "../input.hpp"
#include "../camera.hpp"

#define DEFAULT_TILE_TYPE "4Sides"


std::map<std::string, Sprite*> Block::tileSpriteMap;

void Block::InitializeTileMap() {
    tileSpriteMap = {
        { "4Sides", &SPRITES->Block4Sides },
        { "1Side", &SPRITES->Block1Side },
        { "2SidesOpp", &SPRITES->Block2SidesOpp },
        { "2SidesAdj", &SPRITES->Block2SidesAdj },
        { "3Sides", &SPRITES->Block3Sides },
    };
}

Block *Block::Add() {
    return Add({ 0,0 });
}

Block *Block::Add(Vector2 origin) {

    Block *newBlock = new Block();

    newBlock->tags = Level::IS_COLLIDE_WALL +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_PERSISTABLE +
                            Level::IS_GRIDLOCKED +
                            Level::IS_TILE_BLOCK;
    newBlock->origin = origin;
    newBlock->SetTileType(DEFAULT_TILE_TYPE);
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);


    LinkedList::AddNode(&Level::STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);

    return newBlock;
}

void Block::AddOrInteract(Vector2 origin, int interactionTags) {


    origin = SnapToGrid(origin, LEVEL_GRID);
    Rectangle ghostHitbox = SpriteHitboxFromEdge(&SPRITES->Block4Sides, origin);
    Level::Entity *collidedEntity = Level::CheckCollisionWithAnything(ghostHitbox);


    if (collidedEntity) {

        if ((collidedEntity->tags & Level::IS_TILE_BLOCK) && (interactionTags & EDITOR_INTERACTION_CLICK)) {

            Block *existingBlock = (Block *) collidedEntity;

            if (interactionTags & EDITOR_INTERACTION_ALT)
                existingBlock->TileTypeNext();
            else
                existingBlock->TileRotate();
        }

    }
    else {
        Add(origin);
    }
}

void Block::TileTypeNext() {

    for (auto it = tileSpriteMap.begin(); it != tileSpriteMap.end(); it++) {

        if (tileTypeId == it->first) {

            if (std::next(it) == tileSpriteMap.end()) { // last item
                it = tileSpriteMap.begin();
            } else {
                it++;
            }

            SetTileType(it->first);
            Render::PrintSysMessage(tileTypeId.c_str());
            return;
        }
    }

    TraceLog(LOG_ERROR, "Block tried to toggle type, but couldn't find sprite (tileTypeId=%s).", tileTypeId.c_str());
}

void Block::TileRotate() {
    
    rotation += 90;
    if (rotation >= 360) rotation -= 360;
}

void Block::Draw() {

    Vector2 pos = PosInSceneToScreen({
                                        hitbox.x,
                                        hitbox.y });

    Render::DrawTexture(sprite, { pos.x, pos.y }, WHITE, rotation, false);
}

std::string Block::PersistanceSerialize() {

    std::string data = Level::Entity::PersistanceSerialize();
    persistanceAddValue(&data, "rotation", std::to_string(rotation));
    persistanceAddValue(&data, "tileType", tileTypeId);
    return data;
}

void Block::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    rotation = std::stoi(persistenceReadValue(data, "rotation"));
    SetTileType(persistenceReadValue(data, "tileType"));
}

void Block::SetTileType(const std::string &id) {

    tileTypeId = id;

    try {
        sprite = tileSpriteMap.at(id);
    }
    catch (const std::out_of_range &ex) {
        TraceLog(LOG_ERROR, "Block couldn't set tile type '%s'.", id.c_str());
        sprite = tileSpriteMap.at(DEFAULT_TILE_TYPE);
    }
}

//

AcidBlock *AcidBlock::Add() {
    return Add({ 0,0 });
}

AcidBlock *AcidBlock::Add(Vector2 origin) {

    AcidBlock *newBlock = new AcidBlock();

    newBlock->tags = Level::IS_COLLIDE_WALL +
                            Level::IS_GROUND +
                            Level::IS_COLLIDE_DANGER +
                            Level::IS_HOOKABLE +
                            Level::IS_PERSISTABLE +
                            Level::IS_GRIDLOCKED;
    newBlock->origin = origin;
    newBlock->sprite = &SPRITES->Acid;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    newBlock->persistanceEntityID = ACID_BLOCK_PERSISTENCE_ID;

    LinkedList::AddNode(&Level::STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added acid block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);

    return newBlock;
}

void AcidBlock::CheckAndAdd(Vector2 origin, int interactionTags) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->Acid, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    Add(origin);
}

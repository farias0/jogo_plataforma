#include <raylib.h>

#include "block.hpp"
#include "level.hpp"


Block *Block::Add() {
    return Add({ 0,0 });
}

Block *Block::Add(Vector2 origin) {

    Block *newBlock = new Block();

    newBlock->tags = Level::IS_COLLIDE_WALL +
                            Level::IS_GROUND +
                            Level::IS_HOOKABLE +
                            Level::IS_PERSISTABLE +
                            Level::IS_GRIDLOCKED;
    newBlock->origin = origin;
    newBlock->sprite = &SPRITES->Block;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    newBlock->persistanceEntityID = BLOCK_PERSISTENCE_ID;

    LinkedList::AddNode(&Level::STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);

    return newBlock;
}

void Block::CheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->Block, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    Add(origin);
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

void AcidBlock::CheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(&SPRITES->Acid, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    Add(origin);
}

#include <raylib.h>

#include "block.hpp"
#include "level.hpp"
#include "../input.hpp"
#include "../camera.hpp"


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
    newBlock->sprite = &SPRITES->Block4Sides;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    newBlock->persistanceEntityID = BLOCK_PERSISTENCE_ID;

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
                existingBlock->ToggleTileType();
            else
                existingBlock->ToggleTileRotation();
        }

    }
    else {
        Add(origin);
    }
}

void Block::ToggleTileType() {

    if (sprite == &SPRITES->Block4Sides) sprite = &SPRITES->Block1Side;
    else if (sprite == &SPRITES->Block1Side) sprite = &SPRITES->Block2SidesOpp;
    else if (sprite == &SPRITES->Block2SidesOpp) sprite = &SPRITES->Block2SidesAdj;
    else if (sprite == &SPRITES->Block2SidesAdj) sprite = &SPRITES->Block3Sides;
    else if (sprite == &SPRITES->Block3Sides) sprite = &SPRITES->Block4Sides;
    else TraceLog(LOG_ERROR, "Block tried to toggle type, but sprite is unknown.");
}

void Block::ToggleTileRotation() {
    
    rotation += 90;
    if (rotation >= 360) rotation -= 360;
}

void Block::Draw() {

    Vector2 pos = PosInSceneToScreen({
                                        hitbox.x,
                                        hitbox.y });

    Render::DrawTexture(sprite, { pos.x, pos.y }, WHITE, rotation, false);
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

#include <raylib.h>

#include "block.hpp"
#include "level.hpp"


void BlockAdd(Vector2 origin) {

    Level::Entity *newBlock = new Level::Entity();

    newBlock->tags = Level::IS_SCENARIO +
                            Level::IS_GROUND;
    newBlock->origin = origin;
    newBlock->sprite = SPRITES->Block;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedList::AddNode(&Level::STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void BlockCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(SPRITES->Block, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    BlockAdd(origin);
}

void AcidAdd(Vector2 origin) {

    Level::Entity *newBlock = new Level::Entity();

    newBlock->tags = Level::IS_SCENARIO +
                            Level::IS_GROUND +
                            Level::IS_DANGER;
    newBlock->origin = origin;
    newBlock->sprite = SPRITES->Acid;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedList::AddNode(&Level::STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added acid block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void AcidCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(SPRITES->Acid, origin);
    if (Level::CheckCollisionWithAnything(hitbox)) return;
    
    AcidAdd(origin);
}

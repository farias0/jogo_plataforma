#include <raylib.h>

#include "block.hpp"
#include "level.hpp"


void BlockAdd(Vector2 origin) {

    LevelEntity *newBlock = (LevelEntity *) MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND;
    newBlock->origin = origin;
    newBlock->sprite = SPRITES->Block;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void BlockCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(SPRITES->Block, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    BlockAdd(origin);
}

void AcidAdd(Vector2 origin) {

    LevelEntity *newBlock = (LevelEntity *) MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND +
                            LEVEL_IS_DANGER;
    newBlock->origin = origin;
    newBlock->sprite = SPRITES->Acid;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added acid block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void AcidCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(SPRITES->Acid, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    AcidAdd(origin);
}
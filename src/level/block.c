#include <raylib.h>

#include "block.h"
#include "level.h"


void BlockAdd(Vector2 origin) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND;
    newBlock->origin = origin;
    newBlock->sprite = BlockSprite;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void BlockCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(BlockSprite, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    BlockAdd(origin);
}

void AcidAdd(Vector2 origin) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND +
                            LEVEL_IS_DANGER;
    newBlock->origin = origin;
    newBlock->sprite = AcidSprite;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_STATE->listHead, newBlock);

    TraceLog(LOG_TRACE, "Added acid block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void AcidCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(AcidSprite, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    AcidAdd(origin);
}

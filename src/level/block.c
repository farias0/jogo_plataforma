#include <raylib.h>

#include "level.h"
#include "../core.h"


void LevelBlockAdd(Vector2 origin) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND;
    newBlock->origin = origin;
    newBlock->sprite = BlockSprite;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_LIST_HEAD, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void LevelAcidAdd(Vector2 origin) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND +
                            LEVEL_IS_DANGER;
    newBlock->origin = origin;
    newBlock->sprite = AcidSprite;
    newBlock->hitbox = SpriteHitboxFromEdge(newBlock->sprite, newBlock->origin);

    LinkedListAdd(&LEVEL_LIST_HEAD, newBlock);

    TraceLog(LOG_TRACE, "Added acid block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void LevelBlockCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(BlockSprite, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    LevelBlockAdd(origin);
}

void LevelAcidCheckAndAdd(Vector2 origin) {

    origin = SnapToGrid(origin, LEVEL_GRID);

    Rectangle hitbox = SpriteHitboxFromEdge(AcidSprite, origin);
    if (LevelCheckCollisionWithAnything(hitbox)) return;
    
    LevelAcidAdd(origin);
}

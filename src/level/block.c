#include <raylib.h>

#include "level.h"
#include "../core.h"


void LevelBlockAdd(Vector2 origin) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    origin.x = SnapToGrid(origin.x, LEVEL_GRID.width);
    origin.y = SnapToGrid(origin.y, LEVEL_GRID.height);

    newBlock->components = LEVEL_IS_SCENARIO +
                            LEVEL_IS_GROUND;
    newBlock->origin = origin;
    newBlock->hitbox = SpriteHitboxFromEdge(BlockSprite, origin);
    newBlock->sprite = BlockSprite;

    LinkedListAdd(&LEVEL_LIST_HEAD, newBlock);

    TraceLog(LOG_TRACE, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void LevelBlockCheckAndAdd(Vector2 origin) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {
        
        LevelEntity *possibleBlock = (LevelEntity *)node->item;

        if (possibleBlock->components & LEVEL_IS_SCENARIO &&
                CheckCollisionPointRec(origin, possibleBlock->hitbox)) {

            return;
        }

        node = node->next;
    }

    LevelBlockAdd(origin);
}

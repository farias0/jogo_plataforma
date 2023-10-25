#include <raylib.h>

#include "level.h"
#include "../core.h"


void LevelBlockAdd(Vector2 pos) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    pos.x = SnapToGrid(pos.x, LEVEL_GRID.width);
    pos.y = SnapToGrid(pos.y, LEVEL_GRID.height);

    newBlock->components = LEVEL_IS_SCENARIO;
    newBlock->hitbox = SpriteHitboxFromEdge(BlockSprite, pos);
    newBlock->sprite = BlockSprite;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newBlock;
    LinkedListAdd(&LEVEL_LIST_HEAD, node);

    TraceLog(LOG_DEBUG, "Added block to level (x=%.1f, y=%.1f)",
                newBlock->hitbox.x, newBlock->hitbox.y);
}

void LevelBlockCheckAndAdd(Vector2 pos) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {
        
        LevelEntity *possibleBlock = (LevelEntity *)node->item;

        if (possibleBlock->components & LEVEL_IS_SCENARIO &&
                CheckCollisionPointRec(pos, possibleBlock->hitbox)) {

            return;
        }

        node = node->next;
    }

    LevelBlockAdd(pos);
}

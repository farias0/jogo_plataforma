#include <raylib.h>

#include "level.h"
#include "../core.h"


void LevelBlockAdd(Rectangle hitbox) {

    LevelEntity *newBlock = MemAlloc(sizeof(LevelEntity));

    hitbox.x = SnapToGrid(hitbox.x, LEVEL_GRID.width);
    hitbox.y = SnapToGrid(hitbox.y, LEVEL_GRID.height);

    newBlock->components = LEVEL_IS_SCENARIO;
    newBlock->hitbox = hitbox;
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

    LevelBlockAdd((Rectangle){ pos.x, pos.y, BlockSprite.sprite.width, BlockSprite.sprite.height });
}

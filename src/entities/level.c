#include <raylib.h>
#include "stdlib.h"

#include <stdio.h>

#include "level.h"
#include "../global.h"
#include "entity.h"
#include "enemy.h"
#include "../assets.h"


/*
    Snaps a coordinate (x or y) into the grid BlockSprites.
*/
float snapToGrid(float v) {
    return v - (abs(v) % (int) BlockSprite.sprite.width); // BlockSprite is square, so same for x and y.
}

Entity *addBlockToLevel(Entity *head, Rectangle hitbox) {
    Entity *entity = MemAlloc(sizeof(Entity));

    hitbox.x = snapToGrid(hitbox.x);
    hitbox.y = snapToGrid(hitbox.y);

    entity->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    entity->hitbox = hitbox;
    entity->sprite = BlockSprite;

    return AddToEntityList(head, entity);
}

Entity *InitializeLevel(Entity *head) {

    head = addBlockToLevel(head, (Rectangle){ 0, FLOOR_HEIGHT, BlockSprite.sprite.width*25, BlockSprite.sprite.height*5 });
    
    float x = BlockSprite.sprite.width*30;
    float width = BlockSprite.sprite.width*10;
    head = addBlockToLevel(head, (Rectangle){ x, FLOOR_HEIGHT, width, BlockSprite.sprite.height*5 });
    head = InitializeEnemy(head, x + (width / 2), 200);
    
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*45, FLOOR_HEIGHT-80, BlockSprite.sprite.width*10, BlockSprite.sprite.height*2 });
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*40, FLOOR_HEIGHT-200, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*45, FLOOR_HEIGHT-320, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });

    SetEntityPosition(PLAYER, SCREEN_WIDTH/5, 300);

    return head;
}

void AddBlockToLevel(Entity *head, Vector2 pos) {

    Entity *possibleBlock = head;

    // Check if there's a block there already (currently only works for 1x1 blocks)
    while (possibleBlock != 0) {
        
        if (possibleBlock->components & IsLevelElement &&
                possibleBlock->hitbox.x == snapToGrid(pos.x) &&
                possibleBlock->hitbox.y == snapToGrid(pos.y)) {

            return;
        }

        possibleBlock = possibleBlock->next;

    }

    addBlockToLevel(head, (Rectangle){ pos.x, pos.y, BlockSprite.sprite.width, BlockSprite.sprite.height });
}

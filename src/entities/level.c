#include <raylib.h>
#include "stdlib.h"

#include <stdio.h>

#include "level.h"
#include "../global.h"
#include "entity.h"
#include "enemy.h"
#include "../assets.h"


SpriteDimensions LevelGridDimensions = (SpriteDimensions){
    // Based on a block tile
    32,
    32
};

Vector2 playersStartingPosition =  { SCREEN_WIDTH/5, 300 };


// Snaps a coordinate (x or y) to the Level Grid 
float snapToLevelGrid(float v) {

    if (v >= 0) {
        return v - ((int) v % (int) LevelGridDimensions.width);
    } else {
        return v - LevelGridDimensions.width - ((int) v % (int) LevelGridDimensions.width);
    }
}

Entity *addBlockToLevel(Entity *head, Rectangle hitbox) {
    Entity *newBlock = MemAlloc(sizeof(Entity));

    hitbox.x = snapToLevelGrid(hitbox.x);
    hitbox.y = snapToLevelGrid(hitbox.y);

    newBlock->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    newBlock->hitbox = hitbox;
    newBlock->sprite = BlockSprite;

    return AddToEntityList(head, newBlock);
}

Entity *LoadLevel(Entity *head) {

    head = addBlockToLevel(head, (Rectangle){ 0, FLOOR_HEIGHT, BlockSprite.sprite.width*25, BlockSprite.sprite.height*5 });
    
    float x = BlockSprite.sprite.width*30;
    float width = BlockSprite.sprite.width*10;
    head = addBlockToLevel(head, (Rectangle){ x, FLOOR_HEIGHT, width, BlockSprite.sprite.height*5 });
    head = InitializeEnemy(head, x + (width / 2), 200);
    
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*45, FLOOR_HEIGHT-80, BlockSprite.sprite.width*10, BlockSprite.sprite.height*2 });
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*40, FLOOR_HEIGHT-200, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });
    head = addBlockToLevel(head, (Rectangle){ BlockSprite.sprite.width*45, FLOOR_HEIGHT-320, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });

    return head;
}

Entity *AddBlockToLevel(Vector2 pos) {

    Entity *possibleBlock = ENTITIES_HEAD;

    while (possibleBlock != 0) {
        
        if (possibleBlock->components & IsLevelElement &&
                CheckCollisionPointRec(pos, possibleBlock->hitbox)) {

            return ENTITIES_HEAD;
        }

        possibleBlock = possibleBlock->next;

    }

    return addBlockToLevel(ENTITIES_HEAD, (Rectangle){ pos.x, pos.y, BlockSprite.sprite.width, BlockSprite.sprite.height });
}

Vector2 GetPlayerStartingPosition() {
    return playersStartingPosition;
}

#include <raylib.h>
#include "stdlib.h"

#include <stdio.h>

#include "level.h"
#include "../global.h"
#include "entity.h"
#include "enemy.h"
#include "../assets.h"


typedef struct LevelEnemy {
    Vector2 pos;
} LevelEnemy;

typedef struct LevelBlock {
    Rectangle rect;
} LevelBlock;

typedef struct LevelData {

    int enemyCount;
    LevelEnemy *enemies;

    int blockCount;
    LevelBlock *blocks;

} LevelData;


Texture2D floorTileTexture;


LevelData loadLevelData() {

    LevelData data;
    data.enemyCount = 1;
    data.blockCount = 6;

    data.enemies = MemAlloc(sizeof(LevelEnemy) * data.enemyCount);
    data.blocks = MemAlloc(sizeof(LevelBlock) * data.blockCount);

    data.blocks[0].rect = (Rectangle){ 0,                           FLOOR_HEIGHT,       BlockSprite.sprite.width*25, BlockSprite.sprite.height*5 };

    data.blocks[1].rect = (Rectangle){ BlockSprite.sprite.width*30,  FLOOR_HEIGHT,       BlockSprite.sprite.width*10, BlockSprite.sprite.height*5 };
    data.enemies[0].pos = (Vector2){ data.blocks[1].rect.x + (data.blocks[1].rect.width / 2),
                                        200 };

    data.blocks[2].rect = (Rectangle){ BlockSprite.sprite.width*45,  FLOOR_HEIGHT-80,    BlockSprite.sprite.width*10, BlockSprite.sprite.height*2 };

    data.blocks[3].rect = (Rectangle){ BlockSprite.sprite.width*40,  FLOOR_HEIGHT-200,   BlockSprite.sprite.width*5,  BlockSprite.sprite.height*1 };

    data.blocks[4].rect = (Rectangle){ BlockSprite.sprite.width*45,  FLOOR_HEIGHT-320,   BlockSprite.sprite.width*5,  BlockSprite.sprite.height*1 };

    return data;
}

void freeLoadedLevelData(LevelData data) {

    MemFree(data.blocks);
    MemFree(data.enemies);
} 

/*
    Snaps a coordinate (x or y) into the grid BlockSprites.
*/
float snapToGrid(float v) {
    return v - (abs(v) % (int) BlockSprite.sprite.width); // BlockSprite is square, so same for x and y.
}

Entity *addBlockToLevel(Entity *head, LevelBlock block) {
    Entity *entity = MemAlloc(sizeof(Entity));

    block.rect.x = snapToGrid(block.rect.x);
    block.rect.y = snapToGrid(block.rect.y);

    entity->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    entity->hitbox = block.rect;
    entity->sprite = BlockSprite;

    return AddToEntityList(head, entity);
}

Entity *InitializeLevel(Entity *head) {
    
    floorTileTexture = LoadTexture("../assets/floor_tile_1.png");

    LevelData data = loadLevelData();

    LevelBlock *block = data.blocks;
    for (int idx = 0; idx < data.blockCount; idx++) {
        head = addBlockToLevel(head, block[idx]);
    }

    LevelEnemy *enemy = data.enemies;
    for (int idx = 0; idx < data.enemyCount; idx++) {
        head = InitializeEnemy(head, enemy[idx].pos.x, enemy[idx].pos.y);
    }

    freeLoadedLevelData(data);

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

    LevelBlock block = {
        { pos.x, pos.y, BlockSprite.sprite.width, BlockSprite.sprite.height }
    };

    addBlockToLevel(head, block);
}

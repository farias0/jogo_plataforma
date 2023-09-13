#include <raylib.h>
#include "stdlib.h"

#include <stdio.h>

#include "level.h"
#include "global.h"
#include "entity.h"
#include "enemy.h"

#define FLOOR_TILE_SIZE 32

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

    data.blocks[0].rect = (Rectangle){ 0,                   FLOOR_HEIGHT,       FLOOR_TILE_SIZE*25, FLOOR_TILE_SIZE*5 };

    data.blocks[1].rect = (Rectangle){ FLOOR_TILE_SIZE*30,  FLOOR_HEIGHT,       FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*5 };
    data.enemies[0].pos = (Vector2){ data.blocks[1].rect.x + (data.blocks[1].rect.width / 2),
                                        data.blocks[1].rect.y };

    data.blocks[2].rect = (Rectangle){ FLOOR_TILE_SIZE*45,  FLOOR_HEIGHT-80,    FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*2 };

    data.blocks[3].rect = (Rectangle){ FLOOR_TILE_SIZE*40,  FLOOR_HEIGHT-200,   FLOOR_TILE_SIZE*5,  FLOOR_TILE_SIZE*1 };

    data.blocks[4].rect = (Rectangle){ FLOOR_TILE_SIZE*45,  FLOOR_HEIGHT-320,   FLOOR_TILE_SIZE*5,  FLOOR_TILE_SIZE*1 };

    return data;
}

void freeLoadedLevelData(LevelData data) {

    MemFree(data.blocks);
    MemFree(data.enemies);
} 

void addBlockToLevel(Entity *entitiesItem, LevelBlock block) {
    Entity *entity = MemAlloc(sizeof(Entity));

    entity->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    entity->hitbox = block.rect;
    entity->sprite = floorTileTexture;
    entity->spriteScale = FLOOR_TILE_SIZE;

    AddToEntityList(entitiesItem, entity);
}

Entity *InitializeLevel(Entity *entitiesItem) {
    
    floorTileTexture = LoadTexture("../assets/floor_tile_1.png");

    LevelData data = loadLevelData();

    LevelBlock *block = data.blocks;
    for (int idx = 0; idx < data.blockCount; idx++) {
        addBlockToLevel(entitiesItem, block[idx]);
    }

    LevelEnemy *enemy = data.enemies;
    for (int idx = 0; idx < data.enemyCount; idx++) {
        entitiesItem = InitializeEnemy(entitiesItem, enemy[idx].pos.x, enemy[idx].pos.y);
    }

    freeLoadedLevelData(data);

    SetEntityPosition(PLAYER, SCREEN_WIDTH/5, FLOOR_HEIGHT-PLAYER->hitbox.height);

    return entitiesItem;
}

void AddBlockToLevel(Entity *entitiesItem, Vector2 pos) {

    // Snap the block into a grid
    pos.x -= (abs(pos.x) % FLOOR_TILE_SIZE);
    pos.y -= (abs(pos.y) % FLOOR_TILE_SIZE);

    // Check if there's a block there already (currently only works for 1x1 blocks)
    for (Entity *possibleBlock = entitiesItem->next; possibleBlock != entitiesItem; possibleBlock = possibleBlock->next) {
        
        if (possibleBlock->components & IsLevelElement &&
                possibleBlock->hitbox.x == pos.x &&
                possibleBlock->hitbox.y == pos.y) {

            return;
        }
    }

    LevelBlock block = {
        { pos.x, pos.y, FLOOR_TILE_SIZE, FLOOR_TILE_SIZE }
    };

    addBlockToLevel(entitiesItem, block);
}

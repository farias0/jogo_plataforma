#include <raylib.h>

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
    data.enemyCount = 2;
    data.blockCount = 4;

    data.enemies = MemAlloc(sizeof(LevelEnemy) * data.enemyCount);
    data.blocks = MemAlloc(sizeof(LevelBlock) * data.blockCount);

    data.blocks[0].rect = (Rectangle){ 0,                   FLOOR_HEIGHT,       FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*5 };

    data.blocks[1].rect = (Rectangle){ FLOOR_TILE_SIZE*15,  FLOOR_HEIGHT,       FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*5 };
    data.enemies[0].pos = (Vector2){ data.blocks[1].rect.x + (data.blocks[1].rect.width / 2),
                                        data.blocks[1].rect.y };

    data.blocks[2].rect = (Rectangle){ FLOOR_TILE_SIZE*30,  FLOOR_HEIGHT,       FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*5 };
    data.enemies[1].pos = (Vector2){ data.blocks[2].rect.x + (data.blocks[2].rect.width / 2),
                                        data.blocks[2].rect.y };

    data.blocks[3].rect = (Rectangle){ FLOOR_TILE_SIZE*45,  FLOOR_HEIGHT-50,    FLOOR_TILE_SIZE*10, FLOOR_TILE_SIZE*2 };

    return data;
}

void freeLoadedLevelData(LevelData data) {

    MemFree(data.blocks);
    MemFree(data.enemies);
} 

Entity *InitializeLevel(Entity *entitiesItem) {
    
    floorTileTexture = LoadTexture("../assets/floor_tile_1.png");

    LevelData data = loadLevelData();

    LevelBlock *block = data.blocks;
    for (int idx = 0; idx < data.blockCount; idx++) {
        Entity *entity = MemAlloc(sizeof(Entity));
        entity->components = HasPosition +
                                HasSprite +
                                IsLevelElement;
        entity->hitbox         = block[idx].rect;
        entity->sprite = floorTileTexture;
        entity->spriteScale = FLOOR_TILE_SIZE;
        AddToEntityList(entitiesItem, entity);
    }

    LevelEnemy *enemy = data.enemies;
    for (int idx = 0; idx < data.enemyCount; idx++) {
        entitiesItem = InitializeEnemy(entitiesItem, enemy[idx].pos.x, enemy[idx].pos.y);
    }

    freeLoadedLevelData(data);

    SetEntityPosition(PLAYER, SCREEN_WIDTH/5, 100);

    return entitiesItem;
}

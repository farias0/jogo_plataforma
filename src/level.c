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
    data.blockCount = 3;

    data.enemies = MemAlloc(sizeof(LevelEnemy) * data.enemyCount);
    data.blocks = MemAlloc(sizeof(LevelBlock) * data.blockCount);


    int width = FLOOR_TILE_SIZE * 10;
    int x_spacing = width + (FLOOR_TILE_SIZE * 5);


    data.blocks[0].rect = (Rectangle){ 0,           FLOOR_HEIGHT, width, FLOOR_TILE_SIZE*10 };
    data.blocks[1].rect = (Rectangle){ x_spacing,   FLOOR_HEIGHT, width, FLOOR_TILE_SIZE*10 };
    data.blocks[2].rect = (Rectangle){ x_spacing*2, FLOOR_HEIGHT, width, FLOOR_TILE_SIZE*10 };

    data.enemies[0].pos = (Vector2){ data.blocks[1].rect.x + (data.blocks[1].rect.width / 2),
                                        data.blocks[1].rect.y };
    data.enemies[1].pos = (Vector2){ data.blocks[2].rect.x + (data.blocks[2].rect.width / 2),
                                        data.blocks[2].rect.y };

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

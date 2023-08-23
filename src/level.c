#include <raylib.h>

#include "level.h"
#include "global.h"
#include "entity.h"
#include "enemy.h"

#define FLOOR_TILE_SIZE 32

Texture2D floorTileTexture;

Entity *InitializeLevel(Entity *entitiesItem) {
    
    floorTileTexture = LoadTexture("../assets/floor_tile_1.png");

    int width = FLOOR_TILE_SIZE * 10;
    int x_spacing = width + (FLOOR_TILE_SIZE * 5);


    Entity *block1 = MemAlloc(sizeof(Entity));
    block1->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    block1->hitbox.x       = 0;
    block1->hitbox.y       = FLOOR_HEIGHT;
    block1->hitbox.width   = width;
    block1->hitbox.height  = FLOOR_TILE_SIZE * 10;
    block1->sprite = floorTileTexture;
    AddToEntityList(entitiesItem, block1);

    Entity *block2 = MemAlloc(sizeof(Entity));
    block2->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    block2->hitbox.x       = x_spacing;
    block2->hitbox.y       = FLOOR_HEIGHT;
    block2->hitbox.width   = width;
    block2->hitbox.height  = FLOOR_TILE_SIZE * 10;
    block2->sprite = floorTileTexture;
    AddToEntityList(block1, block2);

    Entity *enemy1 = InitializeEnemy(block2,
                                        block2->hitbox.x + (block2->hitbox.width / 2),
                                        block2->hitbox.y);

    Entity *block3 = MemAlloc(sizeof(Entity));
    block3->components = HasPosition +
                            HasSprite +
                            IsLevelElement;
    block3->hitbox.x       = x_spacing * 2;
    block3->hitbox.y       = FLOOR_HEIGHT;
    block3->hitbox.width   = width;
    block3->hitbox.height  = FLOOR_TILE_SIZE * 10;
    block3->sprite = floorTileTexture;
    AddToEntityList(enemy1, block3);

    Entity *enemy2 = InitializeEnemy(block3,
                                        block3->hitbox.x + (block3->hitbox.width / 2),
                                        block3->hitbox.y);

    return enemy2;
}

void DrawLevel(Entity *entity) {
    
    // How many tiles to be drawn in each axis
    int xTilesCount = entity->hitbox.width / FLOOR_TILE_SIZE;
    int yTilesCount = entity->hitbox.height / FLOOR_TILE_SIZE;

    for (int xCurrent = 0; xCurrent < xTilesCount; xCurrent++) {
        for (int yCurrent = 0; yCurrent < yTilesCount; yCurrent++) {
            DrawTextureEx(
                            floorTileTexture,
                            (Vector2){entity->hitbox.x + (xCurrent * FLOOR_TILE_SIZE),
                                        entity->hitbox.y + (yCurrent * FLOOR_TILE_SIZE)},
                            0,
                            1,
                            WHITE
                        );
        }
    }
}
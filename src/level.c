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
    block1->spriteScale = FLOOR_TILE_SIZE;
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
    block2->spriteScale = FLOOR_TILE_SIZE;
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
    block3->spriteScale = FLOOR_TILE_SIZE;
    AddToEntityList(enemy1, block3);

    Entity *enemy2 = InitializeEnemy(block3,
                                        block3->hitbox.x + (block3->hitbox.width / 2),
                                        block3->hitbox.y);

    return enemy2;
}

#include <raylib.h>

#include "level.h"
#include "global.h"

#define FLOOR_TILE_SIZE 32

Texture2D floorTileTexture;

Level *LoadLevel() {
    Level *level = MemAlloc(sizeof(Level));

    level->tilesCount = 3;
    level->tiles = MemAlloc(sizeof(Rectangle) * level->tilesCount);
    
    int width = FLOOR_TILE_SIZE * 10;
    int x_spacing = width + (FLOOR_TILE_SIZE * 5);

    level->tiles[0].x       = 0;
    level->tiles[0].y       = FLOOR_HEIGHT;
    level->tiles[0].width   = width;
    level->tiles[0].height  = FLOOR_TILE_SIZE * 10;

    level->tiles[1].x       = x_spacing;
    level->tiles[1].y       = FLOOR_HEIGHT;
    level->tiles[1].width   = width;
    level->tiles[1].height  = FLOOR_TILE_SIZE * 10;

    level->tiles[2].x       = x_spacing * 2;
    level->tiles[2].y       = FLOOR_HEIGHT;
    level->tiles[2].width   = width;
    level->tiles[2].height  = FLOOR_TILE_SIZE * 10;

    return level;
}

void InitializeLevel(Level *level) {
    floorTileTexture = LoadTexture("../assets/floor_tile_1.png");
}

void DrawLevel(Level *level) {
    
    /*
        Tries to fill each of the level's tile with tiling floorTileTexture
    */
    for (int tileIdx = 0; tileIdx<level->tilesCount; tileIdx++) {

        Rectangle tile = level->tiles[tileIdx];

        // How many tiling textures to be drawn in each axis
        int xCount = tile.width / FLOOR_TILE_SIZE;
        int yCount = tile.height / FLOOR_TILE_SIZE;

        for (int xCurrent = 0; xCurrent < xCount; xCurrent++) {
            for (int yCurrent = 0; yCurrent < yCount; yCurrent++) {
                DrawTextureEx(
                                floorTileTexture,
                                (Vector2){tile.x + (xCurrent * FLOOR_TILE_SIZE), tile.y + (yCurrent * FLOOR_TILE_SIZE)},
                                0,
                                1,
                                WHITE
                            );
            }
        }
    }
}
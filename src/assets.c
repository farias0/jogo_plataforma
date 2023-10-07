#include "assets.h"
#include <raylib.h>
#include "string.h"

#define PLAYER_SPRITE_SCALE 2
#define ENEMY_SPRITE_SCALE 2
#define FLOOR_TILE_SIZE 2

/*
    TODO use hash map
*/

Sprite PlayerSprite;
Sprite EnemySprite;
Sprite BlockSprite;

void InitializeAssets() {
    PlayerSprite = (Sprite){
        LoadTexture("../assets/player_default_1.png"),
        PLAYER_SPRITE_SCALE
    };
    EnemySprite = (Sprite){
        LoadTexture("../assets/enemy_default_1.png"),
        ENEMY_SPRITE_SCALE
    };
    BlockSprite = (Sprite){
        LoadTexture("../assets/floor_tile_1.png"),
        FLOOR_TILE_SIZE
    };
}

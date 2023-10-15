#include "assets.h"
#include <raylib.h>
#include "string.h"

#define DEFAULT_SPRITE_SCALE 2
#define PLAYER_SPRITE_SCALE 2
#define ENEMY_SPRITE_SCALE 2
#define FLOOR_TILE_SIZE 1

/*
    TODO use hash map
*/

Sprite PlayerSprite;
Sprite EnemySprite;
Sprite BlockSprite;
Sprite EraserSprite;
Sprite LevelDotSprite;
Sprite OverworldCursorSprite;
Sprite PathTileJoinSprite;
Sprite PathTileStraightSprite;
Sprite NightclubSprite;


void InitializeAssets() {

    // Level
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
    EraserSprite = (Sprite){
        LoadTexture("../assets/eraser_1.png"),
        DEFAULT_SPRITE_SCALE
    };
    NightclubSprite = (Sprite){
        LoadTexture("../assets/nightclub_1.png"),
        1
    };

    // Overworld
    LevelDotSprite = (Sprite){
        LoadTexture("../assets/level_dot_1.png"),
        DEFAULT_SPRITE_SCALE
    };
    OverworldCursorSprite = (Sprite){
        LoadTexture("../assets/cursor_default_1.png"),
        DEFAULT_SPRITE_SCALE
    };
    PathTileJoinSprite = (Sprite){
        LoadTexture("../assets/path_tile_join_vertical.png"),
        DEFAULT_SPRITE_SCALE
    };
    PathTileStraightSprite = (Sprite){
        LoadTexture("../assets/path_tile_straight_vertical.png"),
        DEFAULT_SPRITE_SCALE
    };


    TraceLog(LOG_INFO, "Assets initialized.");
}

SpriteDimensions GetScaledDimensions(Sprite s) {
    return (SpriteDimensions) {
        s.sprite.width * s.scale,
        s.sprite.height * s.scale
    };
}
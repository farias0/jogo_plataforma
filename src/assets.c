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

// Editor
Sprite EraserSprite;

// In Level
Sprite PlayerSprite;
Sprite EnemySprite;
Sprite BlockSprite;

// Overworld
Sprite OverworldCursorSprite;
Sprite LevelDotSprite;
Sprite PathTileJoinSprite;
Sprite PathTileStraightSprite;
Sprite PathTileInLSprite;

// Background
Sprite NightclubSprite;
Sprite BGHouseSprite;


void InitializeAssets() {

    // Editor
    EraserSprite = (Sprite){
        LoadTexture("../assets/eraser_1.png"),
        DEFAULT_SPRITE_SCALE
    };

    // In Level
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

    // Overworld
    OverworldCursorSprite = (Sprite){
        LoadTexture("../assets/cursor_default_1.png"),
        DEFAULT_SPRITE_SCALE
    };
    LevelDotSprite = (Sprite){
        LoadTexture("../assets/level_dot_1.png"),
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
    PathTileInLSprite = (Sprite){
        LoadTexture("../assets/path_tile_L.png"),
        DEFAULT_SPRITE_SCALE
    };

    // Background
    NightclubSprite = (Sprite){
        LoadTexture("../assets/nightclub_2.png"),
        DEFAULT_SPRITE_SCALE
    };
    BGHouseSprite = (Sprite){
        LoadTexture("../assets/bg_house_1.png"),
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

void RotateSprite(Sprite *sprite, int degrees) {
    sprite->rotation += degrees;
    
    if (sprite->rotation >= 360 || sprite->rotation < 0) {
        sprite->rotation %= 360;
    }

    TraceLog(LOG_TRACE, "Rotated sprite in %d degrees. Now it's %d degrees.", degrees, sprite->rotation);
}

Rectangle GetSpritesHitboxFromEdge(Sprite sprite, Vector2 origin) {
    SpriteDimensions dimensions = GetScaledDimensions(sprite);
    return (Rectangle) {
        origin.x,
        origin.y,
        dimensions.width,
        dimensions.height
    };
}

Rectangle GetSpritesHitboxFromMiddle(Sprite sprite, Vector2 middlePoint) {
    SpriteDimensions dimensions = GetScaledDimensions(sprite);
    return (Rectangle) {
        middlePoint.x - (dimensions.width / 2),
        middlePoint.y - (dimensions.height / 2),
        dimensions.width,
        dimensions.height
    };
}
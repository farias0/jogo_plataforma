#include "assets.h"
#include <raylib.h>

#define DEFAULT_SPRITE_SCALE 2
#define FLOOR_TILE_SIZE 1


// Editor
Sprite EraserSprite;

// In Level
Sprite PlayerDefaultSprite;
Sprite PlayerGlideOnSprite;
Sprite PlayerGlideFallingSprite;
Sprite EnemySprite;
Sprite BlockSprite;
Sprite AcidSprite;
Sprite LevelEndOrbSprite;

// Overworld
Sprite OverworldCursorSprite;
Sprite LevelDotSprite;
Sprite PathTileJoinSprite;
Sprite PathTileStraightSprite;
Sprite PathTileInLSprite;

// Background
Sprite NightclubSprite;
Sprite BGHouseSprite;


// Shaders
Shader ShaderLevelTransition;


static inline Sprite defaultSprite(char *texturePath) {
    return (Sprite) {
        LoadTexture(texturePath),
        DEFAULT_SPRITE_SCALE,
        0
    };
}

static inline Sprite tileSprite(char *texturePath) {
    return (Sprite) {
        LoadTexture(texturePath),
        FLOOR_TILE_SIZE,
        0
    };
}

void AssetsInitialize() {

    // Editor
    EraserSprite = defaultSprite("../assets/eraser_1.png");

    // In Level
    PlayerDefaultSprite = defaultSprite("../assets/player_default_1.png");
    PlayerGlideOnSprite = defaultSprite("../assets/player_glide_on.png");
    PlayerGlideFallingSprite = defaultSprite("../assets/player_glide_falling.png");
    EnemySprite = defaultSprite("../assets/enemy_default_1.png");
    LevelEndOrbSprite = defaultSprite("../assets/level_end_orb_1.png");
    BlockSprite = tileSprite("../assets/floor_tile_1.png");
    AcidSprite = tileSprite("../assets/acid_tile_1.png");

    // Overworld
    OverworldCursorSprite = defaultSprite("../assets/cursor_default_1.png");
    LevelDotSprite = defaultSprite("../assets/level_dot_1.png");
    PathTileJoinSprite = defaultSprite("../assets/path_tile_join_vertical.png");
    PathTileStraightSprite = defaultSprite("../assets/path_tile_straight_vertical.png");
    PathTileInLSprite = defaultSprite("../assets/path_tile_L.png");

    // Background
    NightclubSprite = defaultSprite("../assets/nightclub_1.png");
    BGHouseSprite = defaultSprite("../assets/bg_house_1.png");

    // Shaders
    // ShaderDefault = (Shader) { rlGetShaderIdDefault(), rlGetShaderLocsDefault() };
    ShaderLevelTransition = LoadShader(0, "../assets/shaders/level_transition.fs");
    while (!IsShaderReady(ShaderLevelTransition)) {
        TraceLog(LOG_INFO, "Waiting for ShaderLevelTransition...");
    }


    TraceLog(LOG_INFO, "Assets initialized.");
}

Dimensions SpriteScaledDimensions(Sprite s) {
    return (Dimensions) {
        s.sprite.width * s.scale,
        s.sprite.height * s.scale
    };
}

Vector2 SpritePosMiddlePoint(Vector2 pos, Sprite sprite) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);

    return (Vector2) {
        pos.x + (dimensions.width / 2),
        pos.y + (dimensions.height / 2),
    };
}

void SpriteRotate(Sprite *sprite, int degrees) {
    sprite->rotation += degrees;
    
    if (sprite->rotation >= 360 || sprite->rotation < 0) {
        sprite->rotation %= 360;
    }

    TraceLog(LOG_TRACE, "Rotated sprite in %d degrees. Now it's %d degrees.", degrees, sprite->rotation);
}

Rectangle SpriteHitboxFromEdge(Sprite sprite, Vector2 origin) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);
    return (Rectangle) {
        origin.x,
        origin.y,
        dimensions.width,
        dimensions.height
    };
}

Rectangle SpriteHitboxFromMiddle(Sprite sprite, Vector2 middlePoint) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);
    return (Rectangle) {
        middlePoint.x - (dimensions.width / 2),
        middlePoint.y - (dimensions.height / 2),
        dimensions.width,
        dimensions.height
    };
}

void ShaderLevelTransitionSetUniforms(
    Vector2 resolution, Vector2 focusPoint, float duration, float currentTime, int isClose) {
    

    int resolutionLoc = GetShaderLocation(ShaderLevelTransition, "u_resolution");
    if (resolutionLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_resolution in ShaderLevelTransition");
        return;
    }
    SetShaderValue(ShaderLevelTransition, resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);

    int focusPointLoc = GetShaderLocation(ShaderLevelTransition, "u_focus_point");
    if (focusPointLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_focus_point in ShaderLevelTransition");
        return;
    }
    SetShaderValue(ShaderLevelTransition, focusPointLoc, &focusPoint, SHADER_UNIFORM_VEC2);

    int durationLoc = GetShaderLocation(ShaderLevelTransition, "u_duration");
    if (durationLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_duration in ShaderLevelTransition");
        return;
    }
    SetShaderValue(ShaderLevelTransition, durationLoc, &duration, SHADER_UNIFORM_FLOAT);

    int currentTimeLoc = GetShaderLocation(ShaderLevelTransition, "u_current_time");
    if (currentTimeLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_current_time in ShaderLevelTransition");
        return;
    }
    SetShaderValue(ShaderLevelTransition, currentTimeLoc, &currentTime, SHADER_UNIFORM_FLOAT);

    int isCloseLoc = GetShaderLocation(ShaderLevelTransition, "u_is_close");
    if (isCloseLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_is_close in ShaderLevelTransition");
        return;
    }
    SetShaderValue(ShaderLevelTransition, isCloseLoc, &isClose, SHADER_UNIFORM_INT);
}

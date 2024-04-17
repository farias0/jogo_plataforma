#include <raylib.h>
#include <string>

#include "assets.hpp"
#include "render.hpp"


struct SoundBank *SOUNDS = 0;

struct SpriteBank *SPRITES = 0;

// Shaders
Shader ShaderLevelTransition;


static inline Sprite normalSizeSprite(std::string texturePath) {
    return {
        LoadTexture(texturePath.c_str()),
        1,
        0
    };
}

static inline Sprite doubleSizeSprite(std::string texturePath) {
    return {
        LoadTexture(texturePath.c_str()),
        2,
        0
    };
}

static void clearAssets() {

    // TODO this is awful. Assets should exist in a hashmap.


    for (Sprite *s = (Sprite *) SPRITES;
            s < (Sprite *) SPRITES + sizeof(SpriteBank);
            s += sizeof(Sprite)) {

        UnloadTexture(s->sprite);
    }

    MemFree(SPRITES);

    TraceLog(LOG_INFO, "Sprites cleared.");


    for (Sound *s = (Sound *) SOUNDS;
            s < (Sound *) SOUNDS + sizeof(SoundBank);
            s += sizeof(Sound)) {
                
        UnloadSound(*s);
    }

    MemFree(SOUNDS);

    TraceLog(LOG_INFO, "Sounds cleared.");
}

void AssetsInitialize() {

    SPRITES = (SpriteBank *) MemAlloc(sizeof(SpriteBank));
    SpriteBank *sp = SPRITES;

    // Editor
    sp->Eraser = doubleSizeSprite("../assets/eraser_1.png");

    // In Level
    sp->PlayerDefault = doubleSizeSprite("../assets/player_default_1.png");
    sp->PlayerWalking1 = doubleSizeSprite("../assets/player_walking_1.png");
    sp->PlayerWalking2 = doubleSizeSprite("../assets/player_walking_2.png");
    sp->PlayerRunning1 = doubleSizeSprite("../assets/player_running_1.png");
    sp->PlayerRunning2 = doubleSizeSprite("../assets/player_running_2.png");
    sp->PlayerJumpingUp = doubleSizeSprite("../assets/player_jumping_up.png");
    sp->PlayerJumpingDown = doubleSizeSprite("../assets/player_jumping_down.png");
    sp->PlayerGlideOn = doubleSizeSprite("../assets/player_glide_on.png");
    sp->PlayerGlideFalling = doubleSizeSprite("../assets/player_glide_falling.png");
    sp->PlayerSwinging = doubleSizeSprite("../assets/player_swinging_1.png");
    sp->Enemy = doubleSizeSprite("../assets/enemy_default_1.png");
    sp->LevelEndOrb = doubleSizeSprite("../assets/level_end_orb_1.png");
    sp->LevelCheckpoint = doubleSizeSprite("../assets/player_child_1.png");
    sp->Block = normalSizeSprite("../assets/floor_tile_1.png");
    sp->Acid = normalSizeSprite("../assets/acid_tile_1.png");
    sp->GlideItem = normalSizeSprite("../assets/glide_item.png");
    sp->TextboxButton = normalSizeSprite("../assets/textbox_button.png");

    // Overworld
    sp->OverworldCursor = doubleSizeSprite("../assets/cursor_default_1.png");
    sp->LevelDot = doubleSizeSprite("../assets/level_dot_1.png");
    sp->PathTileJoin = doubleSizeSprite("../assets/path_tile_join_vertical.png");
    sp->PathTileStraight = doubleSizeSprite("../assets/path_tile_straight_vertical.png");
    sp->PathTileInL = doubleSizeSprite("../assets/path_tile_L.png");

    // Background
    sp->Nightclub = doubleSizeSprite("../assets/nightclub_1.png");
    sp->BGHouse = doubleSizeSprite("../assets/bg_house_1.png");



    SOUNDS = (SoundBank *) MemAlloc(sizeof(SoundBank));
    SoundBank *sn = SOUNDS;

    sn->Jump = LoadSound("../assets/sounds/jump.ogg");
    SetSoundVolume(sn->Jump, 0.5f);


    // Shaders
    // ShaderDefault = (Shader) { rlGetShaderIdDefault(), rlGetShaderLocsDefault() };
    ShaderLevelTransition = LoadShader(0, "../assets/shaders/level_transition.fs");
    while (!IsShaderReady(ShaderLevelTransition)) {
        TraceLog(LOG_INFO, "Waiting for ShaderLevelTransition...");
    }



    TraceLog(LOG_INFO, "Assets initialized.");
}

void AssetsReinitialize() {

    clearAssets();
    AssetsInitialize();
    Render::PrintSysMessage("Assets recarregados");
}

Dimensions SpriteScaledDimensions(Sprite *s) {
    return {
        s->sprite.width * s->scale,
        s->sprite.height * s->scale
    };
}

Vector2 SpritePosMiddlePoint(Vector2 pos, Sprite *sprite) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);

    return {
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

Rectangle SpriteHitboxFromEdge(Sprite *sprite, Vector2 origin) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);
    return {
        origin.x,
        origin.y,
        dimensions.width,
        dimensions.height
    };
}

Rectangle SpriteHitboxFromMiddle(Sprite *sprite, Vector2 middlePoint) {
    Dimensions dimensions = SpriteScaledDimensions(sprite);
    return {
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

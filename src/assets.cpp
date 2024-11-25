#include <raylib.h>
#include <string>

#include "assets.hpp"
#include "render.hpp"
#include "text_bank.hpp"
#include "level/textbox.hpp"


struct SoundBank *SOUNDS = 0;

struct SpriteBank *SPRITES = 0;

// Shaders
Shader ShaderLevelTransition;

Shader ShaderCRT;


static inline Sprite normalSizeSprite(std::string texturePath) {
    return {
        LoadTexture(texturePath.c_str()),
        1
    };
}

static inline Sprite doubleSizeSprite(std::string texturePath) {
    return {
        LoadTexture(texturePath.c_str()),
        2
    };
}

// Unload the resources loaded by each asset
static void unloadAssets() {

    // TODO this is awful. Assets should exist in a hashmap.


    Sprite *sprites = (Sprite *) SPRITES;
    for (int idx = 0; idx < (int) (sizeof(SpriteBank)/sizeof(Sprite)); idx++) {
        UnloadTexture(sprites[idx].sprite);
    }
    TraceLog(LOG_INFO, "Sprites unloaded.");


    Sound *sound = (Sound *) SOUNDS;
    for (int idx = 0; idx < (int) (sizeof(SoundBank)/sizeof(Sound)); idx++) {
        UnloadSound(sound[idx]);
    }
    TraceLog(LOG_INFO, "Sounds unloaded.");


    UnloadShader(ShaderLevelTransition);
    UnloadShader(ShaderCRT);
    TraceLog(LOG_INFO, "Shaders unloaded.");
}

// Load the resources for each asset
static void loadAssets() {

    SpriteBank *sp = SPRITES;

    // Editor
    sp->Eraser = doubleSizeSprite("../assets/eraser_1.png");

    // In Level
    sp->PlayerDefault = doubleSizeSprite("../assets/player_default_1.png");
    sp->PlayerWalking1 = doubleSizeSprite("../assets/player_walking_1.png");
    sp->PlayerWalking2 = doubleSizeSprite("../assets/player_walking_2.png");
    sp->PlayerRunning1 = doubleSizeSprite("../assets/player_running_1.png");
    sp->PlayerRunning2 = doubleSizeSprite("../assets/player_running_2.png");
    sp->PlayerSkidding = doubleSizeSprite("../assets/player_skidding_1.png");
    sp->PlayerJumpingUp = doubleSizeSprite("../assets/player_jumping_up.png");
    sp->PlayerJumpingDown = doubleSizeSprite("../assets/player_jumping_down.png");
    sp->PlayerGlideDefault1 = doubleSizeSprite("../assets/player_glide_default_1.png");
    sp->PlayerGlideDefault2 = doubleSizeSprite("../assets/player_glide_default_2.png");
    sp->PlayerGlideGliding1 = doubleSizeSprite("../assets/player_glide_gliding_1.png");
    sp->PlayerGlideGliding2 = doubleSizeSprite("../assets/player_glide_gliding_2.png");
    sp->PlayerSwinging = doubleSizeSprite("../assets/player_swinging_1.png");
    sp->PlayerSwingingForwards = doubleSizeSprite("../assets/player_swinging_forwards.png");
    sp->PlayerSwingingBackwards = doubleSizeSprite("../assets/player_swinging_backwards.png");
    sp->Enemy = doubleSizeSprite("../assets/enemy_default_1.png");
    sp->EnemyDummySpike = doubleSizeSprite("../assets/enemy_dummy_spike_1.png");
    sp->EnemyDummySpikePoppingOut1 = doubleSizeSprite("../assets/enemy_dummy_spike_popping_out_1.png");
    sp->EnemyDummySpikePoppingOut2 = doubleSizeSprite("../assets/enemy_dummy_spike_popping_out_2.png");
    sp->EnemyDummySpikePoppingOut3 = doubleSizeSprite("../assets/enemy_dummy_spike_popping_out_3.png");
    sp->EnemyDummySpikePoppedOut = doubleSizeSprite("../assets/enemy_dummy_spike_popped_out.png");
    sp->LevelEndOrb = doubleSizeSprite("../assets/level_end_orb_1.png");
    sp->LevelCheckpointFlag = doubleSizeSprite("../assets/player_child_1.png");
    sp->LevelCheckpointPickup1 = doubleSizeSprite("../assets/egg_1.png");
    sp->LevelCheckpointPickup2 = doubleSizeSprite("../assets/egg_2.png");
    sp->LevelCheckpointPickup3 = doubleSizeSprite("../assets/egg_3.png");
    sp->MovingPlatform = normalSizeSprite("../assets/moving_platform.png");
    sp->Block1Side = normalSizeSprite("../assets/floor_tile_1_side.png");
    sp->Block0Sides = normalSizeSprite("../assets/floor_tile_0_sides.png");
    sp->Block2SidesOpp = normalSizeSprite("../assets/floor_tile_2_sides_opposite.png");
    sp->Block2SidesAdj = normalSizeSprite("../assets/floor_tile_2_sides_adjacent.png");
    sp->Block3Sides = normalSizeSprite("../assets/floor_tile_3_sides.png");
    sp->Block4Sides = normalSizeSprite("../assets/floor_tile_4_sides.png");
    sp->Acid = normalSizeSprite("../assets/acid_tile_1.png");
    sp->GlideItem = normalSizeSprite("../assets/glide_item.png");
    sp->TextboxButton = normalSizeSprite("../assets/textbox_button.png");
    sp->TextboxDevButton = normalSizeSprite("../assets/textbox_dev_button.png");
    sp->TextboxButtonPlaying = normalSizeSprite("../assets/textbox_button_playing.png");
    sp->PrincessDefault1 = doubleSizeSprite("../assets/princess_default_1.png");
    sp->PrincessEditorIcon = doubleSizeSprite("../assets/princess_editor_icon.png");
    sp->Coin1 = normalSizeSprite("../assets/coin_1.png");
    sp->Coin2 = normalSizeSprite("../assets/coin_2.png");
    sp->Coin3 = normalSizeSprite("../assets/coin_3.png");

    // Overworld
    sp->OverworldCursor = doubleSizeSprite("../assets/cursor_default_1.png");
    sp->LevelDot = doubleSizeSprite("../assets/level_dot_1.png");
    sp->PathTileJoin = doubleSizeSprite("../assets/path_tile_join_vertical.png");
    sp->PathTileStraight = doubleSizeSprite("../assets/path_tile_straight_vertical.png");
    sp->PathTileInL = doubleSizeSprite("../assets/path_tile_L.png");

    // Background
    sp->Nightclub = doubleSizeSprite("../assets/nightclub_1.png");
    sp->BGHouse = doubleSizeSprite("../assets/bg_house_1.png");

    TraceLog(LOG_INFO, "Sprites loaded.");

    //

    SoundBank *sn = SOUNDS;

    sn->Jump = LoadSound("../assets/sounds/jump.ogg");
    SetSoundVolume(sn->Jump, 1.0f);

    sn->Track1 = LoadSound("../assets/sounds/track_1.wav");
    SetSoundVolume(sn->Track1, 0.6f);

    TraceLog(LOG_INFO, "Sounds loaded.");

    //

    /*
        !! ATTENTION !!  Each new shader added must be individually unloaded in unloadAssets()
    */

    // ShaderDefault = (Shader) { rlGetShaderIdDefault(), rlGetShaderLocsDefault() };

    ShaderLevelTransition = LoadShader(0, "../assets/shaders/level_transition.fs");
    while (!IsShaderReady(ShaderLevelTransition)) {
        TraceLog(LOG_INFO, "Waiting for ShaderLevelTransition...");
    }
    ShaderCRT = LoadShader(0, "../assets/shaders/crt.fs");
    while (!IsShaderReady(ShaderCRT)) {
        TraceLog(LOG_INFO, "Waiting for ShaderCRT...");
    }

    TraceLog(LOG_INFO, "Shaders loaded.");
}

void AssetsInitialize() {

    SPRITES = (SpriteBank *) MemAlloc(sizeof(SpriteBank));
    SOUNDS = (SoundBank *) MemAlloc(sizeof(SoundBank));

    loadAssets();

    TraceLog(LOG_INFO, "Assets initialized.");
}

void AssetsHotReload() {

    unloadAssets();
    loadAssets();

    TextBank::LoadFromDisk();

    Textbox::ReloadAllLevelTexboxes(); // probably shouldn't be here

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

void ShaderCrtSetUniforms() {

    int resolutionLoc = GetShaderLocation(ShaderCRT, "u_resolution");
    if (resolutionLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_resolution in ShaderCRT");
        return;
    }
    Vector2 res = { (float) GetScreenWidth(), (float) GetScreenHeight() };
    SetShaderValue(ShaderCRT, resolutionLoc, &res, SHADER_UNIFORM_VEC2);

    int timeLoc = GetShaderLocation(ShaderCRT, "u_time");
    if (timeLoc == -1) {
        TraceLog(LOG_ERROR, "Couldn't find location for uniform u_time in ShaderCRT");
        return;
    }
    double time = GetTime();
    SetShaderValue(ShaderCRT, timeLoc, &time, SHADER_UNIFORM_INT);
}

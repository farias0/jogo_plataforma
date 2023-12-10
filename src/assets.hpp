#ifndef _ASSETS_H_INCLUDED_
#define _ASSETS_H_INCLUDED_


#include <raylib.h>

#include "core.hpp"


typedef struct Sprite {
    Texture2D sprite;
    float scale;
    int rotation;
} Sprite;

typedef struct Sprites {

    // Editor
   Sprite Eraser;

    // In Level
   Sprite PlayerDefault;
   Sprite PlayerGlideOn;
   Sprite PlayerGlideFalling;
   Sprite Enemy;
   Sprite Block;
   Sprite Acid;
   Sprite LevelEndOrb;
   Sprite LevelCheckpoint;
   Sprite GlideItem;

    // Overworld
   Sprite OverworldCursor;
   Sprite LevelDot;
   Sprite PathTileJoin;
   Sprite PathTileStraight;
   Sprite PathTileInL;

    // Background
   Sprite Nightclub;
   Sprite BGHouse;

} Sprites;


// Editor
extern Sprites *SPRITES;

// Shaders
extern Shader ShaderLevelTransition;


void AssetsInitialize();

// Get a Sprite's dimensions, scaled
Dimensions SpriteScaledDimensions(Sprite sprite);

// Get a sprite's middle point given its position
Vector2 SpritePosMiddlePoint(Vector2 pos, Sprite sprite);

// Rotates sprite in a number of degrees
void SpriteRotate(Sprite *sprite, int degrees);

// Returns a hitbox in the shape of sprite.
Rectangle SpriteHitboxFromEdge(Sprite sprite, Vector2 origin);

// Returns a hitbox in the shape of sprite, centered around middlePoint.
Rectangle SpriteHitboxFromMiddle(Sprite sprite, Vector2 middlePoint);

/*
    Configures the uniforms ShaderLevelTransition will use in each execution.
    
    resolution: The resolution of the scene.
    focusPoint: The point in the scene that represents the focus of the effect.
    duration: The duration in seconds of the effect.
    currentTime: The current time, in seconds, since the start of the effect.
    isClose: If the effect is to close on the focusPoint (1), or to open from it (0). 
*/
void ShaderLevelTransitionSetUniforms(
    Vector2 resolution, Vector2 focusPoint, float duration, float currentTime, int isClose);


#endif // _ASSETS_H_INCLUDED_
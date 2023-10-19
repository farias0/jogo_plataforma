#ifndef _ASSETS_H_INCLUDED_
#define _ASSETS_H_INCLUDED_


#include <raylib.h>


typedef struct Sprite {
    Texture2D sprite;
    float scale;
    int rotation;
} Sprite;

typedef struct Dimensions {
    float width;
    float height;
} Dimensions;


// Editor
extern Sprite EraserSprite;

// In Level
extern Sprite PlayerSprite;
extern Sprite EnemySprite;
extern Sprite BlockSprite;

// Overworld
extern Sprite OverworldCursorSprite;
extern Sprite LevelDotSprite;
extern Sprite PathTileJoinSprite;
extern Sprite PathTileStraightSprite;
extern Sprite PathTileInLSprite;

// Background
extern Sprite NightclubSprite;
extern Sprite BGHouseSprite;


void AssetsInitialize();

// Get a Sprite's dimensions, scaled
Dimensions SpriteScaledDimensions(Sprite sprite);

// Rotates sprite in a number of degrees
void SpriteRotate(Sprite *sprite, int degrees);

// Returns a hitbox in the shape of sprite.
Rectangle SpriteHitboxFromEdge(Sprite sprite, Vector2 origin);

// Returns a hitbox in the shape of sprite, centered around middlePoint.
Rectangle SpriteHitboxFromMiddle(Sprite sprite, Vector2 middlePoint);


#endif // _ASSETS_H_INCLUDED_

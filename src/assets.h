#ifndef _ASSETS_H_INCLUDED_
#define _ASSETS_H_INCLUDED_


#include <raylib.h>


typedef struct Sprite {
    Texture2D sprite;
    float scale;
    int orientation; // In degrees
} Sprite;

typedef struct SpriteDimensions {
    float width;
    float height;
} SpriteDimensions;


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


void InitializeAssets();
// Get a Sprite's dimensions, scaled
SpriteDimensions GetScaledDimensions(Sprite sprite);

void RotateSprite(Sprite *sprite, int degrees);
void FlipSpriteHorizontally(Sprite *sprite);

// Returns a hitbox in the shape of sprite.
Rectangle GetSpritesHitboxFromEdge(Sprite sprite, Vector2 origin);

// Returns a hitbox in the shape of sprite, centered around middlePoint.
Rectangle GetSpritesHitboxFromMiddle(Sprite sprite, Vector2 middlePoint);

#endif // _ASSETS_H_INCLUDED_
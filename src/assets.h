#ifndef _ASSETS_H_INCLUDED_
#define _ASSETS_H_INCLUDED_


#include <raylib.h>


typedef struct Sprite {
    Texture2D sprite;
    float scale;
} Sprite;

typedef struct SpriteDimensions {
    float width;
    float height;
} SpriteDimensions;

extern Sprite PlayerSprite;
extern Sprite EnemySprite;
extern Sprite BlockSprite;
extern Sprite EraserSprite;
extern Sprite LevelDotSprite;
extern Sprite OverworldCursorSprite;
extern Sprite PathTileJoinSprite;
extern Sprite PathTileStraightSprite;
extern Sprite NightclubSprite;
extern Sprite BGHouseSprite;


void InitializeAssets();
// Get a Sprite's dimensions, scaled
SpriteDimensions GetScaledDimensions(Sprite sprite);

void RotateSprite(Sprite *sprite, int degrees);
void FlipSpriteHorizontally(Sprite *sprite);

#endif // _ASSETS_H_INCLUDED_
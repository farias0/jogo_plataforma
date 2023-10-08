#ifndef _ASSETS_H_INCLUDED_
#define _ASSETS_H_INCLUDED_


#include <raylib.h>


typedef struct Sprite {
    Texture2D sprite;
    float scale;
} Sprite;


extern Sprite PlayerSprite;
extern Sprite EnemySprite;
extern Sprite BlockSprite;

void InitializeAssets();



#endif // _ASSETS_H_INCLUDED_
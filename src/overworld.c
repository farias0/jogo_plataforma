#include <raylib.h>

#include "overworld.h"
#include "entities/entity.h"
#include "global.h"

/*
    Snaps a coordinate (x or y) into the grid of LevelDotSprites.
*/
float snapToOverworldGrid(float v) {
    
    // LevelDotSprite is square, so same for x and y.

    if (v >= 0) {
        return v - ((int) v % (int) LevelDotSprite.sprite.width);
    } else {
        return v - LevelDotSprite.sprite.width - ((int) v % (int) LevelDotSprite.sprite.width);
    }
}

void addDotToLevel(Vector2 pos) {

    Entity *newDot = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newDot->components = HasPosition +
                            HasSprite +
                            IsOverworldElement;
    newDot->hitbox = (Rectangle){
        pos.x,
        pos.y,
        LevelDotSprite.sprite.width,
        LevelDotSprite.sprite.height
    };
    newDot->sprite = LevelDotSprite;

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newDot);
}

void LoadOverworld() {

    float dotX = 200;
    float dotY = 200;

    addDotToLevel((Vector2){ dotX, dotY });
}
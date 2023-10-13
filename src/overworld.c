#include <raylib.h>

#include "overworld.h"
#include "entities/entity.h"
#include "global.h"

typedef enum PathTileType {
    STRAIGHT,
    JOIN,
} PathTileType;


Sprite rotateSprite(Sprite s, int degrees) {
    Image img = LoadImageFromTexture(s.sprite);
    ImageRotate(&img, degrees);
    s.sprite = LoadTextureFromImage(img); 
    return s;
}

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

// Calculates the position for the cursor when it's standing over a given dot
Vector2 calculateCursorPosition(Vector2 dot) {

    return (Vector2){
        dot.x,
        dot.y +
            (LevelDotSprite.sprite.height * LevelDotSprite.scale / 2) -
            (OverworldCursorSprite.sprite.height * OverworldCursorSprite.scale)
    };
}

void initializeOverworldCursor(Vector2 pos) {
    Entity *newCursor = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newCursor->components = HasPosition +
                            HasSprite +
                            IsOverworldElement +
                            IsCursor;
    newCursor->hitbox = (Rectangle){
        pos.x,
        pos.y,
        OverworldCursorSprite.sprite.width,
        OverworldCursorSprite.sprite.height
    };
    newCursor->sprite = OverworldCursorSprite;

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newCursor);
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

void addPathTileToLevel(Vector2 pos, PathTileType type, int degrees) {

    Entity *newPathTile = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newPathTile->components = HasPosition +
                            HasSprite +
                            IsOverworldElement;

    Sprite sprite;
    switch (type)
    {
    case STRAIGHT:
        sprite = PathTileStraightSprite; break;
    case JOIN:
        sprite = PathTileJoinSprite; break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for path tile type %d.", type);
    }

    newPathTile->hitbox = (Rectangle){
        pos.x,
        pos.y,
        sprite.sprite.width,
        sprite.sprite.height
    };
    newPathTile->sprite = rotateSprite(sprite, degrees);

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newPathTile);
}

void LoadOverworld() {

    float dotX = SCREEN_WIDTH/2;
    float dotY = SCREEN_HEIGHT/2;

    addDotToLevel((Vector2){ dotX, dotY });
    addPathTileToLevel((Vector2){ dotX + (LevelDotSprite.sprite.width * LevelDotSprite.scale), dotY }, JOIN, 90);
    addPathTileToLevel((Vector2){ dotX + (LevelDotSprite.sprite.width * LevelDotSprite.scale) * 2, dotY }, STRAIGHT, 90);
    addPathTileToLevel((Vector2){ dotX + (LevelDotSprite.sprite.width * LevelDotSprite.scale) * 3, dotY }, JOIN, 270);
    addDotToLevel((Vector2){ dotX + (LevelDotSprite.sprite.width * LevelDotSprite.scale) * 4, dotY });

    // ATTENTION: Cursor is initialized at the end so it's rendered in front of the other entities
    // TODO fix this hack
    initializeOverworldCursor(calculateCursorPosition((Vector2){ dotX, dotY }));
}

void SelectLevel() {

    InitializeLevel(); 
}
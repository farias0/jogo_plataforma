#include <raylib.h>
#include <stdlib.h>

#include "overworld.h"
#include "entities/entity.h"
#include "global.h"

typedef enum PathTileType {
    STRAIGHT,
    JOIN,
} PathTileType;

typedef struct CursorState {
    Entity *cursor;

    // The tile the cursor is over
    Entity *tileUnder;
} CursorState;


CursorState cursorState;

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

// Updates the position for the cursor according to the tile under it
void updateCursorPosition() {

    SpriteDimensions cursorDimensions = GetScaledDimensions(cursorState.cursor->sprite);
    SpriteDimensions tileUnderDimensions = GetScaledDimensions(cursorState.tileUnder->sprite);

    cursorState.cursor->hitbox.x = cursorState.tileUnder->hitbox.x;

    cursorState.cursor->hitbox.y = cursorState.tileUnder->hitbox.y +
                    (tileUnderDimensions.height / 2) -
                    cursorDimensions.height;
}

void initializeOverworldCursor(Vector2 pos) {
    Entity *newCursor = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newCursor->components = HasPosition +
                            HasSprite +
                            IsOverworldElement +
                            IsCursor;
    newCursor->hitbox = GetSpritesHitboxFromEdge(OverworldCursorSprite, pos);
    newCursor->sprite = OverworldCursorSprite;

    ENTITIES_HEAD = AddToEntityList(ENTITIES_HEAD, newCursor);
    cursorState.cursor = newCursor;
}

Entity *addDotToLevel(Vector2 pos) {

    Entity *newDot = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newDot->components = HasPosition +
                            HasSprite +
                            IsOverworldElement +
                            IsLevelDot;
    newDot->hitbox = GetSpritesHitboxFromEdge(LevelDotSprite, pos);
    newDot->sprite = LevelDotSprite;

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newDot);

    return newDot;
}

Entity *addPathTileToLevel(Vector2 pos, PathTileType type, int degrees) {

    Entity *newPathTile = MemAlloc(sizeof(Entity));

    pos.x = snapToOverworldGrid(pos.x);
    pos.y = snapToOverworldGrid(pos.y);

    newPathTile->components = HasPosition +
                            HasSprite +
                            IsOverworldElement;

    switch (type)
    {
    case STRAIGHT:
        newPathTile->sprite = PathTileStraightSprite;
        newPathTile->hitbox = GetSpritesHitboxFromEdge(PathTileStraightSprite, pos);
        break;
    case JOIN:
        newPathTile->sprite = PathTileJoinSprite;
        newPathTile->hitbox = GetSpritesHitboxFromEdge(PathTileJoinSprite, pos);
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for path tile type %d.", type);
    }

    RotateSprite(&newPathTile->sprite, degrees);

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newPathTile);

    return newPathTile;
}

void LoadOverworld() {

    // ATTENTION: Using dot sprite dimension to all tilings
    SpriteDimensions tileDimension = GetScaledDimensions(LevelDotSprite);

    float dotX = SCREEN_WIDTH/2;
    float dotY = SCREEN_HEIGHT/2;

    Entity *dot1 = addDotToLevel((Vector2){ dotX, dotY });

    // Path to the right
    Entity *path1   = addPathTileToLevel    ((Vector2){ dotX + tileDimension.width,     dotY }, JOIN,       90);
    Entity *path2   = addPathTileToLevel    ((Vector2){ dotX + tileDimension.width * 2, dotY }, STRAIGHT,   90);
    Entity *path3   = addPathTileToLevel    ((Vector2){ dotX + tileDimension.width * 3, dotY }, JOIN,       270);
    Entity *dot2    = addDotToLevel         ((Vector2){ dotX + tileDimension.width * 4, dotY });

    // Path up
    Entity *path4   = addPathTileToLevel    ((Vector2){ dotX,   dotY - tileDimension.height },      JOIN,       180);
    Entity *path5   = addPathTileToLevel    ((Vector2){ dotX,   dotY - tileDimension.height * 2},   STRAIGHT,   0);
    Entity *path6   = addPathTileToLevel    ((Vector2){ dotX,   dotY - tileDimension.height * 3},   JOIN,       0);
    Entity *dot3    = addDotToLevel         ((Vector2){ dotX,   dotY - tileDimension.height * 4});

    // ATTENTION: Cursor is initialized at the end so it's rendered in front of the other entities
    // TODO fix this hack
    initializeOverworldCursor((Vector2){ 0, 0 });
    cursorState.tileUnder = dot1;
    updateCursorPosition();
}

void SelectLevel() {

    if (cursorState.tileUnder->components & IsLevelDot) {
        InitializeLevel();
    }
}

void OverworldMoveCursor(OverworldCursorDirection direction) {

    TraceLog(LOG_TRACE, "Overworld move to direction %d", direction);


    SpriteDimensions tileUnderDimensions = GetScaledDimensions(cursorState.tileUnder->sprite);
    SpriteDimensions currentItemDimensions;


    Entity *currentItem = ENTITIES_HEAD;
    while (currentItem != 0) {
        

        currentItemDimensions = GetScaledDimensions(currentItem->sprite);


        bool isTile = currentItem->components & IsOverworldElement &&
                        !(currentItem->components & IsCursor);
        if (!isTile) goto next_entity;


        bool isOnTheSameRow = cursorState.tileUnder->hitbox.y == currentItem->hitbox.y;
        bool isOnTheSameColumn = cursorState.tileUnder->hitbox.x == currentItem->hitbox.x;
        bool foundPath = false;


        // This code is stupid, but I'm leaving it for sake of simplicity and ease of debug.
        switch(direction)
        {
        case UP:
            if (isOnTheSameColumn &&
                cursorState.tileUnder->hitbox.y - currentItemDimensions.height == currentItem->hitbox.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path up.");
                
                }
                break;
        case DOWN:
            if (isOnTheSameColumn &&
                cursorState.tileUnder->hitbox.y + tileUnderDimensions.height == currentItem->hitbox.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path down.");
                
                }
                break;
        case LEFT:
            if (isOnTheSameRow &&
                cursorState.tileUnder->hitbox.x - currentItemDimensions.width == currentItem->hitbox.x) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path left.");
                
                }
                break;
        case RIGHT:
            if (isOnTheSameRow &&
                cursorState.tileUnder->hitbox.x + tileUnderDimensions.width == currentItem->hitbox.x) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path right.");
                
                }
                break;
        default:
            TraceLog(LOG_ERROR, "No code to handle move overworld cursor to direction %d.", direction);
            return;
        }


        if (foundPath) {
            cursorState.tileUnder = currentItem;
            updateCursorPosition();
            break;
        }

next_entity:
        currentItem = currentItem->next;
    }
}

void AddDotToLevel(Vector2 pos) {

    // Rectangle hitbox = GetSpritesHitboxFromMiddle(LevelDotSprite, pos);

    // Entity *possibleTile = ENTITIES_HEAD;

    // while (possibleTile != 0) {
        
    //     if (possibleTile->components & IsOverworldElement &&
    //             !(possibleTile->components & IsCursor) &&
    //             CheckCollisionPointRec(pos, possibleTile->hitbox)) {

    //         return ENTITIES_HEAD;
    //     }

    //     possibleBlock = possibleBlock->next;

    // }

    // return addBlockToLevel(ENTITIES_HEAD, (Rectangle){ pos.x, pos.y, BlockSprite.sprite.width, BlockSprite.sprite.height });
}
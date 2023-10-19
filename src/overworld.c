#include <raylib.h>
#include <stdlib.h>
#include "math.h"

#include "overworld.h"
#include "entities/entity.h"
#include "global.h"
#include "assets.h"


typedef struct CursorState {
    Entity *cursor;

    // The tile the cursor is over
    Entity *tileUnder;
} CursorState;


SpriteDimensions OverworldGridDimensions = (SpriteDimensions){
    // Based on a dot tile
    64,
    64
};

CursorState cursorState;


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

    pos.x = SnapToGrid(pos.x, OverworldGridDimensions.width);
    pos.y = SnapToGrid(pos.y, OverworldGridDimensions.height);

    newCursor->components = HasPosition +
                            HasSprite +
                            IsOverworldElement +
                            IsCursor;
    newCursor->hitbox = GetSpritesHitboxFromEdge(OverworldCursorSprite, pos);
    newCursor->sprite = OverworldCursorSprite;
    newCursor->layer = 1;

    ENTITIES_HEAD = AddToEntityList(ENTITIES_HEAD, newCursor);
    cursorState.cursor = newCursor;
}

Entity *addTileToOverworld(Vector2 pos, OverworldTileType type, int degrees) {

    Entity *newTile = MemAlloc(sizeof(Entity));

    pos.x = SnapToGrid(pos.x, OverworldGridDimensions.width);
    pos.y = SnapToGrid(pos.y, OverworldGridDimensions.height);

    newTile->components = HasPosition +
                            HasSprite +
                            IsOverworldElement;
    if (type == LEVEL_DOT) newTile->components += IsLevelDot;

    switch (type)
    {
    case LEVEL_DOT:
        newTile->sprite = LevelDotSprite;
        newTile->hitbox = GetSpritesHitboxFromEdge(LevelDotSprite, pos);
        break;
    case STRAIGHT_PATH:
        newTile->sprite = PathTileStraightSprite;
        newTile->hitbox = GetSpritesHitboxFromEdge(PathTileStraightSprite, pos);
        RotateSprite(&newTile->sprite, degrees);
        break;
    case JOIN_PATH:
        newTile->sprite = PathTileJoinSprite;
        newTile->hitbox = GetSpritesHitboxFromEdge(PathTileJoinSprite, pos);
        RotateSprite(&newTile->sprite, degrees);
        break;
    case PATH_IN_L:
        newTile->sprite = PathTileInLSprite;
        newTile->hitbox = GetSpritesHitboxFromEdge(PathTileInLSprite, pos);
        RotateSprite(&newTile->sprite, degrees);
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for overworld tile type %d.", type);
    }

    ENTITIES_HEAD =  AddToEntityList(ENTITIES_HEAD, newTile);

    return newTile;
}

void LoadOverworld() {

    // ATTENTION: Using dot sprite dimension to all tilings
    SpriteDimensions tileDimension = GetScaledDimensions(LevelDotSprite);

    float dotX = SCREEN_WIDTH/2;
    float dotY = SCREEN_HEIGHT/2;

    initializeOverworldCursor((Vector2){ 0, 0 });


    Entity *dot1    = addTileToOverworld    ((Vector2){ dotX, dotY },                               LEVEL_DOT,      0);

    // Path to the right
    Entity *path1   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width,     dotY },     JOIN_PATH,      270);
    Entity *path2   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 2, dotY },     STRAIGHT_PATH,  90);
    Entity *path3   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 3, dotY },     JOIN_PATH,      90);
    Entity *dot2    = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 4, dotY },     LEVEL_DOT,      0);

    // Path up
    Entity *path4   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height },      JOIN_PATH,      180);
    Entity *path5   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 2},   STRAIGHT_PATH,  0);
    Entity *path6   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 3},   JOIN_PATH,      0);
    Entity *dot3    = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 4},   LEVEL_DOT,      0);


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


    Entity *currentItem = ENTITIES_HEAD;
    while (currentItem != 0) {

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
                cursorState.tileUnder->hitbox.y - currentItem->hitbox.height == currentItem->hitbox.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path up.");
                
                }
                break;
        case DOWN:
            if (isOnTheSameColumn &&
                cursorState.tileUnder->hitbox.y + currentItem->hitbox.height == currentItem->hitbox.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path down.");
                
                }
                break;
        case LEFT:
            if (isOnTheSameRow &&
                cursorState.tileUnder->hitbox.x - currentItem->hitbox.width == currentItem->hitbox.x) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path left.");
                
                }
                break;
        case RIGHT:
            if (isOnTheSameRow &&
                cursorState.tileUnder->hitbox.x + currentItem->hitbox.width == currentItem->hitbox.x) {

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

void AddTileToOverworld(Vector2 pos) {

    SpriteDimensions dimensions = GetScaledDimensions(PathTileStraightSprite);
    Rectangle hitbox = (Rectangle){ SnapToGrid(pos.x, OverworldGridDimensions.width),
                                    SnapToGrid(pos.y, OverworldGridDimensions.height),
                                    dimensions.width,
                                    dimensions.height };

    Entity *possibleTile = ENTITIES_HEAD;

    while (possibleTile != 0) {
        
        if (possibleTile->components & IsOverworldElement &&
                !(possibleTile->components & IsCursor) &&
                CheckCollisionRecs(hitbox, possibleTile->hitbox)) {

                if (!(possibleTile->components & IsLevelDot)) {

                    RotateSprite(&possibleTile->sprite, 90);

                    TraceLog(LOG_TRACE, "Rotated tile component=%d, x=%.1f, y=%.1f",
                            possibleTile->components, possibleTile->hitbox.x, possibleTile->hitbox.y);
                            
                    return;
                }

                TraceLog(LOG_TRACE, "Couldn't place tile, collided with item component=%d, x=%.1f, y=%.1f",
                            possibleTile->components, possibleTile->hitbox.x, possibleTile->hitbox.y);

            return;
        }

        possibleTile = possibleTile->next;

    }

    switch (STATE->editorSelectedItem->type) {
        case LevelDot:
            addTileToOverworld((Vector2){ hitbox.x, hitbox.y }, LEVEL_DOT, 0); break;
        case PathStraight:
            addTileToOverworld((Vector2){ hitbox.x, hitbox.y }, STRAIGHT_PATH, 0); break;
        case PathJoin:
            addTileToOverworld((Vector2){ hitbox.x, hitbox.y }, JOIN_PATH, 0); break;
        case PathInL:
            addTileToOverworld((Vector2){ hitbox.x, hitbox.y }, PATH_IN_L, 0); break;
        default:
            TraceLog(LOG_ERROR,
                        "Couldn't find Overworld Tile Type for Editor Item Type %d.",
                        STATE->editorSelectedItem->type);
    }
}

void RemoveTileFromOverWorld(Vector2 pos) {

    Entity *entity = GetEntityOn(pos);

    if (!entity) {

        TraceLog(LOG_DEBUG, "Didn't find any tile.");
        return;
    }

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (entity == cursorState.tileUnder) {

            TraceLog(LOG_DEBUG, "Won't remove tile, it's under the cursor.");
            return;
        }

    ENTITIES_HEAD = DestroyEntity(entity);
}
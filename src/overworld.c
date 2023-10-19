#include <raylib.h>
#include <stdlib.h>
#include "math.h"

#include "overworld.h"
#include "core.h"
#include "assets.h"
#include "camera.h"


typedef struct CursorState {
    OverworldEntity *cursor;

    // The tile the cursor is over
    OverworldEntity *tileUnder;
} CursorState;


ListNode *OW_LIST_HEAD = 0;
const Dimensions OW_GRID = (Dimensions){ 64, 64 };

static OverworldEntity *OW_CURSOR = 0;
static CursorState CURSOR_STATE;


static Rectangle getGridSquare(OverworldEntity *entity) {
    return (Rectangle) {
        entity->gridPos.x,
        entity->gridPos.y,
        OW_GRID.width,
        OW_GRID.height,
    };
}

// Updates the position for the cursor according to the tile under it
static void updateCursorPosition() {

    Dimensions cursorDimensions = GetScaledDimensions(OverworldCursorSprite);

    OW_CURSOR->gridPos.x = CURSOR_STATE.tileUnder->gridPos.x;

    OW_CURSOR->gridPos.y = CURSOR_STATE.tileUnder->gridPos.y +
                    (OW_GRID.height / 2) -
                    cursorDimensions.height;
}

static void initializeCursor() {

    OverworldEntity *newCursor = MemAlloc(sizeof(OverworldEntity));

    newCursor->components = OW_IS_CURSOR;
    newCursor->sprite = OverworldCursorSprite;
    newCursor->layer = 1;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newCursor;
    LinkedListAdd(&OW_LIST_HEAD, node);

    OW_CURSOR = newCursor;
    CURSOR_STATE.cursor = OW_CURSOR;

    TraceLog(LOG_INFO, "Added cursor to overworld (x=%.1f, y=%.1f)",
                newCursor->gridPos.x, newCursor->gridPos.y);
}

static OverworldEntity *addTileToOverworld(Vector2 pos, OverworldTileType type, int degrees) {

    OverworldEntity *newTile = MemAlloc(sizeof(OverworldEntity));

    pos.x = SnapToGrid(pos.x, OW_GRID.width);
    pos.y = SnapToGrid(pos.y, OW_GRID.height);

    newTile->tileType = type;
    newTile->gridPos = pos;

    switch (newTile->tileType)
    {
    case OW_LEVEL_DOT:
        newTile->components = OW_IS_LEVEL_DOT;
        newTile->sprite = LevelDotSprite;
        break;
    case OW_STRAIGHT_PATH:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileStraightSprite;
        RotateSprite(&newTile->sprite, degrees);
        break;
    case OW_JOIN_PATH:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileJoinSprite;
        RotateSprite(&newTile->sprite, degrees);
        break;
    case OW_PATH_IN_L:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileInLSprite;
        RotateSprite(&newTile->sprite, degrees);
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for overworld tile type %d.", type);
    }

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newTile;
    LinkedListAdd(&OW_LIST_HEAD, node);

    TraceLog(LOG_DEBUG, "Added tile to overworld (x=%.1f, y=%.1f)",
                newTile->gridPos.x, newTile->gridPos.y);

    return newTile;
}

// Searches for an entity that's not the cursor
// in a given position and returns its node, or 0 if not found.
static ListNode *getNodeOfEntityOn(Vector2 pos) {

    ListNode *node = OW_LIST_HEAD;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (!(entity->components & OW_IS_CURSOR) &&
            CheckCollisionPointRec(pos, getGridSquare(entity))) {

                return node;
            }

        node = node->next;
    };

    return 0;
}

static void overworldLoad() {

    // ATTENTION: Using dot sprite dimension to all tilings
    Dimensions tileDimension = GetScaledDimensions(LevelDotSprite);

    float dotX = SCREEN_WIDTH/2;
    float dotY = SCREEN_HEIGHT/2;


    OverworldEntity *dot1    = addTileToOverworld    ((Vector2){ dotX, dotY },                               OW_LEVEL_DOT,      0);

    // Path to the right
    OverworldEntity *path1   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width,     dotY },     OW_JOIN_PATH,      270);
    OverworldEntity *path2   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 2, dotY },     OW_STRAIGHT_PATH,  90);
    OverworldEntity *path3   = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 3, dotY },     OW_JOIN_PATH,      90);
    OverworldEntity *dot2    = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 4, dotY },     OW_LEVEL_DOT,      0);

    // Path up
    OverworldEntity *path4   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height },      OW_JOIN_PATH,      180);
    OverworldEntity *path5   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 2},   OW_STRAIGHT_PATH,  0);
    OverworldEntity *path6   = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 3},   OW_JOIN_PATH,      0);
    OverworldEntity *dot3    = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 4},   OW_LEVEL_DOT,      0);


    CURSOR_STATE.tileUnder = dot1;

    TraceLog(LOG_INFO, "Overworld loaded.");
}

void OverworldInitialize() {

    GameStateReset();
    STATE->mode = MODE_OVERWORLD;

    LinkedListRemoveAll(&OW_LIST_HEAD);

    initializeCursor();
    overworldLoad();
    updateCursorPosition();

    EditorSync();

    TraceLog(LOG_INFO, "Overworld initialized.");
}

void OverworldLevelSelect() {

    if (CURSOR_STATE.tileUnder->components & OW_IS_LEVEL_DOT) {
        LevelInitialize();
    }
}

void OverworldCursorMove(OverworldCursorDirection direction) {

    TraceLog(LOG_TRACE, "Overworld move to direction %d", direction);

    OverworldEntity *tileUnder = CURSOR_STATE.tileUnder;

    ListNode *node = OW_LIST_HEAD;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        bool isTile = (entity->components & OW_IS_PATH) ||
                        (entity->components & OW_IS_LEVEL_DOT);
        if (!isTile) goto next_entity;


        bool isOnTheSameRow = tileUnder->gridPos.y == entity->gridPos.y;
        bool isOnTheSameColumn = tileUnder->gridPos.x == entity->gridPos.x;
        bool foundPath = false;


        // This code is stupid, but I'm leaving it for sake of simplicity and ease of debug.
        switch(direction)
        {
        case OW_CURSOR_UP:
            if (isOnTheSameColumn &&
                tileUnder->gridPos.y - OW_GRID.height == entity->gridPos.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path up.");
                
                }
                break;
        case OW_CURSOR_DOWN:
            if (isOnTheSameColumn &&
                tileUnder->gridPos.y + OW_GRID.height == entity->gridPos.y) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path down.");
                
                }
                break;
        case OW_CURSOR_LEFT:
            if (isOnTheSameRow &&
                tileUnder->gridPos.x - OW_GRID.width == entity->gridPos.x) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path left.");
                
                }
                break;
        case OW_CURSOR_RIGHT:
            if (isOnTheSameRow &&
                tileUnder->gridPos.x + OW_GRID.width == entity->gridPos.x) {

                    foundPath = true;
                    TraceLog(LOG_TRACE, "Found path right.");
                
                }
                break;
        default:
            TraceLog(LOG_ERROR, "No code to handle move overworld cursor to direction %d.", direction);
            return;
        }


        if (foundPath) {
            CURSOR_STATE.tileUnder = entity;
            updateCursorPosition();
            break;
        }

next_entity:
        node = node->next;
    }
}

void OverworldTileAddOrInteract(Vector2 pos) {

    Dimensions testDimensions = GetScaledDimensions(PathTileStraightSprite);
    Rectangle testHitbox = (Rectangle){ SnapToGrid(pos.x, OW_GRID.width),
                                    SnapToGrid(pos.y, OW_GRID.height),
                                    OW_GRID.width,
                                    OW_GRID.width };

    ListNode *node = OW_LIST_HEAD;

    // First, test collision with other tiles
    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (entity->components & OW_IS_CURSOR) goto next_entity;

        if (!CheckCollisionRecs(testHitbox, getGridSquare(entity))) goto next_entity;

        if (entity->components & OW_IS_LEVEL_DOT) {
            TraceLog(LOG_DEBUG, "Couldn't place tile, collided with item component=%d, x=%.1f, y=%.1f",
                            entity->components, entity->gridPos.x, entity->gridPos.y);
            return;
        }

        RotateSprite(&entity->sprite, 90);
        TraceLog(LOG_DEBUG, "Rotated tile component=%d, x=%.1f, y=%.1f",
                entity->components, entity->gridPos.x, entity->gridPos.y);
        return;

next_entity:
        node = node->next;
    }

    OverworldTileType typeToAdd;

    switch (STATE->editorSelectedItem->type) {
        case LevelDot:
            typeToAdd = OW_LEVEL_DOT; break;
        case PathStraight:
            typeToAdd = OW_STRAIGHT_PATH; break;
        case PathJoin:
            typeToAdd = OW_JOIN_PATH; break;
        case PathInL:
            typeToAdd = OW_PATH_IN_L; break;
        default:
            TraceLog(LOG_ERROR,
                        "Couldn't find Overworld Tile Type for Editor Item Type %d.",
                        STATE->editorSelectedItem->type);
            return;
    }

    addTileToOverworld(pos, typeToAdd, 0);

    TraceLog(LOG_DEBUG, "Added tile of type %d to the overworld (x=%.1f, y=%.1f).",
                typeToAdd, pos.x, pos.y);
}

void OverworldTileRemoveAt(Vector2 pos) {

    ListNode *node = getNodeOfEntityOn(pos);

    if (!node) {

        TraceLog(LOG_DEBUG, "Didn't find any tile to remove.");
        return;
    }

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (node->item == CURSOR_STATE.tileUnder) {
        TraceLog(LOG_DEBUG, "Won't remove tile, it's under the cursor.");
        return;
    }

    LinkedListRemove(&OW_LIST_HEAD, node);
    TraceLog(LOG_DEBUG, "Removed overworld tile.");
}

void OverworldTick() {

    CameraTick();    
}

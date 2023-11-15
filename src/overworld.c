#include <raylib.h>
#include <string.h>

#include "overworld.h"
#include "core.h"
#include "assets.h"
#include "camera.h"
#include "render.h"
#include "persistence.h"


ListNode *OW_LIST_HEAD = 0;

double overworldLevelSelectedAgo = -1;


static OverworldEntity *OW_CURSOR = 0;

static char *overworldLevelSelectedName = 0;


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

    Dimensions cursorDimensions = SpriteScaledDimensions(OverworldCursorSprite);

    OW_CURSOR->gridPos.x = STATE->tileUnderCursor->gridPos.x;

    OW_CURSOR->gridPos.y = STATE->tileUnderCursor->gridPos.y +
                    (OW_GRID.height / 2) -
                    cursorDimensions.height;
}

static void destroyEntityOverworld(ListNode *node) {

    OverworldEntity *entity = (OverworldEntity *) node->item;
    MemFree(entity->levelName);

    LinkedListRemove(&OW_LIST_HEAD, node);

    TraceLog(LOG_TRACE, "Destroyed overworld entity.");
}

static void initializeCursor() {

    OverworldEntity *newCursor = MemAlloc(sizeof(OverworldEntity));

    newCursor->components = OW_IS_CURSOR;
    newCursor->sprite = OverworldCursorSprite;
    newCursor->layer = 1;

    LinkedListAdd(&OW_LIST_HEAD, newCursor);

    OW_CURSOR = newCursor;

    TraceLog(LOG_TRACE, "Added cursor to overworld (x=%.1f, y=%.1f)",
                newCursor->gridPos.x, newCursor->gridPos.y);
}

static OverworldEntity *addTileToOverworld(Vector2 pos, OverworldTileType type, int degrees) {

    OverworldEntity *newTile = MemAlloc(sizeof(OverworldEntity));

    pos.x = SnapToGrid(pos.x, OW_GRID.width);
    pos.y = SnapToGrid(pos.y, OW_GRID.height);

    newTile->tileType = type;
    newTile->gridPos = pos;
    newTile->levelName = MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);

    switch (newTile->tileType)
    {
    case OW_LEVEL_DOT:
        newTile->components = OW_IS_LEVEL_DOT;
        newTile->sprite = LevelDotSprite;
        break;
    case OW_STRAIGHT_PATH:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileStraightSprite;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_JOIN_PATH:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileJoinSprite;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_PATH_IN_L:
        newTile->components = OW_IS_PATH;
        newTile->sprite = PathTileInLSprite;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for overworld tile type %d.", type);
    }

    LinkedListAdd(&OW_LIST_HEAD, newTile);

    TraceLog(LOG_TRACE, "Added tile to overworld (x=%.1f, y=%.1f)",
                newTile->gridPos.x, newTile->gridPos.y);

    return newTile;
}

// Searches for an entity that's not the cursor
// in a given position and returns its node, or 0 if not found.
static ListNode *getEntityOnScene(Vector2 pos) {

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
    Dimensions tileDimension = SpriteScaledDimensions(LevelDotSprite);

    float dotX = SCREEN_WIDTH/2;
    float dotY = SCREEN_HEIGHT/2;


    OverworldEntity *dot1    = addTileToOverworld    ((Vector2){ dotX, dotY },                               OW_LEVEL_DOT,      0);

    // Path to the right
    addTileToOverworld    ((Vector2){ dotX + tileDimension.width,     dotY },     OW_JOIN_PATH,      270);
    addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 2, dotY },     OW_STRAIGHT_PATH,  90);
    addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 3, dotY },     OW_JOIN_PATH,      90);
    OverworldEntity *dot2    = addTileToOverworld    ((Vector2){ dotX + tileDimension.width * 4, dotY },     OW_LEVEL_DOT,      0);

    // Path up
    addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height },      OW_JOIN_PATH,      180);
    addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 2},   OW_STRAIGHT_PATH,  0);
    addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 3},   OW_JOIN_PATH,      0);
    OverworldEntity *dot3    = addTileToOverworld    ((Vector2){ dotX,   dotY - tileDimension.height * 4},   OW_LEVEL_DOT,      0);


    strcpy(dot1->levelName, "level_1.lvl");
    strcpy(dot2->levelName, "level_2.lvl");
    strcpy(dot3->levelName, "level_3.lvl");
    

    STATE->tileUnderCursor = dot1;

    TraceLog(LOG_TRACE, "Overworld loaded.");
}

void OverworldInitialize() {

    GameStateReset();
    STATE->mode = MODE_OVERWORLD;

    if (!OW_LIST_HEAD) {
        // LinkedListRemoveAll(&OW_LIST_HEAD);

        initializeCursor();
        overworldLoad();
        updateCursorPosition();
    }

    STATE->expectingLevelAssociation = false;

    EditorSync();

    CameraPanningReset();

    CameraFollow();

    memset(STATE->loadedLevel, 0, sizeof(STATE->loadedLevel));

    RenderShowEntityInfoStop();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), false);

    TraceLog(LOG_INFO, "Overworld initialized.");
}

void OverworldLevelSelect() {

    if (overworldLevelSelectedAgo >= 0) return;
    

    if (!(STATE->tileUnderCursor->components & OW_IS_LEVEL_DOT)) {
        TraceLog(LOG_TRACE, "Overworld tried to enter level, but not a dot.");
        return;
    }

    if (!STATE->tileUnderCursor->levelName) {
        TraceLog(LOG_ERROR, "tileUnderCursor has no levelName reference.");
        return;
    }

    if (STATE->tileUnderCursor->levelName[0] == '\0') {
        STATE->expectingLevelAssociation = true;
    }

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), true);

    overworldLevelSelectedAgo = GetTime();
    overworldLevelSelectedName = STATE->tileUnderCursor->levelName;
}

void OverworldCursorMove(OverworldCursorDirection direction) {

    if (overworldLevelSelectedAgo >= 0) return;
    

    TraceLog(LOG_TRACE, "Overworld move to direction %d", direction);

    OverworldEntity *tileUnder = STATE->tileUnderCursor;

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
            STATE->tileUnderCursor = entity;
            updateCursorPosition();
            break;
        }

next_entity:
        node = node->next;
    }
}

void OverworldTileAddOrInteract(Vector2 pos) {

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
            TraceLog(LOG_TRACE, "Couldn't place tile, collided with item component=%d, x=%.1f, y=%.1f",
                            entity->components, entity->gridPos.x, entity->gridPos.y);
            return;
        }

        SpriteRotate(&entity->sprite, 90);
        TraceLog(LOG_TRACE, "Rotated tile component=%d, x=%.1f, y=%.1f",
                entity->components, entity->gridPos.x, entity->gridPos.y);
        return;

next_entity:
        node = node->next;
    }

    OverworldTileType typeToAdd;

    switch (STATE->editorSelectedEntity->type) {
        case EDITOR_ENTITY_LEVEL_DOT:
            typeToAdd = OW_LEVEL_DOT; break;
        case EDITOR_ENTITY_STRAIGHT:
            typeToAdd = OW_STRAIGHT_PATH; break;
        case EDITOR_ENTITY_PATH_JOIN:
            typeToAdd = OW_JOIN_PATH; break;
        case EDITOR_ENTITY_PATH_IN_L:
            typeToAdd = OW_PATH_IN_L; break;
        default:
            TraceLog(LOG_ERROR,
                        "Couldn't find Overworld Tile Type for Editor Item Type %d.",
                        STATE->editorSelectedEntity->type);
            return;
    }

    addTileToOverworld(pos, typeToAdd, 0);

    TraceLog(LOG_TRACE, "Added tile of type %d to the overworld (x=%.1f, y=%.1f).",
                typeToAdd, pos.x, pos.y);
}

void OverworldTileRemoveAt(Vector2 pos) {

    ListNode *node = getEntityOnScene(pos);

    if (!node) {

        TraceLog(LOG_TRACE, "Didn't find any tile to remove.");
        return;
    }

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (node->item == STATE->tileUnderCursor) {
        TraceLog(LOG_TRACE, "Won't remove tile, it's under the cursor.");
        return;
    }

    
    destroyEntityOverworld(node);
}

void OverworldTick() {

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (overworldLevelSelectedAgo != -1 &&
        GetTime() - overworldLevelSelectedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        LevelInitialize(overworldLevelSelectedName);

        overworldLevelSelectedAgo = -1;
        overworldLevelSelectedName = 0;

        return;
    }

    CameraTick();    
}

#include <raylib.h>
#include <string.h>
#include <stdlib.h>

#include "overworld.h"
#include "core.h"
#include "assets.h"
#include "camera.h"
#include "render.h"
#include "persistence.h"
#include "editor.h"


ListNode *OW_LIST_HEAD = 0;

double overworldLevelSelectedAgo = -1;


static OverworldEntity *OW_CURSOR = 0;

static char *overworldLevelSelectedName = 0;


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

    LinkedListDestroyNode(&OW_LIST_HEAD, node);

    TraceLog(LOG_TRACE, "Destroyed overworld entity.");
}

// Removes overworld tile, if possible 
static void checkAndRemoveTile(ListNode *node) {

    OverworldEntity *entity = (OverworldEntity *) node->item;

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (entity == STATE->tileUnderCursor ||
        entity->components & OW_IS_CURSOR) {

            TraceLog(LOG_TRACE, "Won't remove tile, it's the cursor or it's under it.");
            return;
    }
    
    destroyEntityOverworld(node);
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

// Searches for an entity that's not the cursor
// in a given position and returns its node, or 0 if not found.
static ListNode *getEntityOnScene(Vector2 pos) {

    ListNode *node = OW_LIST_HEAD;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (!(entity->components & OW_IS_CURSOR) &&
            CheckCollisionPointRec(pos, OverworldEntitySquare(entity))) {

                return node;
            }

        node = node->next;
    };

    return 0;
}

void OverworldInitialize() {

    GameStateReset();
    STATE->mode = MODE_OVERWORLD;

    if (!OW_LIST_HEAD) {

        initializeCursor();

        if (!PersistenceOverworldLoad()) {
            TraceLog(LOG_ERROR, "Could not initialize overworld; error reading persistence.");
            exit(1);
        }

        updateCursorPosition();
    }

    STATE->expectingLevelAssociation = false;

    EditorSync();

    CameraPanningReset();

    CameraFollow();

    memset(STATE->loadedLevel, 0, sizeof(STATE->loadedLevel));

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), false);

    TraceLog(LOG_INFO, "Overworld initialized.");
}

OverworldEntity *OverworldTileAdd(Vector2 pos, OverworldTileType type, int degrees) {

    OverworldEntity *newTile = MemAlloc(sizeof(OverworldEntity));

    newTile->tileType = type;
    newTile->gridPos = SnapToGrid(pos, OW_GRID);
    newTile->levelName = MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);

    switch (newTile->tileType)
    {
    case OW_LEVEL_DOT:
        newTile->components = OW_IS_LEVEL_DOT;
        newTile->sprite = LevelDotSprite;
        break;
    case OW_STRAIGHT_PATH:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = PathTileStraightSprite;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_JOIN_PATH:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = PathTileJoinSprite;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_PATH_IN_L:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
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

    CameraPanningReset();

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

    pos = SnapToGrid(pos, OW_GRID);

    Rectangle testHitbox = (Rectangle){ pos.x,
                                    pos.y,
                                    OW_GRID.width,
                                    OW_GRID.width };

    OverworldEntity *entity = OverworldCheckCollisionWithAnyTile(testHitbox);

    if (entity) {

        if (!(entity->components & OW_IS_ROTATABLE)) {
            TraceLog(LOG_TRACE, "Couldn't place tile, collided with item component=%d, x=%.1f, y=%.1f",
                            entity->components, entity->gridPos.x, entity->gridPos.y);
            return;
        }

        // Interacting
        SpriteRotate(&entity->sprite, 90);
        TraceLog(LOG_TRACE, "Rotated tile component=%d, x=%.1f, y=%.1f",
                entity->components, entity->gridPos.x, entity->gridPos.y);
        return;
    }

    // Adding
    OverworldTileType typeToAdd;

    // This is highly gambiarra, the tile type should be informed to this function somehow,
    // instead of having to read here which editor button is toggled
    switch (STATE->editorButtonToggled->type) {
        
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
                        STATE->editorButtonToggled->type);
            return;
    }

    OverworldTileAdd(pos, typeToAdd, 0);

    TraceLog(LOG_TRACE, "Added tile of type %d to the overworld (x=%.1f, y=%.1f).",
                typeToAdd, pos.x, pos.y);
}

void OverworldTileRemoveAt(Vector2 pos) {

    ListNode *node = getEntityOnScene(pos);

    if (!node) {

        TraceLog(LOG_TRACE, "Didn't find any tile to remove.");
        return;
    }

    OverworldEntity *entity = (OverworldEntity *) node->item;

    bool isPartOfSelection = LinkedListGetNode(EDITOR_STATE->selectedEntities, entity);
    if (isPartOfSelection) {

        ListNode *node = EDITOR_STATE->selectedEntities;
        while (node) {
            ListNode *next = node->next;
            ListNode *nodeInOW = LinkedListGetNode(OW_LIST_HEAD, (OverworldEntity *) node->item);
            checkAndRemoveTile(nodeInOW);
            node = next;
        }

        EditorSelectionCancel();

    } else {

        checkAndRemoveTile(node);
    }
}

OverworldEntity *OverworldCheckCollisionWithAnyTile(Rectangle hitbox) {

    ListNode *node = OW_LIST_HEAD;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (entity->tileType & OW_NOT_TILE) goto next_entity;

        if (!CheckCollisionRecs(hitbox, OverworldEntitySquare(entity))) goto next_entity;

        return entity;

next_entity:
        node = node->next;
    }

    return 0;
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

    updateCursorPosition();

    CameraTick();    
}

void OverworldSave() {
    PersistenceOverworldSave();
}

Rectangle OverworldEntitySquare(OverworldEntity *entity) {

    return (Rectangle) {
        entity->gridPos.x,
        entity->gridPos.y,
        OW_GRID.width,
        OW_GRID.height,
    };
}

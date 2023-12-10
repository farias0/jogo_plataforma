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
#include "debug.h"


OverworldState *OW_STATE = 0;

// Reference to the cursor part of the entity list
static OverworldEntity *OW_CURSOR = 0;

// How long ago, in seconds, a level was selected in the OW, or -1 if it wasn't
static double levelSelectedAgo = -1;

// The name of the selected level
static char *levelSelectedName = 0;


static void initializeOverworldState() {

    OW_STATE = MemAlloc(sizeof(OverworldState));

    TraceLog(LOG_INFO, "Overworld State initialized.");
}

// Updates the position for the cursor according to the tile under it
static void updateCursorPosition() {

    Dimensions cursorDimensions = SpriteScaledDimensions(SPRITES->OverworldCursor);

    OW_CURSOR->gridPos.x = OW_STATE->tileUnderCursor->gridPos.x;

    OW_CURSOR->gridPos.y = OW_STATE->tileUnderCursor->gridPos.y +
                    (OW_GRID.height / 2) -
                    cursorDimensions.height;
}

static void destroyEntityOverworld(ListNode *node) {

    OverworldEntity *entity = (OverworldEntity *) node->item;

    DebugEntityStop(entity);

    MemFree(entity->levelName);

    LinkedListDestroyNode(&OW_STATE->listHead, node);

    TraceLog(LOG_TRACE, "Destroyed overworld entity.");
}

// Removes overworld tile, if possible 
static void checkAndRemoveTile(ListNode *node) {

    OverworldEntity *entity = (OverworldEntity *) node->item;

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (entity == OW_STATE->tileUnderCursor ||
        entity->components & OW_IS_CURSOR) {

            TraceLog(LOG_TRACE, "Won't remove tile, it's the cursor or it's under it.");
            return;
    }
    
    destroyEntityOverworld(node);
}

static void initializeCursor() {

    OverworldEntity *newCursor = MemAlloc(sizeof(OverworldEntity));

    newCursor->components = OW_IS_CURSOR;
    newCursor->sprite = SPRITES->OverworldCursor;
    newCursor->layer = 1;

    LinkedListAdd(&OW_STATE->listHead, newCursor);

    OW_CURSOR = newCursor;

    TraceLog(LOG_TRACE, "Added cursor to overworld (x=%.1f, y=%.1f)",
                newCursor->gridPos.x, newCursor->gridPos.y);
}

// Searches for an entity in a given position
// and returns its node, or 0 if not found.
static ListNode *getEntityNodeAtPos(Vector2 pos) {

    ListNode *node = OW_STATE->listHead;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (CheckCollisionPointRec(pos, OverworldEntitySquare(entity))) {

                return node;
            }

        node = node->next;
    };

    return 0;
}

void OverworldInitialize() {

    initializeOverworldState();

    initializeCursor();

    if (!PersistenceOverworldLoad()) {
        TraceLog(LOG_ERROR, "Could not initialize overworld; error reading persistence.");
        exit(1);
    }

    TraceLog(LOG_INFO, "Overworld system initialized.");
}

void OverworldLoad() {

    GAME_STATE->mode = MODE_OVERWORLD;

    updateCursorPosition();

    EditorSync();

    CameraPanningReset();

    CameraFollow();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), false);

    TraceLog(LOG_INFO, "Overworld loaded.");
}

OverworldEntity *OverworldTileAdd(Vector2 pos, OverworldTileType type, int degrees) {

    OverworldEntity *newTile = MemAlloc(sizeof(OverworldEntity));

    newTile->tileType = type;
    newTile->gridPos = pos;
    newTile->levelName = MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);

    switch (newTile->tileType)
    {
    case OW_LEVEL_DOT:
        newTile->components = OW_IS_LEVEL_DOT;
        newTile->sprite = SPRITES->LevelDot;
        break;
    case OW_STRAIGHT_PATH:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = SPRITES->PathTileStraight;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_JOIN_PATH:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = SPRITES->PathTileJoin;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    case OW_PATH_IN_L:
        newTile->components = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = SPRITES->PathTileInL;
        SpriteRotate(&newTile->sprite, degrees);
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for overworld tile type %d.", type);
    }

    LinkedListAdd(&OW_STATE->listHead, newTile);

    TraceLog(LOG_TRACE, "Added tile to overworld (x=%.1f, y=%.1f)",
                newTile->gridPos.x, newTile->gridPos.y);

    return newTile;
}

OverworldEntity *OverworldEntityGetAt(Vector2 pos) {

    ListNode *node = getEntityNodeAtPos(pos);

    if (!node) return 0;

    return ((OverworldEntity *) node->item);
}

void OverworldLevelSelect() {

    if (levelSelectedAgo >= 0) return;
    

    if (!(OW_STATE->tileUnderCursor->components & OW_IS_LEVEL_DOT)) {
        TraceLog(LOG_TRACE, "Overworld tried to enter level, but not a dot.");
        return;
    }

    if (!OW_STATE->tileUnderCursor->levelName) {
        TraceLog(LOG_ERROR, "tileUnderCursor has no levelName reference.");
        return;
    }

    CameraPanningReset();

    DebugEntityStopAll();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), true);

    levelSelectedAgo = GetTime();
    levelSelectedName = OW_STATE->tileUnderCursor->levelName;
}

void OverworldCursorMove(OverworldCursorDirection direction) {

    if (levelSelectedAgo >= 0) return;
    

    TraceLog(LOG_TRACE, "Overworld move to direction %d", direction);

    OverworldEntity *tileUnder = OW_STATE->tileUnderCursor;

    ListNode *node = OW_STATE->listHead;

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
            OW_STATE->tileUnderCursor = entity;
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
                                    OW_GRID.height };

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
    switch (EDITOR_STATE->toggledEntityButton->type) {
        
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
                        EDITOR_STATE->toggledEntityButton->type);
            return;
    }

    OverworldTileAdd(pos, typeToAdd, 0);

    TraceLog(LOG_TRACE, "Added tile of type %d to the overworld (x=%.1f, y=%.1f).",
                typeToAdd, pos.x, pos.y);
}

void OverworldTileRemoveAt(Vector2 pos) {

    ListNode *node = getEntityNodeAtPos(pos);

    if (!node) {
        TraceLog(LOG_TRACE, "Didn't find any tile to remove.");
        return;
    }

    OverworldEntity *entity = (OverworldEntity *) node->item;

    if (entity == OW_CURSOR) {
        TraceLog(LOG_TRACE, "Can't remove overworld cursor.");
        return;
    }

    bool isPartOfSelection = LinkedListGetNode(EDITOR_STATE->selectedEntities, entity);
    if (isPartOfSelection) {

        ListNode *node = EDITOR_STATE->selectedEntities;
        while (node) {
            ListNode *next = node->next;
            ListNode *nodeInOW = LinkedListGetNode(OW_STATE->listHead, (OverworldEntity *) node->item);
            checkAndRemoveTile(nodeInOW);
            node = next;
        }

        EditorSelectionCancel();

    } else {

        checkAndRemoveTile(node);
    }
}

OverworldEntity *OverworldCheckCollisionWithAnyTile(Rectangle hitbox) {

    return OverworldCheckCollisionWithAnyTileExcept(hitbox, 0);
}

OverworldEntity *OverworldCheckCollisionWithAnyTileExcept(Rectangle hitbox, ListNode *entityListHead) {

    ListNode *node = OW_STATE->listHead;

    while (node != 0) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        if (entity->tileType == OW_NOT_TILE) goto next_entity;

        if (!CheckCollisionRecs(hitbox, OverworldEntitySquare(entity))) goto next_entity;

        ListNode *excludedNode = entityListHead;
        while (excludedNode != 0) {
            if (excludedNode->item == entity) goto next_entity;
            excludedNode = excludedNode->next;
        }

        return entity;

next_entity:
        node = node->next;
    }

    return 0;
}

void OverworldTick() {

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (levelSelectedAgo != -1 &&
        GetTime() - levelSelectedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        LevelLoad(levelSelectedName);

        levelSelectedAgo = -1;
        levelSelectedName = 0;

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

#include <raylib.h>
#include <stdlib.h>

#include "overworld.hpp"
#include "core.hpp"
#include "assets.hpp"
#include "camera.hpp"
#include "render.hpp"
#include "persistence.hpp"
#include "editor.hpp"
#include "debug.hpp"


OverworldState *OW_STATE = 0;

// Reference to the cursor part of the entity list
static OverworldEntity *OW_CURSOR = 0;

// How long ago, in seconds, a level was selected in the OW, or -1 if it wasn't
static double levelSelectedAgo = -1;

// The name of the selected level
static char *levelSelectedName = 0;


static void initializeOverworldState() {

    OW_STATE = (OverworldState *) MemAlloc(sizeof(OverworldState));

    TraceLog(LOG_INFO, "Overworld State initialized.");
}

// Updates the position for the cursor according to the tile under it
static void updateCursorPosition() {

    Dimensions cursorDimensions = SpriteScaledDimensions(&SPRITES->OverworldCursor);

    OW_CURSOR->gridPos.x = OW_STATE->tileUnderCursor->gridPos.x;

    OW_CURSOR->gridPos.y = OW_STATE->tileUnderCursor->gridPos.y +
                    (OW_GRID.height / 2) -
                    cursorDimensions.height;
}

static void destroyEntityOverworld(OverworldEntity *entity) {

    DebugEntityStop(entity);

    MemFree(entity->levelName);

    LinkedList::DestroyNode(&OW_STATE->listHead, entity);

    TraceLog(LOG_TRACE, "Destroyed overworld entity.");
}

// Removes overworld tile, if possible 
static void checkAndRemoveTile(OverworldEntity *entity) {

    // Ideally the game would support removing the tile under the player,
    // but this would demand some logic to manage the tileUnder pointer.
    // For now this is good enough.
    if (entity == OW_STATE->tileUnderCursor ||
        entity->tags & OW_IS_CURSOR) {

            TraceLog(LOG_TRACE, "Won't remove tile, it's the cursor or it's under it.");
            return;
    }
    
    destroyEntityOverworld(entity);
}

static void initializeCursor() {

    OverworldEntity *newCursor = new OverworldEntity();

    newCursor->tags = OW_IS_CURSOR;
    newCursor->sprite = &SPRITES->OverworldCursor;
    newCursor->layer = 1;

    LinkedList::AddNode(&OW_STATE->listHead, newCursor);

    OW_CURSOR = newCursor;

    TraceLog(LOG_TRACE, "Added cursor to overworld (x=%.1f, y=%.1f)",
                newCursor->gridPos.x, newCursor->gridPos.y);
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

    Render::LevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), false);

    TraceLog(LOG_INFO, "Overworld loaded.");
}

OverworldEntity *OverworldTileAdd(Vector2 pos, OverworldTileType type, int degrees) {

    if (degrees >= 360 || degrees < 0) {
        degrees %= 360;
    }


    OverworldEntity *newTile = new OverworldEntity();

    newTile->tileType = type;
    newTile->gridPos = pos;
    newTile->levelName = (char *) MemAlloc(sizeof(char) * LEVEL_NAME_BUFFER_SIZE);

    switch (newTile->tileType)
    {
    case OW_LEVEL_DOT:
        newTile->tags = OW_IS_LEVEL_DOT;
        newTile->sprite = &SPRITES->LevelDot;
        break;
    case OW_STRAIGHT_PATH:
        newTile->tags = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = &SPRITES->PathTileStraight;
        newTile->rotation = degrees;
        break;
    case OW_JOIN_PATH:
        newTile->tags = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = &SPRITES->PathTileJoin;
        newTile->rotation = degrees;
        break;
    case OW_PATH_IN_L:
        newTile->tags = OW_IS_PATH + OW_IS_ROTATABLE;
        newTile->sprite = &SPRITES->PathTileInL;
        newTile->rotation = degrees;
        break;
    default:
        TraceLog(LOG_ERROR, "Could not find sprite for overworld tile type %d.", type);
    }

    LinkedList::AddNode(&OW_STATE->listHead, newTile);

    TraceLog(LOG_TRACE, "Added tile to overworld (x=%.1f, y=%.1f)",
                newTile->gridPos.x, newTile->gridPos.y);

    return newTile;
}

OverworldEntity *OverworldEntityGetAt(Vector2 pos) {

    OverworldEntity *entity = (OverworldEntity *) OW_STATE->listHead;

    while (entity != 0) {

        if (CheckCollisionPointRec(pos, OverworldEntitySquare(entity))) {

                return entity;
            }

        entity = (OverworldEntity *) entity->next;
    };

    return 0;
}

void OverworldLevelSelect() {

    if (levelSelectedAgo >= 0) return;
    

    if (!(OW_STATE->tileUnderCursor->tags & OW_IS_LEVEL_DOT)) {
        TraceLog(LOG_TRACE, "Overworld tried to enter level, but not a dot.");
        return;
    }

    if (!OW_STATE->tileUnderCursor->levelName) {
        TraceLog(LOG_ERROR, "tileUnderCursor has no levelName reference.");
        return;
    }

    CameraPanningReset();

    DebugEntityStopAll();

    Render::LevelTransitionEffectStart(
        SpritePosMiddlePoint(OW_CURSOR->gridPos, OW_CURSOR->sprite), true);

    levelSelectedAgo = GetTime();
    levelSelectedName = OW_STATE->tileUnderCursor->levelName;
}

void OverworldCursorMove(OverworldCursorDirection direction) {

    if (levelSelectedAgo >= 0) return;
    

    TraceLog(LOG_TRACE, "Overworld move to direction %d", direction);

    OverworldEntity *tileUnder = OW_STATE->tileUnderCursor;

    OverworldEntity *entity = (OverworldEntity *) OW_STATE->listHead;

    while (entity != 0) {

        bool isOnTheSameRow;
        bool isOnTheSameColumn;
        bool foundPath;

        // TODO replace this with a tag "OW_IS_WALKABLE"
        bool isTile = (entity->tags & OW_IS_PATH) ||
                        (entity->tags & OW_IS_LEVEL_DOT);
        if (!isTile) goto next_entity;

        isOnTheSameRow = tileUnder->gridPos.y == entity->gridPos.y;
        isOnTheSameColumn = tileUnder->gridPos.x == entity->gridPos.x;
        foundPath = false;

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
        entity = (OverworldEntity *) entity->next;
    }
}

void OverworldTileAddOrInteract(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;
    

    pos = SnapToGrid(pos, OW_GRID);

    Rectangle testHitbox = { pos.x,
                                pos.y,
                                OW_GRID.width,
                                OW_GRID.height };

    OverworldEntity *entity = OverworldCheckCollisionWithAnyTile(testHitbox);

    if (entity) {

        if (!(entity->tags & OW_IS_ROTATABLE)) {
            TraceLog(LOG_TRACE, "Couldn't place tile, collided with item component=%d, x=%.1f, y=%.1f",
                            entity->tags, entity->gridPos.x, entity->gridPos.y);
            return;
        }

        // Interacting
        entity->rotation += 90;
        if (entity->rotation >= 360) entity->rotation -= 360;

        TraceLog(LOG_TRACE, "Rotated tile component=%d, x=%.1f, y=%.1f",
                entity->tags, entity->gridPos.x, entity->gridPos.y);
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

    OverworldEntity *entity = OverworldEntityGetAt(pos);

    if (!entity) {
        TraceLog(LOG_TRACE, "Didn't find any tile to remove.");
        return;
    }

    if (entity == OW_CURSOR) {
        TraceLog(LOG_TRACE, "Can't remove overworld cursor.");
        return;
    }

    bool isPartOfSelection = std::find(EDITOR_STATE->selectedEntities.begin(), EDITOR_STATE->selectedEntities.end(), entity) != EDITOR_STATE->selectedEntities.end();
    if (isPartOfSelection) {

        for (auto e = EDITOR_STATE->selectedEntities.begin(); e < EDITOR_STATE->selectedEntities.end(); e++) {
            checkAndRemoveTile((OverworldEntity *) *e);
        }

        EditorSelectionCancel();

    } else {

        checkAndRemoveTile(entity);
    }
}

OverworldEntity *OverworldCheckCollisionWithAnyTile(Rectangle hitbox) {

    return OverworldCheckCollisionWithAnyTileExcept(hitbox, std::vector<LinkedList::Node *>());
}

OverworldEntity *OverworldCheckCollisionWithAnyTileExcept(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore) {

    OverworldEntity *entity = (OverworldEntity *) OW_STATE->listHead;

    while (entity != 0) {

        if (entity->tileType == OW_NOT_TILE) goto next_entity;

        if (!CheckCollisionRecs(hitbox, OverworldEntitySquare(entity))) goto next_entity;

        for (auto e = entitiesToIgnore.begin(); e < entitiesToIgnore.end(); e++) {
            if (*e == entity) goto next_entity;
        }

        return entity;

next_entity:
        entity = (OverworldEntity *) entity->next;
    }

    return 0;
}

void OverworldTick() {

    if (GAME_STATE->waitingForTextInput) return;

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (levelSelectedAgo != -1 &&
        GetTime() - levelSelectedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        Level::Load(levelSelectedName);

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

    return {
        entity->gridPos.x,
        entity->gridPos.y,
        OW_GRID.width,
        OW_GRID.height,
    };
}

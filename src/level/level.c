#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#include "level.h"
#include "../core.h"
#include "../camera.h"
#include "../render.h"
#include "../editor.h"
#include "../overworld.h"


// The difference between the y of the hitbox and the ground to be considered "on the ground"
#define ON_THE_GROUND_Y_TOLERANCE 5

// TODO seek an unused level name, i.e. "new_level_2.lvl"
// THe name of a new level
#define DEFAULT_NEW_LEVEL_NAME "new_level.lvl"

// The players origin for all new levels, unchangeably.
// -- This allows us to set the player's origin via code for all new levels,
// untying it from the default new level's data.
// TODO allow to drag and modify the player's origin in the editor
#define PLAYERS_ORIGIN (Vector2){ 344, 200 };


ListNode *LEVEL_LIST_HEAD = 0;

double levelConcludedAgo = -1;

static ListNode *levelExitNode = 0;


// Searches for an entity in a given position
// and returns its node, or 0 if not found.
static ListNode *getEntityOnScene(Vector2 pos) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (CheckCollisionPointRec(pos, entity->hitbox)) {

            return node;
        }

        node = node->next;
    };

    return 0;
}

// Searches the level for a ground immediatelly beneath the hitbox.
// Accepts an optional 'entity' reference, in case its checking for ground
// beneath an existing level entity.
static LevelEntity *getGroundBeneath(Rectangle hitbox, LevelEntity *entity) {

    if (!entity) {
        // Virtual entity to use when checking only hitboxes
        LevelEntity virtualEntity;
        entity = &virtualEntity;
    }

    int feetHeight = hitbox.y + hitbox.height;

    ListNode *node = LEVEL_LIST_HEAD;

    LevelEntity *foundGround = 0; 

    while (node != 0) {

        LevelEntity *possibleGround = (LevelEntity *)node->item;

        if (possibleGround != entity &&
            possibleGround->components & LEVEL_IS_GROUND &&

            !(possibleGround->isDead) &&

            // If x is within the possible ground
            possibleGround->hitbox.x < (hitbox.x + hitbox.width) &&
            hitbox.x < (possibleGround->hitbox.x + possibleGround->hitbox.width) &&

            // If y is RIGHT above the possible ground
            abs((int) (possibleGround->hitbox.y - feetHeight)) <= ON_THE_GROUND_Y_TOLERANCE) {
                
                // Is on a ground

                if (foundGround) {

                    // In case of multiple grounds beneath
                    if (
                        (entity->isFacingRight && (possibleGround->hitbox.x > foundGround->hitbox.x)) ||
                        (!entity->isFacingRight && (possibleGround->hitbox.x < foundGround->hitbox.x))
                    ) {
                        
                        foundGround = possibleGround;

                    }
                }
                else foundGround = possibleGround;
            } 

        node = node->next;
    }

    return foundGround;   
}

void LevelInitialize(char *levelName) {

    GameStateReset();
    STATE->mode = MODE_IN_LEVEL;

    LinkedListDestroyAll(&LEVEL_LIST_HEAD);
    LEVEL_PLAYER = 0;
    levelExitNode = 0;

    if (levelName[0] == '\0') {
        EditorEmpty();
        CameraPanningReset();
        TraceLog(LOG_INFO, "Level waiting for file drop.");
        return;
    }

    if (!PersistenceLevelLoad(levelName)) {
        OverworldInitialize();
        return;
    }

    strcpy(STATE->loadedLevel, levelName);

    EditorSync();

    CameraLevelCentralizeOnPlayer();

    CameraFollow();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(
            (Vector2){LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y}, LEVEL_PLAYER->sprite), false);

    TraceLog(LOG_INFO, "Level initialized: %s.", levelName);
}

void LevelGoToOverworld() {

    if (!LEVEL_PLAYER) {
        OverworldInitialize();
        return;    
    }

    CameraPanningReset();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(
            (Vector2){LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y}, LEVEL_PLAYER->sprite), true);

    RenderDebugEntityStopAll();

    levelConcludedAgo = GetTime();
}

void LevelExitAdd(Vector2 pos) {

    LevelEntity *newExit = MemAlloc(sizeof(LevelEntity));

    Sprite sprite = LevelEndOrbSprite;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newExit->components = LEVEL_IS_EXIT;
    newExit->hitbox = hitbox;
    newExit->origin = pos;
    newExit->sprite = sprite;
    newExit->isFacingRight = true;

    levelExitNode = LinkedListAdd(&LEVEL_LIST_HEAD, newExit);

    TraceLog(LOG_TRACE, "Added exit to level (x=%.1f, y=%.1f)",
                newExit->hitbox.x, newExit->hitbox.y);
}

void LevelExitCheckAndAdd(Vector2 pos) {
    
    Rectangle hitbox = SpriteHitboxFromMiddle(LevelEndOrbSprite, pos);

    if (LevelCheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add level exit, collision with entity.");
        return;
    }

    // Currently only one level exit is supported, but this should change in the future.
    if (levelExitNode) LinkedListDestroyNode(&LEVEL_LIST_HEAD, levelExitNode);
    
    LevelExitAdd((Vector2){ hitbox.x, hitbox.y });
}

LevelEntity *LevelGetGroundBeneath(LevelEntity *entity) {

    return getGroundBeneath(entity->hitbox, entity);    
}

LevelEntity *LevelGetGroundBeneathHitbox(Rectangle hitbox) {

    return getGroundBeneath(hitbox, 0);
}

void LevelEntityDestroy(ListNode *node) {

    if (node == levelExitNode) levelExitNode = 0;

    RenderDebugEntityStop((LevelEntity *) node->item);

    LinkedListDestroyNode(&LEVEL_LIST_HEAD, node);

    TraceLog(LOG_TRACE, "Destroyed level entity.");
}

LevelEntity *LevelEntityGetAt(Vector2 pos) {

    ListNode *node = getEntityOnScene(pos);

    if (!node) return 0;

    return (LevelEntity *) node->item;
}

void LevelEntityRemoveAt(Vector2 pos) {

    // TODO This function is a monstruosity and should be broken up
    // in at least two others ASAP

    ListNode *node = LEVEL_LIST_HEAD;
    while (node != 0) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity->components & LEVEL_IS_PLAYER) goto next_node;

        if (CheckCollisionPointRec(pos, LevelEntityOriginHitbox(entity))) {
            break;
        }

        if (!(entity->isDead) && CheckCollisionPointRec(pos, entity->hitbox)) {
            break;
        }

next_node:
        node = node->next;
    };

    if (!node) return;

    LevelEntity *entity = (LevelEntity *) node->item;

    bool isPartOfSelection = LinkedListGetNode(EDITOR_STATE->selectedEntities, entity);
    if (isPartOfSelection) {

        ListNode *selectedNode = EDITOR_STATE->selectedEntities;
        while (selectedNode) {

            ListNode *next = selectedNode->next;
            LevelEntity *selectedEntity = (LevelEntity *) selectedNode->item;

            if (selectedEntity->components & LEVEL_IS_PLAYER) goto next_selected_node;

            ListNode *nodeInLevel = LinkedListGetNode(LEVEL_LIST_HEAD, selectedEntity);
            LevelEntityDestroy(nodeInLevel);

next_selected_node:
            selectedNode = next;
        }

        EditorSelectionCancel();

    } else {

        LevelEntityDestroy(node);
    }
}

void LevelTick() {

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (levelConcludedAgo != -1 &&
        GetTime() - levelConcludedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        OverworldInitialize();

        levelConcludedAgo = -1;

        return;
    }

    if (STATE->isPaused) goto skip_entities_tick;
    if (STATE->isEditorEnabled) goto skip_entities_tick;

    ListNode *node = LEVEL_LIST_HEAD;
    ListNode *next;

    while (node != 0) {

        next = node->next;

        LevelEntity *entity = (LevelEntity *)node->item;

        if (entity->components & LEVEL_IS_ENEMY) LevelEnemyTick(node);
        else if (entity->components & LEVEL_IS_PLAYER) LevelPlayerTick();

        node = next;
    }
skip_entities_tick:

    CameraTick();
}

bool LevelCheckCollisionWithAnyEntity(Rectangle hitbox) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {
    
        LevelEntity *entity = (LevelEntity *) node->item;

        if (!(entity->isDead) && CheckCollisionRecs(hitbox, entity->hitbox)) {

            return true;
        }

        node = node->next;

    }

    return false;
}

bool LevelCheckCollisionWithAnything(Rectangle hitbox) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {
    
        LevelEntity *entity = (LevelEntity *) node->item;

        Rectangle entitysOrigin = (Rectangle) {
                                                entity->origin.x,       entity->origin.y,
                                                entity->hitbox.width,   entity->hitbox.height
                                            };

        if (CheckCollisionRecs(hitbox, entitysOrigin) ||
            (!(entity->isDead) && CheckCollisionRecs(hitbox, entity->hitbox))) {

            return true;
        }

        node = node->next;

    }

    return false;
}

void LevelSave() {
    PersistenceLevelSave(STATE->loadedLevel);
}

void LevelLoadNew() {
    LevelInitialize(NEW_LEVEL_NAME);

    LEVEL_PLAYER->origin = PLAYERS_ORIGIN;
    LEVEL_PLAYER->hitbox.x = LEVEL_PLAYER->origin.x;
    LEVEL_PLAYER->hitbox.y = LEVEL_PLAYER->origin.y;

    strcpy(STATE->loadedLevel, DEFAULT_NEW_LEVEL_NAME);
}

Rectangle LevelEntityOriginHitbox(LevelEntity *entity) {

    return (Rectangle){
        entity->origin.x, entity->origin.y,
        entity->hitbox.width, entity->hitbox.height
    };
}

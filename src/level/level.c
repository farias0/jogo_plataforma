#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#include "level.h"
#include "player.h"
#include "enemy.h"
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

// With how many checkpoints available the player starts
#define STARTING_CHECKPOINTS_NUMBER          1;


LevelState *LEVEL_STATE = 0;


void resetLevelState() {

    LinkedListDestroyAll(&LEVEL_STATE->listHead);
    memset(LEVEL_STATE->levelName, 0, sizeof(LEVEL_STATE->levelName));
    LEVEL_STATE->isPaused = false;
    LEVEL_STATE->awaitingAssociation = false;
    LEVEL_STATE->concludedAgo = -1;
    LEVEL_STATE->exitNode = 0;
    LEVEL_STATE->checkpoint = 0;
    LEVEL_STATE->checkpointsLeft = 0;

    PLAYER_ENTITY = 0;

    TraceLog(LOG_INFO, "Level State initialized.");
}

void initializeLevelState() {

    LEVEL_STATE = MemAlloc(sizeof(LevelState));

    resetLevelState();

    TraceLog(LOG_INFO, "Level State initialized.");
}

void leaveLevel() {
    
    resetLevelState();
    OverworldInitialize();

    TraceLog(LOG_TRACE, "Level left.");
}

// Searches for an entity in a given position
// and returns its node, or 0 if not found.
static ListNode *getEntityOnScene(Vector2 pos) {

    ListNode *node = LEVEL_STATE->listHead;

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

    // Virtual entity to use when checking only hitboxes
    LevelEntity virtualEntity;
    if (!entity) {
        virtualEntity.isFacingRight = false;
        entity = &virtualEntity;
    }

    int feetHeight = hitbox.y + hitbox.height;

    ListNode *node = LEVEL_STATE->listHead;

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

    if (!LEVEL_STATE) initializeLevelState();

    GAME_STATE->mode = MODE_IN_LEVEL;

    if (levelName[0] == '\0') {
        EditorEmpty();
        CameraPanningReset();
        LEVEL_STATE->awaitingAssociation = true;
        TraceLog(LOG_INFO, "Level waiting for file drop.");
        return;
    }

    if (!PersistenceLevelLoad(levelName)) {
        leaveLevel();
        return;
    }

    LEVEL_STATE->checkpointsLeft = STARTING_CHECKPOINTS_NUMBER;

    strcpy(LEVEL_STATE->levelName, levelName);

    EditorSync();

    CameraLevelCentralizeOnPlayer();

    CameraFollow();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(
            (Vector2){PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y}, PLAYER_ENTITY->sprite), false);

    TraceLog(LOG_INFO, "Level initialized: %s.", levelName);
}

void LevelGoToOverworld() {

    if (!PLAYER_ENTITY) {
        leaveLevel();
        return;    
    }

    CameraPanningReset();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(
            (Vector2){PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y}, PLAYER_ENTITY->sprite), true);

    RenderDebugEntityStopAll();

    LEVEL_STATE->concludedAgo = GetTime();
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

    LEVEL_STATE->exitNode = LinkedListAdd(&LEVEL_STATE->listHead, newExit);

    TraceLog(LOG_TRACE, "Added exit to level (x=%.1f, y=%.1f)",
                newExit->hitbox.x, newExit->hitbox.y);
}

LevelEntity *LevelCheckpointAdd(Vector2 pos) {

    LevelEntity *newCheckpoint = MemAlloc(sizeof(LevelEntity));

    Sprite sprite = LevelCheckpointSprite;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newCheckpoint->components = LEVEL_IS_CHECKPOINT;
    newCheckpoint->hitbox = hitbox;
    newCheckpoint->origin = pos;
    newCheckpoint->sprite = sprite;
    newCheckpoint->isFacingRight = true;
    newCheckpoint->layer = -1;

    LinkedListAdd(&LEVEL_STATE->listHead, newCheckpoint);

    TraceLog(LOG_TRACE, "Added checkpoint to level (x=%.1f, y=%.1f)",
                newCheckpoint->hitbox.x, newCheckpoint->hitbox.y);

    return newCheckpoint;
}

void LevelExitCheckAndAdd(Vector2 pos) {
    
    Rectangle hitbox = SpriteHitboxFromMiddle(LevelEndOrbSprite, pos);

    if (LevelCheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add level exit, collision with entity.");
        return;
    }

    // Currently only one level exit is supported, but this should change in the future.
    if (LEVEL_STATE->exitNode)
        LinkedListDestroyNode(&LEVEL_STATE->listHead, LEVEL_STATE->exitNode);
    
    LevelExitAdd((Vector2){ hitbox.x, hitbox.y });
}

LevelEntity *LevelGetGroundBeneath(LevelEntity *entity) {

    return getGroundBeneath(entity->hitbox, entity);    
}

LevelEntity *LevelGetGroundBeneathHitbox(Rectangle hitbox) {

    return getGroundBeneath(hitbox, 0);
}

void LevelEntityDestroy(ListNode *node) {

    if (node == LEVEL_STATE->exitNode) LEVEL_STATE->exitNode = 0;

    RenderDebugEntityStop((LevelEntity *) node->item);

    LinkedListDestroyNode(&LEVEL_STATE->listHead, node);

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

    ListNode *node = LEVEL_STATE->listHead;
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

            ListNode *nodeInLevel = LinkedListGetNode(LEVEL_STATE->listHead, selectedEntity);
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
    if (LEVEL_STATE->concludedAgo != -1 &&
        GetTime() - LEVEL_STATE->concludedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        leaveLevel();

        LEVEL_STATE->concludedAgo = -1;

        return;
    }

    if (LEVEL_STATE->isPaused) goto skip_entities_tick;
    if (EDITOR_STATE->isEnabled) goto skip_entities_tick;

    ListNode *node = LEVEL_STATE->listHead;
    ListNode *next;

    while (node != 0) {

        next = node->next;

        LevelEntity *entity = (LevelEntity *)node->item;

        if (entity->components & LEVEL_IS_ENEMY) EnemyTick(node);
        else if (entity->components & LEVEL_IS_PLAYER) PlayerTick();

        node = next;
    }
skip_entities_tick:

    CameraTick();
}

bool LevelCheckCollisionWithAnyEntity(Rectangle hitbox) {

    ListNode *node = LEVEL_STATE->listHead;

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

    ListNode *node = LEVEL_STATE->listHead;

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
    PersistenceLevelSave(LEVEL_STATE->levelName);
}

void LevelLoadNew() {
    LevelInitialize(NEW_LEVEL_NAME);

    PLAYER_ENTITY->origin = PLAYERS_ORIGIN;
    PLAYER_ENTITY->hitbox.x = PLAYER_ENTITY->origin.x;
    PLAYER_ENTITY->hitbox.y = PLAYER_ENTITY->origin.y;

    strcpy(LEVEL_STATE->levelName, DEFAULT_NEW_LEVEL_NAME);
}

Rectangle LevelEntityOriginHitbox(LevelEntity *entity) {

    return (Rectangle){
        entity->origin.x, entity->origin.y,
        entity->hitbox.width, entity->hitbox.height
    };
}

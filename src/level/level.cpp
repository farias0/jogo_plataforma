#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <sstream>

#include "level.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../editor.hpp"
#include "../overworld.hpp"
#include "../debug.hpp"
#include "../input.hpp"


// The difference between the y of the hitbox and the ground to be considered "on the ground"
#define ON_THE_GROUND_Y_TOLERANCE       5

// TODO seek an unused level name, i.e. "new_level_2.lvl"
// The filename of the newly created levels
#define NEW_LEVEL_NAME                  "new_level.lvl"

// The players origin by default.
#define PLAYERS_ORIGIN                  { 344, 200 };

// With how many checkpoints available the player starts
#define STARTING_CHECKPOINTS_NUMBER     1;


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

    LEVEL_STATE = (LevelState *) MemAlloc(sizeof(LevelState));

    resetLevelState();

    TraceLog(LOG_INFO, "Level State initialized.");
}

void leaveLevel() {
    
    RenderDisplayTextboxStop();
    
    resetLevelState();

    OverworldLoad();

    TraceLog(LOG_TRACE, "Level left.");
}

// Searches for an entity in a given position
// and returns its node, or 0 if not found.
static ListNode *getEntityNodeAtPos(Vector2 pos) {

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

void tickAllEntities() {

    ListNode *node = LEVEL_STATE->listHead;
    ListNode *next;

    while (node != 0) {

        next = node->next;

        LevelEntity *entity = (LevelEntity *)node->item;

        if (entity->components & LEVEL_IS_ENEMY) EnemyTick(node);
        else if (entity->components & LEVEL_IS_PLAYER) PlayerTick();

        node = next;
    }
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

void createTextboxFromIdInput(Vector2 pos, std::string input) {

    int id;
    
    try {
        id = std::stoi(input);
    }
    catch (std::invalid_argument &e) {
        RenderPrintSysMessage((char *) "ID inválido");
        TraceLog(LOG_DEBUG, "Textbox ID input invalid: %s.", input.c_str());
        return; // does nothing
    }

    LevelTextboxAdd(pos, id);
}

void LevelInitialize() {

    initializeLevelState();
    TraceLog(LOG_INFO, "Level system initialized.");
}

void LevelLoad(char *levelName) {

    // Gambiarra. In case a level file was dragged and there was a level loaded already
    if (PLAYER_ENTITY) {
        resetLevelState();
    }

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
            {PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y}, PLAYER_ENTITY->sprite), false);

    TraceLog(LOG_INFO, "Level loaded: %s.", levelName);
}

void LevelGoToOverworld() {

    if (!PLAYER_ENTITY) {
        leaveLevel();
        return;    
    }

    CameraPanningReset();

    RenderLevelTransitionEffectStart(
        SpritePosMiddlePoint(
            {PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y}, PLAYER_ENTITY->sprite), true);

    DebugEntityStopAll();

    LEVEL_STATE->concludedAgo = GetTime();
}

LevelEntity *LevelCheckpointAdd(Vector2 pos) {

    LevelEntity *newCheckpoint = (LevelEntity *) MemAlloc(sizeof(LevelEntity));

    Sprite sprite = SPRITES->LevelCheckpoint;
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

void LevelExitAdd(Vector2 pos) {

    LevelEntity *newExit = (LevelEntity *) MemAlloc(sizeof(LevelEntity));

    Sprite sprite = SPRITES->LevelEndOrb;
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

void LevelExitCheckAndAdd(Vector2 pos) {
    
    Rectangle hitbox = SpriteHitboxFromMiddle(SPRITES->LevelEndOrb, pos);

    if (LevelCheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add level exit, collision with entity.");
        return;
    }

    // Currently only one level exit is supported, but this should change in the future.
    if (LEVEL_STATE->exitNode)
        LinkedListDestroyNode(&LEVEL_STATE->listHead, LEVEL_STATE->exitNode);
    
    LevelExitAdd({ hitbox.x, hitbox.y });
}

void LevelTextboxAdd(Vector2 pos, int textId) {

    LevelEntity *newTextbox = (LevelEntity *) MemAlloc(sizeof(LevelEntity));

    Sprite sprite = SPRITES->TextboxButton;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newTextbox->components = LEVEL_IS_TEXTBOX;
    newTextbox->hitbox = hitbox;
    newTextbox->origin = pos;
    newTextbox->sprite = sprite;
    newTextbox->layer = -1;
    newTextbox->isFacingRight = true;
    newTextbox->textId = textId;

    LinkedListAdd(&LEVEL_STATE->listHead, newTextbox);

    TraceLog(LOG_TRACE, "Added textbox button to level (x=%.1f, y=%.1f)",
                newTextbox->hitbox.x, newTextbox->hitbox.y);
}

void LevelTextboxCheckAndAdd(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(SPRITES->TextboxButton, pos);

    if (LevelCheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add textbox button, collision with entity.");
        return;
    }

    pos = RectangleGetPos(hitbox);
    TextInputCallback *callback = new TextInputCallback([pos] (std::string input) { 
                                                            createTextboxFromIdInput(pos, input);
                                                        });
    Input::GetTextInput(callback);
}

LevelEntity *LevelGetGroundBeneath(LevelEntity *entity) {

    return getGroundBeneath(entity->hitbox, entity);    
}

LevelEntity *LevelGetGroundBeneathHitbox(Rectangle hitbox) {

    return getGroundBeneath(hitbox, 0);
}

void LevelEntityDestroy(ListNode *node) {

    if (node == LEVEL_STATE->exitNode) LEVEL_STATE->exitNode = 0;

    LevelEntity *entity = (LevelEntity *) node->item;

    if (entity == LEVEL_STATE->checkpoint) LEVEL_STATE->checkpoint = 0;

    DebugEntityStop(entity);

    LinkedListDestroyNode(&LEVEL_STATE->listHead, node);

    TraceLog(LOG_TRACE, "Destroyed level entity.");
}

LevelEntity *LevelEntityGetAt(Vector2 pos) {

    ListNode *node = getEntityNodeAtPos(pos);

    if (!node) return 0;

    return (LevelEntity *) node->item;
}

void LevelEntityRemoveAt(Vector2 pos) {

    // TODO This function is too big and should be broken up
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

            LevelEntityDestroy(
                LinkedListGetNode(LEVEL_STATE->listHead, selectedEntity));

next_selected_node:
            selectedNode = next;
        }

        EditorSelectionCancel();

    } else {

        LevelEntityDestroy(node);
    }
}

void LevelPauseToggle() {

    if (LEVEL_STATE->isPaused) {

        if (PLAYER_ENTITY && PLAYER_ENTITY->isDead) {
            PlayerContinue();
        }

        LEVEL_STATE->isPaused = false;

    } else {
        LEVEL_STATE->isPaused = true;
    }
}

void LevelTick() {

    if (GAME_STATE->waitingForTextInput) return;

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (LEVEL_STATE->concludedAgo != -1 &&
        GetTime() - LEVEL_STATE->concludedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        leaveLevel();

        LEVEL_STATE->concludedAgo = -1;

        return;
    }

    if (!LEVEL_STATE->isPaused && !EDITOR_STATE->isEnabled)
        tickAllEntities();

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

    return LevelCheckCollisionWithAnythingElse(hitbox, 0);
}

bool LevelCheckCollisionWithAnythingElse(Rectangle hitbox, ListNode *entityListHead) {

    ListNode *node = LEVEL_STATE->listHead;

    while (node != 0) {
    
        LevelEntity *entity = (LevelEntity *) node->item;

        Rectangle entitysOrigin = {
                                        entity->origin.x,       entity->origin.y,
                                        entity->hitbox.width,   entity->hitbox.height
                                    };

        if (CheckCollisionRecs(hitbox, entitysOrigin) ||
            (!(entity->isDead) && CheckCollisionRecs(hitbox, entity->hitbox))) {

            ListNode *excludedNode = entityListHead;
            while (excludedNode != 0) {
                if (excludedNode->item == entity) goto next_node;
                excludedNode = excludedNode->next;
            }

            return true;
        }

next_node:
        node = node->next;

    }

    return false;
}

void LevelSave() {
    PersistenceLevelSave(LEVEL_STATE->levelName);
}

void LevelLoadNew() {

    LevelLoad((char *) LEVEL_BLUEPRINT_NAME);

    PLAYER_ENTITY->origin = PLAYERS_ORIGIN;
    PLAYER_ENTITY->hitbox.x = PLAYER_ENTITY->origin.x;
    PLAYER_ENTITY->hitbox.y = PLAYER_ENTITY->origin.y;

    strcpy(LEVEL_STATE->levelName, NEW_LEVEL_NAME);
}

Rectangle LevelEntityOriginHitbox(LevelEntity *entity) {

    return {
        entity->origin.x, entity->origin.y,
        entity->hitbox.width, entity->hitbox.height
    };
}

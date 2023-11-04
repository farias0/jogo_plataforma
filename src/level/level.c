#include <raylib.h>
#include "stdlib.h"
#include <stdio.h>
#include <string.h>

#include "level.h"
#include "../linked_list.h"
#include "../core.h"
#include "../camera.h"
#include "../persistence.h"
#include "../overworld.h"


// The difference between the y of the hitbox and the ground to be considered "on the ground"
#define ON_THE_GROUND_Y_TOLERANCE 5

// TODO seek an unused level name, i.e. "new_level_2.lvl"
// THe name of a new level
#define DEFAULT_NEW_LEVEL_NAME "new_level.lvl"


ListNode *LEVEL_LIST_HEAD = 0;

static Vector2 playersStartingPosition =  { SCREEN_WIDTH/5, 300 };

static ListNode *levelExitNode = 0;


// Searches for an entity that's not the player
// in a given position and returns its node, or 0 if not found.
static ListNode *getNodeOfEntityOn(Vector2 pos) {

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (!(entity->components & LEVEL_IS_PLAYER) &&
            CheckCollisionPointRec(pos, entity->hitbox)) {

                return node;
            }

        node = node->next;
    };

    return 0;
}

void LevelInitialize(char *levelName) {

    GameStateReset();
    STATE->mode = MODE_IN_LEVEL;

    LinkedListRemoveAll(&LEVEL_LIST_HEAD);
    LEVEL_PLAYER = 0;

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

    TraceLog(LOG_INFO, "Level initialized: %s.", levelName);
}

void LevelExitAdd(Vector2 pos) {

    LevelEntity *newExit = MemAlloc(sizeof(LevelEntity));

    Sprite sprite = LevelEndOrbSprite;
    Rectangle hitbox = SpriteHitboxFromMiddle(sprite, pos);

    newExit->components = LEVEL_IS_EXIT;
    newExit->hitbox = hitbox;
    newExit->sprite = sprite;
    newExit->isFacingRight = true;

    ListNode *node = MemAlloc(sizeof(ListNode));
    node->item = newExit;
    LinkedListAdd(&LEVEL_LIST_HEAD, node);

    levelExitNode = node;

    TraceLog(LOG_TRACE, "Added exit to level (x=%.1f, y=%.1f)",
                newExit->hitbox.x, newExit->hitbox.y);
}

void LevelExitCheckAndAdd(Vector2 pos) {
    
    ListNode *node = LEVEL_LIST_HEAD;
    Rectangle hitbox = SpriteHitboxFromMiddle(LevelEndOrbSprite, pos);

    while (node != 0) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity->components & LEVEL_IS_SCENARIO &&
            CheckCollisionRecs(hitbox, entity->hitbox)) {

                TraceLog(LOG_DEBUG, "Couldn't place level exit, collision with entity on x=%1.f, y=%1.f.",
                    entity->hitbox.x, entity->hitbox.y);
                
                return;
            }

        node = node->next;
    }

    // Currently only one level exit is supported, but this should change in the future.
    if (levelExitNode) LinkedListRemove(&LEVEL_LIST_HEAD, levelExitNode);
    
    LevelExitAdd(pos);
}

Vector2 LevelGetPlayerStartingPosition() {
    return playersStartingPosition;
}

LevelEntity *LevelGetGroundBeneath(LevelEntity *entity) {

    int entitysFoot = entity->hitbox.y + entity->hitbox.height;

    ListNode *node = LEVEL_LIST_HEAD;

    while (node != 0) {

        LevelEntity *possibleGround = (LevelEntity *)node->item;

        if (possibleGround != entity &&
            possibleGround->components & LEVEL_IS_GROUND &&

            // If x is within the possible ground
            possibleGround->hitbox.x < (entity->hitbox.x + entity->hitbox.width) &&
            entity->hitbox.x < (possibleGround->hitbox.x + possibleGround->hitbox.width) &&

            // If y is RIGHT above the possible ground
            abs((int) (possibleGround->hitbox.y - entitysFoot)) <= 
                ON_THE_GROUND_Y_TOLERANCE) {
                
                return possibleGround;
            } 

        node = node->next;
    }

    return 0;    
}

void LevelEntityRemoveAt(Vector2 pos) {

    ListNode *node = getNodeOfEntityOn(pos);

    if (node) {
        LinkedListRemove(&LEVEL_LIST_HEAD, node);
        TraceLog(LOG_TRACE, "Removed level entity.");
    }
}

void LevelTick() {

    ListNode *node = LEVEL_LIST_HEAD;
    ListNode *next;

    while (node != 0) {

        next = node->next;

        LevelEntity *entity = (LevelEntity *)node->item;

        if (entity->components & LEVEL_IS_ENEMY) LevelEnemyTick(node);
        else if (entity->components & LEVEL_IS_PLAYER) LevelPlayerTick();

        node = next;
    }

    CameraTick();
}

void LevelSave() {
    PersistenceLevelSave(STATE->loadedLevel);
}

void LevelLoadNew() {
    LevelInitialize(NEW_LEVEL_NAME);
    strcpy(STATE->loadedLevel, DEFAULT_NEW_LEVEL_NAME);
}

void LevelPlayerSetStartingPos(Vector2 pos) {
    playersStartingPosition = pos;
}

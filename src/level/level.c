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

static char currentLoadedLevel[LEVEL_NAME_BUFFER_SIZE] = "";

static Vector2 playersStartingPosition =  { SCREEN_WIDTH/5, 300 };


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

    if (!PersistenceLevelLoad(levelName)) {
        OverworldInitialize();
        return;
    }

    strncpy(currentLoadedLevel, levelName, LEVEL_NAME_BUFFER_SIZE);

    EditorSync();

    TraceLog(LOG_INFO, "Level initialized: %s.", levelName);
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
    PersistenceLevelSave(currentLoadedLevel);
}

void LevelLoadNew() {
    LevelInitialize(NEW_LEVEL_NAME);
    strncpy(currentLoadedLevel, DEFAULT_NEW_LEVEL_NAME, LEVEL_NAME_BUFFER_SIZE);
}

void LevelPlayerSetStartingPos(Vector2 pos) {
    playersStartingPosition = pos;
}

#include <raylib.h>
#include "stdlib.h"

#include <stdio.h>

#include "level.h"
#include "../linked_list.h"
#include "../core.h"
#include "../camera.h"
#include "../files.h"


// The difference between the y of the hitbox and the ground to be considered "on the ground"
#define ON_THE_GROUND_Y_TOLERANCE 5


ListNode *LEVEL_LIST_HEAD = 0;
const Dimensions LEVEL_GRID = (Dimensions){ 32, 32 };

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

static bool levelLoad() {

    FileData filedata = FileLoad(sizeof(LevelEntity));

    if (!filedata.itemCount) {
        TraceLog(LOG_ERROR, "Could not load level.");
        // TODO print something to screen
        return false;
    }

    LevelEntity *data = (LevelEntity *) filedata.data;

    for (size_t i = 0; i < filedata.itemCount; i++) {

        Vector2 pos = (Vector2){ data[i].hitbox.x, data[i].hitbox.y };
        
        if (data[i].components & LEVEL_IS_PLAYER) {
            LevelPlayerInitialize(pos);
            playersStartingPosition = (Vector2){ LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y }; // TODO replace with 'origin' level entity param
        }

        else if (data[i].components & LEVEL_IS_ENEMY)
            LevelEnemyAdd(pos);
        
        else if (data[i].components & LEVEL_IS_SCENARIO)
            LevelBlockAdd(pos);
        
        else
            TraceLog(LOG_ERROR, "Unknow entity type found when loading level, components=%d."); 
    }

    MemFree(data);

    TraceLog(LOG_INFO, "Level loaded.");
    // TODO print something to screen

    return true;
}

void LevelInitialize() {

    GameStateReset();
    STATE->mode = MODE_IN_LEVEL;

    LinkedListRemoveAll(&LEVEL_LIST_HEAD);

    if (!levelLoad()) {
        GameModeToggle();
        return;
    }

    EditorSync();

    TraceLog(LOG_INFO, "Level initialized.");
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
        TraceLog(LOG_DEBUG, "Removed level entity.");
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

    size_t itemCount = LinkedListCountNodes(LEVEL_LIST_HEAD);
    LevelEntity data[itemCount];

    TraceLog(LOG_DEBUG,
        "Saving level... (struct size=%d, item count=%d)", sizeof(LevelEntity), itemCount);

    ListNode *node = LEVEL_LIST_HEAD;
    for (size_t i = 0; i < itemCount; i++) {
        data[i] = *((LevelEntity *) node->item);
        node = node->next;
    }

    FileData filedata = (FileData){
        &data,
        sizeof(LevelEntity),
        itemCount
    };

    if (!FileSave(filedata)) {
        TraceLog(LOG_ERROR, "Could not save level.");
        // TODO print something to screen
        return;
    }

    TraceLog(LOG_INFO, "Level saved.");
    // TODO print something to screen
}

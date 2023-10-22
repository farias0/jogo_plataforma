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

static void levelLoad() {

    const float floorHeight = 600;

    LevelBlockAdd((Rectangle){ 0, floorHeight, BlockSprite.sprite.width*25, BlockSprite.sprite.height*5 });
    
    float x = BlockSprite.sprite.width*30;
    float width = BlockSprite.sprite.width*10;
    LevelBlockAdd((Rectangle){ x, floorHeight, width, BlockSprite.sprite.height*5 });
    LevelEnemyAdd((Vector2){ x + (width / 2), 200 });
    
    LevelBlockAdd((Rectangle){ BlockSprite.sprite.width*45, floorHeight-80, BlockSprite.sprite.width*10, BlockSprite.sprite.height*2 });
    LevelBlockAdd((Rectangle){ BlockSprite.sprite.width*40, floorHeight-200, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });
    LevelBlockAdd((Rectangle){ BlockSprite.sprite.width*45, floorHeight-320, BlockSprite.sprite.width*5, BlockSprite.sprite.height*1 });

    TraceLog(LOG_INFO, "Level loaded.");
}

void LevelInitialize() {

    GameStateReset();
    STATE->mode = MODE_IN_LEVEL;

    LinkedListRemoveAll(&LEVEL_LIST_HEAD);
    LevelPlayerInitialize(playersStartingPosition);
    levelLoad();

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

    size_t itemsCount = LinkedListCountNodes(LEVEL_LIST_HEAD);
    LevelEntity data[itemsCount];

    TraceLog(LOG_DEBUG,
        "Saving level... (struct size=%d, items count=%d)", sizeof(LevelEntity), itemsCount);

    ListNode *node = LEVEL_LIST_HEAD;
    for (size_t i = 0; i < itemsCount; i++) {
        data[i] = *((LevelEntity *) node->item);
        node = node->next;
    }

    if (!FilesSave(&data, sizeof(LevelEntity), itemsCount)) {
        TraceLog(LOG_ERROR, "Could not save level.");
        // TODO print something to screen
        return;
    }

    TraceLog(LOG_INFO, "Level saved.");
    // TODO print something to screen
}

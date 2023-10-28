#include <stdint.h>
#include <stddef.h>

#include "persistence.h"
#include "linked_list.h"
#include "level/level.h"
#include "files.h"
#include "render.h"


static char *LEVELS_DIR = "../levels/";

static char *LEVEL_NAME = "my_level.lvl";

static char *DEFAULT_NEW_LEVEL_NAME = "default_new_level.lvl";


typedef enum LevelEntityType {
    LEVEL_ENTITY_PLAYER,
    LEVEL_ENTITY_ENEMY,
    LEVEL_ENTITY_BLOCK
} LevelEntityType;

typedef struct PersistenceLevelEntity {
    uint16_t entityType;
    uint32_t x;
    uint32_t y;
} PersistenceLevelEntity;


static char *getFullPath(char *filename) {

    char *path = MemAlloc(sizeof(char) * 100);
    
    strcat(path, LEVELS_DIR);
    strcat(path, filename);
    
    return path;
}

void PersistenceLevelSave() {

    size_t levelItemCount = LinkedListCountNodes(LEVEL_LIST_HEAD);
    size_t saveItemCount = levelItemCount;
    size_t entitySize = sizeof(PersistenceLevelEntity);
    PersistenceLevelEntity *data = MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving level... (struct size=%d, level item count=%d)", entitySize, levelItemCount);

    ListNode *node = LEVEL_LIST_HEAD;
    for (size_t i = 0; i < levelItemCount; ) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity->components & LEVEL_IS_PLAYER)           data[i].entityType = LEVEL_ENTITY_PLAYER;
        else if (entity->components & LEVEL_IS_ENEMY)       data[i].entityType = LEVEL_ENTITY_ENEMY;
        else if (entity->components & LEVEL_IS_SCENARIO)    data[i].entityType = LEVEL_ENTITY_BLOCK;
        else { 
            TraceLog(LOG_WARNING, "Unknow entity type found when serializing level, components=%d. Skipping it...");
            saveItemCount--;
            goto skip_entity; 
        }
        
        data[i].x = (uint32_t) entity->hitbox.x;
        data[i].y = (uint32_t) entity->hitbox.y;

        i++;

skip_entity:
        node = node->next;
    }

    FileData filedata = (FileData){ data, entitySize, saveItemCount };

    char *filepath = getFullPath(LEVEL_NAME);

    if (FileSave(filepath, filedata)) {
        TraceLog(LOG_INFO, "Level saved.");
        RenderPrintSysMessage("Level saved.");
    } else {
        TraceLog(LOG_ERROR, "Could not save level.");
        RenderPrintSysMessage("Could not save level.");
    }

    MemFree(data);

    return;
}

bool PersistenceLevelLoad() {

    char *filepath = getFullPath(LEVEL_NAME);
    FileData filedata = FileLoad(filepath, sizeof(PersistenceLevelEntity));

    if (!filedata.itemCount) {
        TraceLog(LOG_ERROR, "Could not load level.");
        RenderPrintSysMessage("Could not load level.");
        return false;
    }

    PersistenceLevelEntity *data = (PersistenceLevelEntity *) filedata.data;

    for (size_t i = 0; i < filedata.itemCount; i++) {

        Vector2 pos = (Vector2){
            (float) data[i].x,
            (float) data[i].y    
        };

        switch (data[i].entityType) {
        
        case LEVEL_ENTITY_PLAYER:
            LevelPlayerInitialize(pos); break;
        case LEVEL_ENTITY_ENEMY:
            LevelEnemyAdd(pos); break;
        case LEVEL_ENTITY_BLOCK:
            LevelBlockAdd(pos); break;
        default:
            TraceLog(LOG_ERROR, "Unknow entity type found when desserializing level, type=%d.", data[i].entityType); 
        }
    }

    MemFree(data);

    TraceLog(LOG_TRACE, "Level loaded.");

    return true;
}

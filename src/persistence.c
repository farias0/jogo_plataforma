#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "persistence.h"
#include "linked_list.h"
#include "level/level.h"
#include "files.h"
#include "render.h"


#define LEVELS_DIR_NAME "levels"
#define LEVELS_DIR "../" LEVELS_DIR_NAME "/"
#define LEVEL_FILE_EXTENSION ".lvl"

#define LEVEL_PATH_BUFFER_SIZE LEVEL_NAME_BUFFER_SIZE + 20


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


static void getLevelPath(char *pathBuffer, char *levelName) {
    
    strncat(pathBuffer, LEVELS_DIR, LEVEL_PATH_BUFFER_SIZE);
    strncat(pathBuffer, levelName, LEVEL_PATH_BUFFER_SIZE);
}

void PersistenceLevelSave(char *levelName) {

    size_t levelItemCount = LinkedListCountNodes(LEVEL_LIST_HEAD);
    size_t saveItemCount = levelItemCount;
    size_t entitySize = sizeof(PersistenceLevelEntity);
    PersistenceLevelEntity *data = MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving level %s... (struct size=%d, level item count=%d)",
                        levelName, entitySize, levelItemCount);

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

    char *levelPath = MemAlloc(sizeof(char) * LEVEL_PATH_BUFFER_SIZE);
    getLevelPath(levelPath, levelName);

    if (FileSave(levelPath, filedata)) {
        TraceLog(LOG_INFO, "Level saved: %s.", levelName);
        RenderPrintSysMessage("Fase salva.");
    } else {
        TraceLog(LOG_ERROR, "Could not save level %s.", levelName);
        RenderPrintSysMessage("Erro salvando fase.");
    }

    MemFree(data);
    MemFree(levelPath);

    return;
}

bool PersistenceLevelLoad(char *levelName) {

    char *levelPath = MemAlloc(sizeof(char) * LEVEL_PATH_BUFFER_SIZE);
    getLevelPath(levelPath, levelName);

    FileData filedata = FileLoad(levelPath, sizeof(PersistenceLevelEntity));

    if (!filedata.itemCount) {
        TraceLog(LOG_ERROR, "Could not load level %s.", levelName);
        RenderPrintSysMessage("Erro carregando fase.");
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
    MemFree(levelPath);

    TraceLog(LOG_TRACE, "Level loaded: %s.", levelName);

    return true;
}

bool PersistenceGetDroppedLevelName(char *nameBuffer) {
    
    FilePathList fileList = LoadDroppedFiles();
    bool result = false;

    if (fileList.count > 1) {
        TraceLog(LOG_ERROR, "Multiple files dropped. Ignoring them.");
        goto return_result;
    }

    char *filePath = fileList.paths[0];

    // Ideally we'd support loading levels from anywhere,
    // but this would involve rewriting the InitializeLevel logic.
    // ATTENTION: It presumes the working dir is next to the 'levels' dir (i.e., it's the 'build' dir)
    char *fileDir = GetDirectoryPath(filePath);
    char *projectRootPath = GetPrevDirectoryPath(GetWorkingDirectory());
    if (strcmp(projectRootPath, GetPrevDirectoryPath(fileDir)) != 0 ||
        strcmp(GetFileName(fileDir), LEVELS_DIR_NAME) != 0) {

            TraceLog(LOG_ERROR, "Dropped file is not on 'levels' directory.");
            RenderPrintSysMessage("Arquivo não é parte do jogo");
            goto return_result;
    }

    if (strcmp(GetFileExtension(filePath), LEVEL_FILE_EXTENSION) != 0) {
        TraceLog(LOG_ERROR, "Dropped file extension is not %s. Ignoring it",
                    LEVEL_FILE_EXTENSION);
        RenderPrintSysMessage("Arquivo não é fase");
        goto return_result;
    }

    char *fileName = GetFileName(filePath);

    if (strcmp(fileName, NEW_LEVEL_NAME) == 0 ||
        strlen(fileName) > LEVEL_NAME_BUFFER_SIZE) {

            TraceLog(LOG_ERROR, "Dropped file has invalid level name %s.",
                        fileName);
            RenderPrintSysMessage("Nome de fase proibido");
            goto return_result;
    }

    strncpy(nameBuffer, fileName, LEVEL_NAME_BUFFER_SIZE);
    result = true;

return_result:
    UnloadDroppedFiles(fileList);
    return result;
}

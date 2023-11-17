#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "persistence.h"
#include "linked_list.h"
#include "level/level.h"
#include "files.h"
#include "render.h"
#include "overworld.h"


#define PERSISTENCE_DIR_NAME            "levels"
#define PERSISTENCE_DIR                 "../" PERSISTENCE_DIR_NAME "/"
#define PERSISTENCE_DIR_BUFFER_SIZE     20

#define LEVEL_FILE_EXTENSION            ".lvl"
#define LEVEL_PATH_BUFFER_SIZE          LEVEL_NAME_BUFFER_SIZE + PERSISTENCE_DIR_BUFFER_SIZE

#define OW_FILE_NAME                    "overworld.ow"
#define OW_PATH_BUFFER_SIZE             20 + PERSISTENCE_DIR_BUFFER_SIZE


typedef enum LevelEntityType {
    LEVEL_ENTITY_PLAYER,
    LEVEL_ENTITY_ENEMY,
    LEVEL_ENTITY_BLOCK,
    LEVEL_ENTITY_EXIT,
} LevelEntityType;

typedef struct PersistenceLevelEntity {
    uint16_t entityType;
    uint32_t originX;
    uint32_t originY;
} PersistenceLevelEntity;

typedef struct PersistenceOverworldEntity {
    char        levelName[LEVEL_NAME_BUFFER_SIZE];
    uint32_t    posX;
    uint32_t    posY;
    uint16_t    tileType;
    int32_t     rotation;
} PersistenceOverworldEntity;


static void getFilePath(char *pathBuffer, size_t bufferSize, char *fileName) {
    
    strncat(pathBuffer, PERSISTENCE_DIR, bufferSize);
    strncat(pathBuffer, fileName, bufferSize);
}

void PersistenceLevelSave(char *levelName) {

    size_t levelItemCount = LinkedListCountNodes(LEVEL_LIST_HEAD);
    size_t saveItemCount = levelItemCount;
    size_t entitySize = sizeof(PersistenceLevelEntity);
    PersistenceLevelEntity *data = MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving level %s... (struct size=%d, level item count=%d)",
                        levelName, entitySize, levelItemCount);

    ListNode *node = LEVEL_LIST_HEAD;
    for (size_t i = 0; i < saveItemCount; ) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity->components & LEVEL_IS_PLAYER)           data[i].entityType = LEVEL_ENTITY_PLAYER;
        else if (entity->components & LEVEL_IS_ENEMY)       data[i].entityType = LEVEL_ENTITY_ENEMY;
        else if (entity->components & LEVEL_IS_SCENARIO)    data[i].entityType = LEVEL_ENTITY_BLOCK;
        else if (entity->components & LEVEL_IS_EXIT)        data[i].entityType = LEVEL_ENTITY_EXIT;
        else { 
            TraceLog(LOG_WARNING, "Unknow entity type found when serializing level, components=%d. Skipping it...");
            saveItemCount--;
            goto skip_entity; 
        }
        
        data[i].originX = (uint32_t) entity->origin.x;
        data[i].originY = (uint32_t) entity->origin.y;

        i++;

skip_entity:
        node = node->next;
    }

    FileData filedata = (FileData){ data, entitySize, saveItemCount };

    char *levelPath = MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);

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

    char *levelPath = MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);

    FileData filedata = FileLoad(levelPath, sizeof(PersistenceLevelEntity));

    if (!filedata.itemCount) {
        TraceLog(LOG_ERROR, "Could not load level %s.", levelName);
        RenderPrintSysMessage("Erro carregando fase.");
        return false;
    }

    PersistenceLevelEntity *data = (PersistenceLevelEntity *) filedata.data;

    for (size_t i = 0; i < filedata.itemCount; i++) {

        Vector2 origin = (Vector2){
            (float) data[i].originX,
            (float) data[i].originY    
        };

        switch (data[i].entityType) {
        
        case LEVEL_ENTITY_PLAYER:
            LevelPlayerInitialize(origin); break;
        case LEVEL_ENTITY_ENEMY:
            LevelEnemyAdd(origin); break;
        case LEVEL_ENTITY_BLOCK:
            LevelBlockAdd(origin); break;
        case LEVEL_ENTITY_EXIT:
            LevelExitAdd(origin); break;
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
    const char *fileDir = GetDirectoryPath(filePath);
    const char *projectRootPath = GetPrevDirectoryPath(GetWorkingDirectory());
    if (strcmp(projectRootPath, GetPrevDirectoryPath(fileDir)) != 0 ||
        strcmp(GetFileName(fileDir), PERSISTENCE_DIR_NAME) != 0) {

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

    const char *fileName = GetFileName(filePath);

    if (strcmp(fileName, NEW_LEVEL_NAME) == 0 ||
        strlen(fileName) > LEVEL_NAME_BUFFER_SIZE) {

            TraceLog(LOG_ERROR, "Dropped file has invalid level name %s.",
                        fileName);
            RenderPrintSysMessage("Nome de fase proibido");
            goto return_result;
    }

    strcpy(nameBuffer, fileName);
    result = true;

return_result:
    UnloadDroppedFiles(fileList);
    return result;
}

void PersistenceOverworldSave() {

    size_t owItemCount = LinkedListCountNodes(OW_LIST_HEAD);
    size_t saveItemCount = owItemCount;
    size_t entitySize = sizeof(PersistenceOverworldEntity);
    PersistenceOverworldEntity *data = MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving overworld... (struct size=%d, level item count=%d)",
                        entitySize, owItemCount);

    ListNode *node = OW_LIST_HEAD;
    for (size_t i = 0; i < saveItemCount; ) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        // Saves only OW_NOT_TILEs
        if (entity->tileType == OW_NOT_TILE) {
            saveItemCount--;
            goto skip_entity;
        }
        
        data[i].tileType =          entity->tileType;
        data[i].posX =              (uint32_t) entity->gridPos.x;
        data[i].posY =              (uint32_t) entity->gridPos.y;
        data[i].rotation =          (int32_t) entity->sprite.rotation;

        if (entity->levelName) {
            strcpy(data[i].levelName, entity->levelName);
        } else {
            data[i].levelName[0] = '\0';
        }

        i++;

skip_entity:
        node = node->next;
    }

    FileData filedata = (FileData){ data, entitySize, saveItemCount };

    char *filePath = MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, OW_FILE_NAME);

    if (FileSave(filePath, filedata)) {
        TraceLog(LOG_INFO, "Overworld saved.");
        RenderPrintSysMessage("Mundo salvo.");
    } else {
        TraceLog(LOG_ERROR, "Could not save overworld.");
        RenderPrintSysMessage("Erro salvando mundo.");
    }

    MemFree(data);
    MemFree(filePath);

    return;
}

bool PersistenceOverworldLoad() {

    char *filePath = MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, OW_FILE_NAME);

    FileData fileData = FileLoad(filePath, sizeof(PersistenceOverworldEntity));

    if (!fileData.itemCount) {
        TraceLog(LOG_ERROR, "Could not overworld.");
        RenderPrintSysMessage("Erro carregando mundo.");
        return false;
    }

    PersistenceOverworldEntity *data = (PersistenceOverworldEntity *) fileData.data;

    for (size_t i = 0; i < fileData.itemCount; i++) {

        Vector2 pos = (Vector2){
            (float) data[i].posX,
            (float) data[i].posY    
        };

        OverworldEntity *newTile = 
            OverworldTileAdd(pos, (OverworldTileType) data[i].tileType, (int) data[i].rotation);

        if (data[i].levelName[0] != '\0') {
            newTile->levelName = MemAlloc(LEVEL_NAME_BUFFER_SIZE);
            strncpy(newTile->levelName, data[i].levelName, LEVEL_NAME_BUFFER_SIZE);
        }
    }

    MemFree(data);
    MemFree(filePath);

    TraceLog(LOG_TRACE, "Overworld loaded.");

    return true;
}

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "persistence.hpp"
#include "linked_list.hpp"
#include "level/level.hpp"
#include "level/player.hpp"
#include "level/enemy.hpp"
#include "level/block.hpp"
#include "level/powerups.hpp"
#include "files.hpp"
#include "render.hpp"
#include "overworld.hpp"


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
    LEVEL_ENTITY_ACID,
    LEVEL_ENTITY_EXIT,
    LEVEL_ENTITY_GLIDE
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
    char        isTileUnderCursor;
} PersistenceOverworldEntity;


static void getFilePath(char *pathBuffer, size_t bufferSize, char *fileName) {
    
    strncat(pathBuffer, PERSISTENCE_DIR, bufferSize);
    strncat(pathBuffer, fileName, bufferSize);
}

void PersistenceLevelSave(char *levelName) {

    size_t levelItemCount = LinkedListCountNodes(LEVEL_STATE->listHead);
    size_t saveItemCount = levelItemCount;
    size_t entitySize = sizeof(PersistenceLevelEntity);
    PersistenceLevelEntity *data = (PersistenceLevelEntity *) MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving level %s... (struct size=%d, level item count=%d)",
                        levelName, entitySize, levelItemCount);

    ListNode *node = LEVEL_STATE->listHead;
    for (size_t i = 0; i < saveItemCount; ) {

        LevelEntity *entity = (LevelEntity *) node->item;

        if (entity->components & LEVEL_IS_PLAYER)           data[i].entityType = LEVEL_ENTITY_PLAYER;
        else if (entity->components & LEVEL_IS_ENEMY)       data[i].entityType = LEVEL_ENTITY_ENEMY;

        else if (entity->components & LEVEL_IS_SCENARIO && entity->components & LEVEL_IS_DANGER)
                                                            data[i].entityType = LEVEL_ENTITY_ACID;

        else if (entity->components & LEVEL_IS_SCENARIO && !(entity->components & LEVEL_IS_DANGER))
                                                            data[i].entityType = LEVEL_ENTITY_BLOCK;

        else if (entity->components & LEVEL_IS_EXIT)        data[i].entityType = LEVEL_ENTITY_EXIT;
        else if (entity->components & LEVEL_IS_GLIDE)       data[i].entityType = LEVEL_ENTITY_GLIDE;
        else { 
            TraceLog(LOG_WARNING, "Unknow entity type found when serializing level, components=%d. Skipping it...");
            saveItemCount--;
            goto skip_entity; 
        }
        
        memcpy(&data[i].originX,    &entity->origin.x,  sizeof(uint32_t));
        memcpy(&data[i].originY,    &entity->origin.y,  sizeof(uint32_t));

        i++;

skip_entity:
        node = node->next;
    }

    FileData filedata = { data, entitySize, saveItemCount };

    char *levelPath = (char *) MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);

    if (FileSave(levelPath, filedata)) {
        TraceLog(LOG_INFO, "Level saved: %s.", levelName);
        RenderPrintSysMessage((char *) "Fase salva.");
    } else {
        TraceLog(LOG_ERROR, "Could not save level %s.", levelName);
        RenderPrintSysMessage((char *) "Erro salvando fase.");
    }

    MemFree(data);
    MemFree(levelPath);

    return;
}

bool PersistenceLevelLoad(char *levelName) {

    char *levelPath = (char *) MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);

    FileData filedata = FileLoad(levelPath, sizeof(PersistenceLevelEntity));

    if (!filedata.itemCount) {
        TraceLog(LOG_ERROR, "Could not load level %s.", levelName);
        RenderPrintSysMessage((char *) "Erro carregando fase.");
        return false;
    }

    PersistenceLevelEntity *data = (PersistenceLevelEntity *) filedata.data;

    for (size_t i = 0; i < filedata.itemCount; i++) {

        Vector2 origin;
        memcpy(&origin.x,   &data[i].originX,   sizeof(uint32_t));
        memcpy(&origin.y,   &data[i].originY,   sizeof(uint32_t));
        
        switch (data[i].entityType) {
        
        case LEVEL_ENTITY_PLAYER:
            PlayerInitialize(origin); break;
        case LEVEL_ENTITY_ENEMY:
            EnemyAdd(origin); break;
        case LEVEL_ENTITY_BLOCK:
            BlockAdd(origin); break;
        case LEVEL_ENTITY_ACID:
            AcidAdd(origin); break;
        case LEVEL_ENTITY_EXIT:
            LevelExitAdd(origin); break;
        case LEVEL_ENTITY_GLIDE:
            GlideAdd(origin); break;
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

    char *filePath = fileList.paths[0];

    // Ideally we'd support loading levels from anywhere,
    // but this would involve rewriting the InitializeLevel logic.
    // ATTENTION: It presumes the working dir is next to the 'levels' dir (i.e., it's the 'build' dir)
    const char *fileDir = GetDirectoryPath(filePath);
    const char *projectRootPath = GetPrevDirectoryPath(GetWorkingDirectory());
    const char *fileName = GetFileName(filePath);

    if (fileList.count > 1) {
        TraceLog(LOG_ERROR, "Multiple files dropped. Ignoring them.");
        goto return_result;
    }

    if (strcmp(projectRootPath, GetPrevDirectoryPath(fileDir)) != 0 ||
        strcmp(GetFileName(fileDir), PERSISTENCE_DIR_NAME) != 0) {

            TraceLog(LOG_ERROR, "Dropped file is not on 'levels' directory.");
            RenderPrintSysMessage((char *) "Arquivo não é parte do jogo");
            goto return_result;
    }

    if (strcmp(GetFileExtension(filePath), LEVEL_FILE_EXTENSION) != 0) {
        TraceLog(LOG_ERROR, "Dropped file extension is not %s. Ignoring it",
                    LEVEL_FILE_EXTENSION);
        RenderPrintSysMessage((char *) "Arquivo não é fase");
        goto return_result;
    }

    if (strcmp(fileName, LEVEL_BLUEPRINT_NAME) == 0 ||
        strlen(fileName) > LEVEL_NAME_BUFFER_SIZE) {

            TraceLog(LOG_ERROR, "Dropped file has invalid level name %s.",
                        fileName);
            RenderPrintSysMessage((char *) "Nome de fase proibido");
            goto return_result;
    }

    strcpy(nameBuffer, fileName);
    result = true;

return_result:
    UnloadDroppedFiles(fileList);
    return result;
}

void PersistenceOverworldSave() {

    size_t owItemCount = LinkedListCountNodes(OW_STATE->listHead);
    size_t saveItemCount = owItemCount;
    size_t entitySize = sizeof(PersistenceOverworldEntity);
    PersistenceOverworldEntity *data = (PersistenceOverworldEntity *) MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving overworld... (struct size=%d, level item count=%d)",
                        entitySize, owItemCount);

    ListNode *node = OW_STATE->listHead;
    for (size_t i = 0; i < saveItemCount; ) {

        OverworldEntity *entity = (OverworldEntity *) node->item;

        // Saves only OW_NOT_TILEs
        if (entity->tileType == OW_NOT_TILE) {
            saveItemCount--;
            goto skip_entity;
        }
        
        data[i].tileType =          entity->tileType;
        data[i].rotation =          (int32_t) entity->sprite.rotation;
        memcpy(&data[i].posX,       &entity->gridPos.x,         sizeof(uint32_t));
        memcpy(&data[i].posY,       &entity->gridPos.y,         sizeof(uint32_t));

        if (entity->levelName) {
            strcpy(data[i].levelName, entity->levelName);
        } else {
            data[i].levelName[0] = '\0';
        }

        if (entity == OW_STATE->tileUnderCursor) data[i].isTileUnderCursor = 1;

        i++;

skip_entity:
        node = node->next;
    }

    FileData filedata = { data, entitySize, saveItemCount };

    char *filePath = (char *) MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, (char *) OW_FILE_NAME);

    if (FileSave(filePath, filedata)) {
        TraceLog(LOG_INFO, "Overworld saved.");
        RenderPrintSysMessage((char *) "Mundo salvo.");
    } else {
        TraceLog(LOG_ERROR, "Could not save overworld.");
        RenderPrintSysMessage((char *) "Erro salvando mundo.");
    }

    MemFree(data);
    MemFree(filePath);

    return;
}

bool PersistenceOverworldLoad() {

    char *filePath = (char *) MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, (char *) OW_FILE_NAME);

    FileData fileData = FileLoad(filePath, sizeof(PersistenceOverworldEntity));

    if (!fileData.itemCount) {
        TraceLog(LOG_ERROR, "Could not overworld.");
        RenderPrintSysMessage((char *) "Erro carregando mundo.");
        return false;
    }

    PersistenceOverworldEntity *data = (PersistenceOverworldEntity *) fileData.data;

    for (size_t i = 0; i < fileData.itemCount; i++) {

        Vector2 pos;
        memcpy(&(pos.x), &(data[i].posX), sizeof(uint32_t));
        memcpy(&(pos.y), &(data[i].posY), sizeof(uint32_t));

        OverworldEntity *newTile = 
            OverworldTileAdd(pos, (OverworldTileType) data[i].tileType, (int) data[i].rotation);

        if (data[i].levelName[0] != '\0') {
            newTile->levelName = (char *) MemAlloc(LEVEL_NAME_BUFFER_SIZE);
            strncpy(newTile->levelName, data[i].levelName, LEVEL_NAME_BUFFER_SIZE);
        }

        if (data[i].isTileUnderCursor) OW_STATE->tileUnderCursor = newTile;
    }

    MemFree(data);
    MemFree(filePath);

    TraceLog(LOG_TRACE, "Overworld loaded.");

    return true;
}
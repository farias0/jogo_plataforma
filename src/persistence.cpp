#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sstream>

#include "persistence.hpp"
#include "linked_list.hpp"
#include "level/level.hpp"
#include "level/player.hpp"
#include "level/enemy.hpp"
#include "level/block.hpp"
#include "level/powerups.hpp"
#include "level/checkpoint.hpp"
#include "level/textbox.hpp"
#include "level/moving_platform.hpp"
#include "level/npc/princess.hpp"
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


typedef struct PersistenceLevelEntity {
    uint16_t    entityType;
    uint32_t    originX;
    uint32_t    originY;
    uint32_t    textId;
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

    std::string data = "levelname:" + std::string(levelName) + "\n";

    for (Level::Entity *entity = (Level::Entity *) Level::STATE->listHead;
        entity;
        entity = (Level::Entity *) entity->next) {

            if (!(entity->tags & Level::IS_PERSISTABLE)) continue;

            data += entity->PersitenceEntityID() + ":" + entity->PersistanceSerialize() + '\n';
    }

    char *levelPath = (char *) MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);

    Files::TextSave(levelPath, data);
    TraceLog(LOG_INFO, "Level saved: %s.", levelName);
    Render::PrintSysMessage("Fase salva.");

    MemFree(levelPath);

    return;
}

bool PersistenceLevelLoad(char *levelName) {

    char *levelPath = (char *) MemAlloc(LEVEL_PATH_BUFFER_SIZE);
    getFilePath(levelPath, LEVEL_PATH_BUFFER_SIZE, levelName);


    std::string data = Files::TextLoad(levelPath);

    std::stringstream stream(data);
    std::string line;
    while (std::getline(stream, line)) {

        try {

            size_t tagDelimiter = line.find(":");
            std::string entityTag = line.substr(0, tagDelimiter);
            std::string entityData = line.substr(tagDelimiter, line.size());

            if (entityTag == "levelname") continue; // TODO exhibit level name instead of filename

            Level::Entity *entity;
            
                                                        // There's gotta be a better way to associate (at compilation
                                                        // or initialization time) a string tag to a function pointer
            if (entityTag == PLAYER_ENTITY_ID)
                entity = Player::Initialize();
            else if (entityTag == ENEMY_ENTITY_ID)
                entity = Enemy::Add();
            else if (entityTag == BLOCK_ENTITY_ID)
                entity = Block::Add();
            else if (entityTag == ACID_BLOCK_ENTITY_ID)
                entity = AcidBlock::Add();
            else if (entityTag == EXIT_ENTITY_ID)
                entity = Level::ExitAdd();
            else if (entityTag == GLIDE_PICKUP_ENTITY_ID)
                entity = GlideAdd();
            else if (entityTag == CHECKPOINT_PICKUP_ENTITY_ID)
                entity = CheckpointPickup::Add();
            else if (entityTag == TEXTBOX_BUTTON_ENTITY_ID)
                entity = Textbox::Add();
            else if (entityTag == MOVING_PLATFORM_ENTITY_ID)
                entity = MovingPlatform::Add();
            else if (entityTag == ENEMY_DUMMY_ENTITY_ID)
                entity = EnemyDummySpike::Add();
            else if (entityTag == PRINCESS_ENTITY_ID)
                entity = Princess::Add();
            else {
                TraceLog(LOG_ERROR, "Unknow entity type found when desserializing level, entityTag=%s.", entityTag.c_str());
                continue; 
            }

            entity->PersistenceParse(entityData);

        }

        catch (const std::exception &ex) {
            TraceLog(LOG_ERROR, "Could not parse loaded level entity (%s): %s", line.c_str(), ex.what());
        }
    }

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
            Render::PrintSysMessage("Arquivo não é parte do jogo");
            goto return_result;
    }

    if (strcmp(GetFileExtension(filePath), LEVEL_FILE_EXTENSION) != 0) {
        TraceLog(LOG_ERROR, "Dropped file extension is not %s. Ignoring it",
                    LEVEL_FILE_EXTENSION);
        Render::PrintSysMessage("Arquivo não é fase");
        goto return_result;
    }

    if (strcmp(fileName, LEVEL_BLUEPRINT_NAME) == 0 ||
        strlen(fileName) > LEVEL_NAME_BUFFER_SIZE) {

            TraceLog(LOG_ERROR, "Dropped file has invalid level name %s.",
                        fileName);
            Render::PrintSysMessage("Nome de fase proibido");
            goto return_result;
    }

    strcpy(nameBuffer, fileName);
    result = true;

return_result:
    UnloadDroppedFiles(fileList);
    return result;
}

void PersistenceOverworldSave() {

    size_t owItemCount = LinkedList::CountNodes(OW_STATE->listHead);
    size_t saveItemCount = owItemCount;
    size_t entitySize = sizeof(PersistenceOverworldEntity);
    PersistenceOverworldEntity *data = (PersistenceOverworldEntity *) MemAlloc(entitySize * saveItemCount);

    TraceLog(LOG_DEBUG, "Saving overworld... (struct size=%d, level item count=%d)",
                        entitySize, owItemCount);

    OverworldEntity *entity = (OverworldEntity *) OW_STATE->listHead;
    for (size_t i = 0; i < saveItemCount; ) {

        // Saves only OW_NOT_TILEs
        if (entity->tileType == OW_NOT_TILE) {
            saveItemCount--;
            goto skip_entity;
        }
        
        data[i].tileType =          entity->tileType;
        data[i].rotation =          (int32_t) entity->rotation;
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
        entity = (OverworldEntity *) entity->next;
    }

    Files::FileData filedata = { data, entitySize, saveItemCount };

    char *filePath = (char *) MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, (char *) OW_FILE_NAME);

    if (Files::DataSave(filePath, filedata)) {
        TraceLog(LOG_INFO, "Overworld saved.");
        Render::PrintSysMessage("Mundo salvo.");
    } else {
        TraceLog(LOG_ERROR, "Could not save overworld.");
        Render::PrintSysMessage("Erro salvando mundo.");
    }

    MemFree(data);
    MemFree(filePath);

    return;
}

bool PersistenceOverworldLoad() {

    char *filePath = (char *) MemAlloc(OW_PATH_BUFFER_SIZE);
    getFilePath(filePath, OW_PATH_BUFFER_SIZE, (char *) OW_FILE_NAME);

    Files::FileData fileData = Files::DataLoad(filePath, sizeof(PersistenceOverworldEntity));

    if (!fileData.itemCount) {
        TraceLog(LOG_ERROR, "Could not overworld.");
        Render::PrintSysMessage("Erro carregando mundo.");
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

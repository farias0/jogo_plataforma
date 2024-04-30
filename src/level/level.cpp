#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <algorithm>

#include "level.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "grappling_hook.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../editor.hpp"
#include "../overworld.hpp"
#include "../debug.hpp"
#include "../input.hpp"
#include "../render.hpp"


// The difference between the y of the hitbox and the ground to be considered "on the ground"
#define ON_THE_GROUND_Y_TOLERANCE       5

// TODO seek an unused level name, i.e. "new_level_2.lvl"
// The filename of the newly created levels
#define NEW_LEVEL_NAME                  "new_level.lvl"

// The players origin by default.
#define PLAYERS_ORIGIN                  { 344, 200 };

// With how many checkpoints available the player starts
#define STARTING_CHECKPOINTS_NUMBER     1;


namespace Level {


LevelState *STATE = 0;


void resetState() {

    LinkedList::DestroyAll(&STATE->listHead);
    memset(STATE->levelName, 0, sizeof(STATE->levelName));
    STATE->isPaused = false;
    STATE->awaitingAssociation = false;
    STATE->concludedAgo = -1;
    STATE->exit = 0;
    STATE->checkpoint = 0;
    STATE->checkpointsLeft = 0;

    PLAYER = 0;

    TraceLog(LOG_INFO, "Level State initialized.");
}

void initializeState() {

    STATE = (LevelState *) MemAlloc(sizeof(LevelState));

    resetState();

    TraceLog(LOG_INFO, "Level State initialized.");
}

void leave() {
    
    Render::DisplayTextboxStop();
    
    resetState();

    OverworldLoad();

    TraceLog(LOG_TRACE, "Level left.");
}

// Searches for an entity in a given position
// and returns its node, or 0 if not found.
static LinkedList::Node *getEntityNodeAtPos(Vector2 pos) {

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        if (CheckCollisionPointRec(pos, entity->hitbox)) {

            return entity;
        }

        entity = (Entity *) entity->next;
    };

    return 0;
}

void tickAllEntities() {

    Entity *entity = (Entity *) STATE->listHead;
    Entity *next;

    while (entity != 0) {

        next = (Entity *) entity->next;

        // ATTENTION: If the 'next' entity is deleted during Tick() this will break.
        // Honestly, it's a miracle this hasn't broken so far.

        entity->Tick();
        entity = next;        
    }
}

// Searches the level for a ground immediatelly beneath the hitbox.
// Accepts an optional 'entity' reference, in case its checking for ground
// beneath an existing level entity.
static Entity *getGroundBeneath(Rectangle hitbox, Entity *entity) {

    // Virtual entity to use when checking only hitboxes
    Entity virtualEntity;
    if (!entity) {
        virtualEntity.isFacingRight = false;
        entity = &virtualEntity;
    }

    int feetHeight = hitbox.y + hitbox.height;

    Entity *possibleGround = (Entity *) STATE->listHead;
    Entity *foundGround = 0; 

    while (possibleGround != 0) {

        if (possibleGround != entity &&
            possibleGround->tags & IS_GROUND &&

            !possibleGround->IsADeadEnemy() &&

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

        possibleGround = (Entity *) possibleGround->next;
    }

    return foundGround;   
}

void Initialize() {

    initializeState();
    TraceLog(LOG_INFO, "Level system initialized.");
}

void Load(char *levelName) {

    // Gambiarra. In case a level file was dragged and there was a level loaded already
    if (PLAYER) {
        resetState();
    }

    GAME_STATE->mode = MODE_IN_LEVEL;

    if (levelName[0] == '\0') {
        EditorEmpty();
        CameraPanningReset();
        STATE->awaitingAssociation = true;
        TraceLog(LOG_INFO, "Level waiting for file drop.");
        return;
    }

    if (!PersistenceLevelLoad(levelName)) {
        leave();
        return;
    }

    STATE->checkpointsLeft = STARTING_CHECKPOINTS_NUMBER;

    strcpy(STATE->levelName, levelName);

    EditorSync();

    CameraLevelCentralizeOnPlayer();

    CameraFollow();

    Render::LevelTransitionEffectStart(
        SpritePosMiddlePoint(
            {PLAYER->hitbox.x, PLAYER->hitbox.y}, PLAYER->sprite), false);

    TraceLog(LOG_INFO, "Level loaded: %s.", levelName);
}

void GoToOverworld() {

    if (!PLAYER) {
        leave();
        return;    
    }

    if (STATE->concludedAgo != -1) return;

    CameraPanningReset();

    Render::LevelTransitionEffectStart(
        SpritePosMiddlePoint(
            {PLAYER->hitbox.x, PLAYER->hitbox.y}, PLAYER->sprite), true);

    DebugEntityStopAll();

    STATE->concludedAgo = GetTime();
}

Entity *CheckpointFlagAdd(Vector2 pos) {

    Entity *newCheckpoint = new Entity();

    Sprite *sprite = &SPRITES->LevelCheckpointFlag;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newCheckpoint->tags = 0;
    newCheckpoint->hitbox = hitbox;
    newCheckpoint->origin = pos;
    newCheckpoint->sprite = sprite;
    newCheckpoint->isFacingRight = true;
    newCheckpoint->layer = -1;

    LinkedList::AddNode(&STATE->listHead, newCheckpoint);

    TraceLog(LOG_TRACE, "Added checkpoint flag to level (x=%.1f, y=%.1f)",
                newCheckpoint->hitbox.x, newCheckpoint->hitbox.y);

    return newCheckpoint;
}

Entity *ExitAdd() {
    return ExitAdd({ 0, 0 });
}

Entity *ExitAdd(Vector2 pos) {

    Entity *newExit = new Entity();

    Sprite *sprite = &SPRITES->LevelEndOrb;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newExit->tags = IS_EXIT;
    newExit->hitbox = hitbox;
    newExit->origin = pos;
    newExit->sprite = sprite;
    newExit->isFacingRight = true;

    newExit->persistanceEntityID = "lvl_exit";

    STATE->exit = (Entity *) LinkedList::AddNode(&STATE->listHead, newExit);

    TraceLog(LOG_TRACE, "Added exit to level (x=%.1f, y=%.1f)",
                newExit->hitbox.x, newExit->hitbox.y);

    return newExit;
}

void ExitCheckAndAdd(Vector2 pos) {
    
    Rectangle hitbox = SpriteHitboxFromMiddle(&SPRITES->LevelEndOrb, pos);

    if (CheckCollisionWithAnything(hitbox)) {
        TraceLog(LOG_DEBUG, "Couldn't add level exit, collision with entity.");
        return;
    }

    // Currently only one level exit is supported, but this should change in the future.
    if (STATE->exit)
        LinkedList::DestroyNode(&STATE->listHead, STATE->exit);
    
    ExitAdd({ hitbox.x, hitbox.y });
}

Entity *GetGroundBeneath(Entity *entity) {

    return getGroundBeneath(entity->hitbox, entity);    
}

Entity *GetGroundBeneathHitbox(Rectangle hitbox) {

    return getGroundBeneath(hitbox, 0);
}

// TODO put this in the Entity's destructor
void EntityDestroy(Entity *entity) {

    if (entity == STATE->exit) STATE->exit = 0;

    if (entity == STATE->checkpoint) STATE->checkpoint = 0;

    if (PLAYER->hookLaunched)
        if (entity == PLAYER->hookLaunched->attachedTo)
            PLAYER->hookLaunched->attachedTo = 0;

    DebugEntityStop(entity);

    LinkedList::DestroyNode(&STATE->listHead, entity);

    TraceLog(LOG_TRACE, "Destroyed level entity.");
}

Entity *EntityGetAt(Vector2 pos) {

    LinkedList::Node *node = getEntityNodeAtPos(pos);

    if (!node) return 0;

    return (Entity *) node;
}

void EntityRemoveAt(Vector2 pos) {

    // TODO This function is too big and should be broken up
    // in at least two others ASAP

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        if (entity->tags & IS_PLAYER) goto next_entity;

        if (CheckCollisionPointRec(pos, EntityOriginHitbox(entity))) {
            break;
        }

        if (!entity->IsADeadEnemy() && CheckCollisionPointRec(pos, entity->hitbox)) {
            break;
        }

next_entity:
        entity = (Entity *) entity->next;
    };

    if (!entity) return;

    bool isPartOfSelection = std::find(EDITOR_STATE->selectedEntities.begin(), EDITOR_STATE->selectedEntities.end(), entity) != EDITOR_STATE->selectedEntities.end();
    if (isPartOfSelection) {

        for (auto e = EDITOR_STATE->selectedEntities.begin(); e < EDITOR_STATE->selectedEntities.end(); e++) {

            Entity *selectedEntity = (Entity *) *e;

            if (selectedEntity->tags & IS_PLAYER) continue;

            EntityDestroy(selectedEntity);
        }

        EditorSelectionCancel();

    } else {

        EntityDestroy(entity);
    }
}

void PauseToggle() {

    if (STATE->isPaused) {

        if (PLAYER && PLAYER->isDead) {
            PLAYER->Continue();
        }

        STATE->isPaused = false;

    } else {
        STATE->isPaused = true;
    }
}

void Tick() {

    if (GAME_STATE->waitingForTextInput) return;

    // TODO check if having the first check before saves on processing,
    // of if it's just redundant. 
    if (STATE->concludedAgo != -1 &&
        GetTime() - STATE->concludedAgo > LEVEL_TRANSITION_ANIMATION_DURATION) {

        leave();

        STATE->concludedAgo = -1;

        return;
    }

    if (!STATE->isPaused && !EDITOR_STATE->isEnabled)
        tickAllEntities();

    CameraTick();
}

bool CheckCollisionWithAnyEntity(Rectangle hitbox) {

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        if (!entity->IsADeadEnemy() && CheckCollisionRecs(hitbox, entity->hitbox)) {
            return true;
        }

        entity = (Entity *) entity->next;
    }

    return false;
}

bool CheckCollisionWithAnything(Rectangle hitbox) {

    return CheckCollisionWithAnythingElse(hitbox, std::vector<LinkedList::Node *>());
}

bool CheckCollisionWithAnythingElse(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore) {

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        Rectangle entitysOrigin = {
                                        entity->origin.x,       entity->origin.y,
                                        entity->hitbox.width,   entity->hitbox.height
                                    };

        if (CheckCollisionRecs(hitbox, entitysOrigin) ||
            (!entity->IsADeadEnemy() && CheckCollisionRecs(hitbox, entity->hitbox))) {

            for (auto e = entitiesToIgnore.begin(); e < entitiesToIgnore.end(); e++) {
                if (*e == entity) goto next_entity;
            }

            return true;
        }

next_entity:
        entity = (Entity *) entity->next;
    }

    return false;
}

void Save() {
    PersistenceLevelSave(STATE->levelName);
}

void LoadNew() {

    Load((char *) LEVEL_BLUEPRINT_NAME);

    PLAYER->origin = PLAYERS_ORIGIN;
    PLAYER->hitbox.x = PLAYER->origin.x;
    PLAYER->hitbox.y = PLAYER->origin.y;

    strcpy(STATE->levelName, NEW_LEVEL_NAME);
}

Rectangle EntityOriginHitbox(Entity *entity) {

    return {
        entity->origin.x, entity->origin.y,
        entity->hitbox.width, entity->hitbox.height
    };
}


void Entity::Tick() {            

    // Default entity has no tick routine
}

void Entity::Draw() {            

    Render::DrawLevelEntity(this);

    if (EDITOR_STATE->isEnabled)
        Render::DrawLevelEntityOrigin(this);
}

std::string Entity::PersistanceSerialize() {

    std::string data;
    persistanceAddValue(&data, "originX", std::to_string(origin.x));
    persistanceAddValue(&data, "originY", std::to_string(origin.y));
    return data;
}

void Entity::PersistenceParse(const std::string &data) {

    origin.x = std::stof(persistenceReadValue(data, "originX"));
    origin.y = std::stof(persistenceReadValue(data, "originY"));
    
    hitbox.x = origin.x;
    hitbox.y = origin.y;
}

} // namespace

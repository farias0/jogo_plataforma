#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <algorithm>

#include "level.hpp"
#include "player.hpp"
#include "enemy.hpp"
#include "grappling_hook.hpp"
#include "moving_platform.hpp"
#include "block.hpp"
#include "npc/npc.hpp"
#include "coin.hpp"
#include "npc/princess.hpp"
#include "powerups.hpp"
#include "checkpoint.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../editor.hpp"
#include "../overworld.hpp"
#include "../debug.hpp"
#include "../input.hpp"
#include "../render.hpp"
#include "../menu.hpp"
#include "../core.hpp"
#include "../sounds.hpp"


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

// Resets the level after the player dies and continues
void continueLevel() {

    STATE->isPaused = false;

    // Reset all the entities to their origins
    for (Entity *entity = (Entity *) STATE->listHead;
        entity != 0;
        entity = (Entity *) entity->next) {

        // Doesn't respawn enemies that are too close to the checkpoint,
        // so the player doesn't get stuck
        if (STATE->checkpoint && entity->tags & IS_ENEMY && entity->isDead) {

            const auto h = STATE->checkpoint->hitbox;
            Rectangle enlargedHitbox = { h.x - h.width,
                                            h.y - h.height,
                                            h.width * 3,
                                            h.height * 3 };

            if (CheckCollisionRecs(enlargedHitbox, entity->GetOriginHitbox())) {

                TraceLog(LOG_DEBUG, "Didn't respawn entity with tag %d, collided with checkpoint.", entity->tags);
                continue;
            }
        }

        entity->Reset();
    }

    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Level continue.");
}

void leave() {
    
    resetState();

    Sounds::StopMusic();

    OverworldLoad();

    TraceLog(LOG_TRACE, "Level left.");
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

    Entity *foundGround = 0; 

    // Virtual entity to use when checking only hitboxes
    Entity virtualEntity;
    if (!entity) {
        virtualEntity.isFacingRight = false;
        entity = &virtualEntity;
    }

    int feetHeight = hitbox.y + hitbox.height;


    for (Entity *possibleGround = (Entity *) STATE->listHead;
        possibleGround != 0;
        possibleGround = (Entity *) possibleGround->next) {

    
        if (possibleGround == entity ||
            !(possibleGround->tags & IS_GROUND) ||
            possibleGround->IsDisabled()) {

            continue;
        }


        if (// If x is within the possible ground
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
    }


    return foundGround;   
}

void Initialize() {

    initializeState();
    Block::InitializeTileMap();
    INpc::Initialize();
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

    Sounds::PlayMusic(&SOUNDS->Track1);

    Render::LevelTransitionEffectStart(
        SpritePosMiddlePoint(
            {PLAYER->hitbox.x, PLAYER->hitbox.y}, PLAYER->sprite), false);

    TraceLog(LOG_INFO, "Level loaded: %s.", levelName);
}

void GoToOverworld() {

    if (GAME_STATE->menu) DeflatePauseMenu();

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

Entity *ExitAddFromPersistence() {
    return ExitAdd({ 0, 0 });
}

Entity *ExitAdd(Vector2 pos) {

    Entity *newExit = new Entity();

    Sprite *sprite = &SPRITES->LevelEndOrb;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newExit->tags = IS_EXIT +
                    IS_PERSISTABLE;
    newExit->hitbox = hitbox;
    newExit->origin = pos;
    newExit->sprite = sprite;
    newExit->isFacingRight = true;

    newExit->entityTypeID = EXIT_ENTITY_ID;

    STATE->exit = (Entity *) LinkedList::AddNode(&STATE->listHead, newExit);

    TraceLog(LOG_TRACE, "Added exit to level (x=%.1f, y=%.1f)",
                newExit->hitbox.x, newExit->hitbox.y);

    return newExit;
}

void ExitAddFromEditor(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;
    
    
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

void EntityDestroy(Entity *entity) {

    if (entity->tags & IS_PLAYER) {
        TraceLog(LOG_DEBUG, "Tried to destroy Player entity");
        return;
    }

    if (entity->tags & IS_ANCHOR) {
        TraceLog(LOG_DEBUG, "Tried to destroy an anchor entity");
        return;
    }

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

    Entity *result = 0;

    for (Entity *e = (Entity *) STATE->listHead;
        e != 0;
        e = (Entity *) e->next) {

            if (CheckCollisionPointRec(pos, e->hitbox)) {
                result = e; break;
            }
    }

    return result;
}

Entity *EntityGetRemoveableAt(Vector2 pos) {

    Entity *result = 0;

    for (Entity *entity = (Entity *) STATE->listHead;
        entity != 0;
        entity = (Entity *) entity->next) {

            if (entity->tags & IS_PLAYER) continue;

            if (CheckCollisionPointRec(pos, entity->GetOriginHitbox())) {
                result = entity; break;
            }

            if (!entity->IsDisabled() && CheckCollisionPointRec(pos, entity->hitbox)) {
                result = entity; break;
            }
    }

    return result;
}

void PauseToggle() {

    if (STATE->isPaused) {

        if (PLAYER && PLAYER->isDead) {
            continueLevel();
        }

        STATE->isPaused = false;
        DeflatePauseMenu();

    } else {

        STATE->isPaused = true;
        InflatePauseMenu();

    }
}

void InflatePauseMenu() {

    GAME_STATE->menu = new Menu();
    GAME_STATE->menu->AddItem(new MenuItem("Continuar", &PauseToggle));
    GAME_STATE->menu->AddItem(new MenuItemToggle("Caixas de texto do desenvolvedor", &ToggleDevTextbox, &IsDevTextboxEnabled));
    GAME_STATE->menu->AddItem(new MenuItemToggle("Som", &Sounds::Toggle, &Sounds::IsEnabled));
    GAME_STATE->menu->AddItem(new MenuItemToggle("Tela cheia", &Render::FullscreenToggle, &Render::IsFullscreen));
    GAME_STATE->menu->AddItem(new MenuItemToggle("Shader CRT", &Render::CrtToggle, &Render::IsCrtEnabled));
    GAME_STATE->menu->AddItem(new MenuItem("Sair do jogo", &GameExit));
}

void DeflatePauseMenu() {

    delete GAME_STATE->menu;
    GAME_STATE->menu = 0;
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

Level::Entity *CheckCollisionWithAnyEntity(Vector2 point) {

    return CheckCollisionWithAnyEntity({ point.x, point.y, 1, 1 });
}

Level::Entity *CheckCollisionWithAnyEntity(Rectangle hitbox) {

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        if (!entity->IsDisabled() && CheckCollisionRecs(hitbox, entity->hitbox)) {
            break; // found it
        }

        entity = (Entity *) entity->next;
    }

    return entity;
}

Level::Entity *CheckCollisionWithAnything(Rectangle hitbox) {

    return CheckCollisionWithAnythingElse(hitbox, std::vector<LinkedList::Node *>());
}

Level::Entity *CheckCollisionWithAnythingElse(Rectangle hitbox, std::vector<LinkedList::Node *> entitiesToIgnore) {

    Entity *entity = (Entity *) STATE->listHead;

    while (entity != 0) {

        Rectangle entitysOrigin = {
                                        entity->origin.x,       entity->origin.y,
                                        entity->hitbox.width,   entity->hitbox.height
                                    };

        if (CheckCollisionRecs(hitbox, entitysOrigin) ||
            (!entity->IsDisabled() && CheckCollisionRecs(hitbox, entity->hitbox))) {

            for (auto e = entitiesToIgnore.begin(); e < entitiesToIgnore.end(); e++) {
                if (*e == entity) goto next_entity;
            }

            break; // found it
        }

next_entity:
        entity = (Entity *) entity->next;
    }

    return entity;
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

void Entity::AddFromPersistence(const std::string &entityTypeID, const std::string &data) {

    Level::Entity *entity;

    if (entityTypeID == PLAYER_ENTITY_ID)
        entity = Player::AddFromPersistence();
    else if (entityTypeID == ENEMY_ENTITY_ID)
        entity = Enemy::AddFromPersistence();
    else if (entityTypeID == BLOCK_ENTITY_ID)
        entity = Block::AddFromPersistence();
    else if (entityTypeID == ACID_BLOCK_ENTITY_ID)
        entity = AcidBlock::AddFromPersistence();
    else if (entityTypeID == EXIT_ENTITY_ID)
        entity = Level::ExitAddFromPersistence();
    else if (entityTypeID == GLIDE_PICKUP_ENTITY_ID)
        entity = GlideAddFromPersistence();
    else if (entityTypeID == CHECKPOINT_PICKUP_ENTITY_ID)
        entity = CheckpointPickup::AddFromPersistence();
    else if (entityTypeID == TEXTBOX_BUTTON_ENTITY_ID)
        entity = Textbox::AddFromPersistence();
    else if (entityTypeID == MOVING_PLATFORM_ENTITY_ID)
        entity = MovingPlatform::AddFromPersistence();
    else if (entityTypeID == ENEMY_DUMMY_ENTITY_ID)
        entity = EnemyDummySpike::AddFromPersistence();
    else if (entityTypeID == PRINCESS_ENTITY_ID)
        entity = Princess::AddFromPersistence();
    else if (entityTypeID == COIN_ENTITY_ID)
        entity = Coin::AddFromPersistence();
    else {
        TraceLog(LOG_ERROR, "Unknow entity type found when adding level entity for persistence, entityTypeID=%s.", entityTypeID.c_str());
        return; 
    }

    entity->PersistenceParse(data);
}

void Entity::Reset()
{

    isDead = false;
    hitbox.x = origin.x;
    hitbox.y = origin.y;
}

void Entity::Tick() {            

    // Default entity has no tick routine
}

std::string Entity::GetEntityDebugString() {
    return std::string("x=" + std::to_string((int) hitbox.x) +
                        "\ny=" + std::to_string((int) hitbox.y));
}

void Entity::Draw() {            

    Render::DrawLevelEntity(this);

    if (EDITOR_STATE->isEnabled)
        Render::DrawLevelEntityOriginGhost(this);
}

void Entity::DrawMoveGhost() {

    Render::DrawLevelEntityMoveGhost(this);
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

#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#include "level.h"
#include "../core.h"
#include "../camera.h"
#include "../render.h"


#define PLAYERS_UPPERBODY_PROPORTION    0.90f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT            5.5f
#define PLAYER_SPEED_FAST               9.0f

#define JUMP_START_VELOCITY_DEFAULT     10.0f
#define JUMP_START_VELOCITY_RUNNING     12.0f

#define DOWNWARDS_VELOCITY_TARGET       -10.0f
#define Y_VELOCITY_TARGET_TOLERANCE     1

#define Y_ACCELERATION_RATE             0.4f

// How much of the Y velocity is preserved when the ceiling is hit
// and the trajectory vector is inverted (from upwards to downwards)
#define CEILING_VELOCITY_FACTOR         0.4f

// How many seconds before landing on the ground the jump command
// still works 
#define JUMP_BUFFER_BACKWARDS_SIZE      0.12f         

// How many seconds after having left the ground the jump command
// still works
#define JUMP_BUFFER_FORWARDS_SIZE       0.15f

#define Y_VELOCITY_GLIDING              -1.5f


LevelEntity *LEVEL_PLAYER = 0;

PlayerState *LEVEL_PLAYER_STATE = 0;


static void initializePlayerState() {

    MemFree(LEVEL_PLAYER_STATE);
    LEVEL_PLAYER_STATE = MemAlloc(sizeof(PlayerState));

    LEVEL_PLAYER_STATE->isAscending = false;
    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;
    LEVEL_PLAYER_STATE->mode = PLAYER_MODE_DEFAULT;
    LEVEL_PLAYER_STATE->respawnFlagSet = false;
    LEVEL_PLAYER_STATE->lastPressedJump = -1;
    LEVEL_PLAYER_STATE->lastGroundBeneath = -1;

    TraceLog(LOG_DEBUG, "Player state initialized.");
}

// The vertical velocity that works as the initial
// propulsion of a jump
inline static float jumpStartVelocity() {

    if (LEVEL_PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING)
        return JUMP_START_VELOCITY_RUNNING;
    else
        return JUMP_START_VELOCITY_DEFAULT;
}

// Syncs the player state's hitbox with the player entity's data 
static void syncPlayersHitboxes() {

    LEVEL_PLAYER_STATE->upperbody = (Rectangle){
        LEVEL_PLAYER->hitbox.x + 1,
        LEVEL_PLAYER->hitbox.y - 1,
        LEVEL_PLAYER->hitbox.width + 2,
        LEVEL_PLAYER->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    LEVEL_PLAYER_STATE->lowerbody = (Rectangle){
        LEVEL_PLAYER->hitbox.x - 1,
        LEVEL_PLAYER->hitbox.y + LEVEL_PLAYER_STATE->upperbody.height,
        LEVEL_PLAYER->hitbox.width + 2,
        LEVEL_PLAYER->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

static void die() {

    LEVEL_PLAYER->isDead = true;
    STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isAscending=%d",
                LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y, LEVEL_PLAYER_STATE->isAscending);
}


void LevelPlayerInitialize(Vector2 origin) {

    LevelEntity *newPlayer = MemAlloc(sizeof(LevelEntity));
    LEVEL_PLAYER = newPlayer;
    LinkedListAdd(&LEVEL_LIST_HEAD, newPlayer);
 
    newPlayer->components = LEVEL_IS_PLAYER;
    newPlayer->origin = origin;
    newPlayer->sprite = PlayerDefaultSprite;
    newPlayer->hitbox = SpriteHitboxFromEdge(newPlayer->sprite, newPlayer->origin);
    newPlayer->isFacingRight = true;

    initializePlayerState();
    
    syncPlayersHitboxes();

    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
}

void LevelPlayerCheckAndSetOrigin(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(PlayerDefaultSprite, pos);
    
    if (LevelCheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player's origin couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        RenderPrintSysMessage("Origem iria colidir.");
        return;
    }

    LEVEL_PLAYER->origin = (Vector2){ hitbox.x, hitbox.y };

    TraceLog(LOG_DEBUG, "Player's origin set to x=%.1f, y=%.1f.", LEVEL_PLAYER->origin.x, LEVEL_PLAYER->origin.y);
}

void LevelPlayerCheckAndSetPos(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(LEVEL_PLAYER->sprite, pos);
    
    if (LevelCheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        RenderPrintSysMessage("Jogador iria colidir.");
        return;
    }
    
    LEVEL_PLAYER->hitbox = hitbox;
    syncPlayersHitboxes();
    TraceLog(LOG_DEBUG, "Player set to pos x=%.1f, y=%.1f.", LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y);
}

void LevelPlayerSetMode(PlayerMode mode) {

    LEVEL_PLAYER_STATE->mode = mode;

    TraceLog(LOG_DEBUG, "Player set mode to %d.", mode);
}

void LevelPlayerSetRespawn() {

    LEVEL_PLAYER_STATE->respawnFlag = (Vector2){ LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y };
    LEVEL_PLAYER_STATE->respawnFlagSet = true;
    TraceLog(LOG_DEBUG, "Respawn set to %.1f, %.1f.", LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y);
    RenderPrintSysMessage("Atualizado ponto de renascimento.");
}

void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction) {

    if (levelConcludedAgo >= 0) return;
    

    float amount = PLAYER_SPEED_DEFAULT;
    if (LEVEL_PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING)
        amount = PLAYER_SPEED_FAST;

    if (direction == PLAYER_MOVEMENT_LEFT) {
        LEVEL_PLAYER->isFacingRight = false;
        LEVEL_PLAYER_STATE->xVelocity = -amount;
    }
    else if (direction == PLAYER_MOVEMENT_RIGHT) {
        LEVEL_PLAYER->isFacingRight = true;
        LEVEL_PLAYER_STATE->xVelocity = amount;
    }
    else {
        LEVEL_PLAYER_STATE->xVelocity = 0;
    }

    syncPlayersHitboxes();
}

void LevelPlayStartRunning() {

    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_RUNNING;
}

void LevelPlayerStopRunning() {

    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;
}

void LevelPlayerJump() {

    LEVEL_PLAYER_STATE->lastPressedJump = GetTime();
}

void LevelPlayerTick() {

    PlayerState *pState = LEVEL_PLAYER_STATE;


    if (levelConcludedAgo >= 0) return;


    pState->groundBeneath = LevelGetGroundBeneath(LEVEL_PLAYER);


    if (pState->groundBeneath) {

        LEVEL_PLAYER_STATE->lastGroundBeneath = GetTime();

        if (!pState->isAscending) {
            // Is on the ground
            LEVEL_PLAYER->hitbox.y = pState->groundBeneath->hitbox.y - LEVEL_PLAYER->hitbox.height;
            pState->yVelocity = 0;
            pState->yVelocityTarget = 0;
        }
    }


    bool yVelocityWithinTarget =
        abs((int) (pState->yVelocity - pState->yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
    
    if (yVelocityWithinTarget) {
        pState->isAscending = false;

        if (!pState->groundBeneath) {
            // Starts falling down
            pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }

    const double now = GetTime();
    if (!pState->isAscending &&
        (now - LEVEL_PLAYER_STATE->lastPressedJump < JUMP_BUFFER_BACKWARDS_SIZE) &&
        (now - LEVEL_PLAYER_STATE->lastGroundBeneath < JUMP_BUFFER_FORWARDS_SIZE)) {

        // Starts jump
        pState->isAscending = true;
        pState->yVelocity = jumpStartVelocity();
        pState->yVelocityTarget = 0.0f;

        // Player hit enemy
        // -- this check is for the case the player jumps off the enemy,
        // and only in the case the enemy wasn't already destroyed in the previous frame.
        if (pState->groundBeneath &&
            pState->groundBeneath->components & LEVEL_IS_ENEMY) {

            LEVEL_PLAYER_STATE->lastGroundBeneath = GetTime();
            LevelEnemyKill(pState->groundBeneath);
            pState->groundBeneath = 0;
        }
    }

    float oldX = LEVEL_PLAYER->hitbox.x;
    float oldY = LEVEL_PLAYER->hitbox.y;
    LEVEL_PLAYER->hitbox.x += pState->xVelocity;
    LEVEL_PLAYER->hitbox.y -= pState->yVelocity;
    syncPlayersHitboxes();

    // Collision checking
    {
        if (LEVEL_PLAYER->hitbox.y + LEVEL_PLAYER->hitbox.height > FLOOR_DEATH_HEIGHT) {
            die();
            return;
        }

        ListNode *node = LEVEL_LIST_HEAD;

        while (node != 0) {

            LevelEntity *entity = (LevelEntity *) node->item;

            if ((entity->components & LEVEL_IS_ENEMY) && !entity->isDead) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, pState->upperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, pState->lowerbody)) {
                    LEVEL_PLAYER_STATE->lastGroundBeneath = GetTime();
                    LevelEnemyKill(entity);
                    goto next_entity;
                }
            }

            else if (entity->components & LEVEL_IS_SCENARIO) {

                // Check for collision with level geometry

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, LEVEL_PLAYER->hitbox);

                if (entity->components & LEVEL_IS_DANGER &&
                    (collisionRec.width > 0 || collisionRec.height > 0 || pState->groundBeneath == entity)) {
                    
                    // Player hit dangerous level element
                    die();
                    break;
                }

                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < LEVEL_PLAYER->hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < LEVEL_PLAYER->hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    const bool isPlayerMovingToTheLeft = LEVEL_PLAYER->hitbox.x < oldX;
                    const bool isPlayerMovingToTheRight = LEVEL_PLAYER->hitbox.x > oldX;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft && isPlayerMovingToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight && isPlayerMovingToTheRight)) {


                        // if (STATE->showDebugHUD) RenderPrintSysMessage("Hit wall");

                        LEVEL_PLAYER->hitbox.x = oldX;

                    }
                }


                if (isACeiling && pState->isAscending) {

                    // if (STATE->showDebugHUD) RenderPrintSysMessage("Hit ceiling");

                    pState->isAscending = false;
                    LEVEL_PLAYER->hitbox.y = oldY;
                    pState->yVelocity = (pState->yVelocity * -1) * CEILING_VELOCITY_FACTOR;
                    pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
                }

                // TODO maybe ground check should be here as well
            }

            else if (entity->components & LEVEL_IS_EXIT &&
                        CheckCollisionRecs(entity->hitbox, LEVEL_PLAYER->hitbox)) {

                // Player exit level

                LevelGoToOverworld();
            }

next_entity:
            node = node->next;
        }
    }


    bool isGliding = false;

    if (pState->yVelocity > pState->yVelocityTarget) {

        if (LEVEL_PLAYER_STATE->mode == PLAYER_MODE_GLIDE &&
                !LEVEL_PLAYER_STATE->isAscending &&
                LEVEL_PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING) {

            // Is gliding
            pState->yVelocity = Y_VELOCITY_GLIDING;
            isGliding = true;

        } else {

            // Accelerates jump's vertical movement
            pState->yVelocity -= Y_ACCELERATION_RATE;
        }
    }


    // "Animation"
    Sprite currentSprite;

    if (LEVEL_PLAYER_STATE->mode == PLAYER_MODE_GLIDE) {
        if (isGliding) currentSprite =      PlayerGlideFallingSprite;
        else currentSprite =                PlayerGlideOnSprite;
    }
    else currentSprite =                    PlayerDefaultSprite;

    LEVEL_PLAYER->sprite.sprite = currentSprite.sprite;
}

void LevelPlayerContinue() {

    STATE->isPaused = false;

    // Reset all the entities to their origins
    ListNode *node = LEVEL_LIST_HEAD;
    while (node) {

        LevelEntity *entity = (LevelEntity *) node->item;
        entity->isDead = false;
        entity->hitbox.x = entity->origin.x;
        entity->hitbox.y = entity->origin.y;

        node = node->next;
    }

    if (LEVEL_PLAYER_STATE->respawnFlagSet) {
        LEVEL_PLAYER->hitbox.x = LEVEL_PLAYER_STATE->respawnFlag.x;
        LEVEL_PLAYER->hitbox.y = LEVEL_PLAYER_STATE->respawnFlag.y;
    } else {
        LEVEL_PLAYER->hitbox.x = LEVEL_PLAYER->origin.x;
        LEVEL_PLAYER->hitbox.y = LEVEL_PLAYER->origin.y;
    }
    LEVEL_PLAYER_STATE->isAscending = false;

    syncPlayersHitboxes();
    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

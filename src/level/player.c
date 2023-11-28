#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"
#include "level.h"
#include "enemy.h"
#include "../core.h"
#include "../camera.h"
#include "../render.h"


#define PLAYERS_UPPERBODY_PROPORTION        0.90f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT                5.5f
#define PLAYER_SPEED_FAST                   9.0f

#define JUMP_START_VELOCITY_DEFAULT         10.0f
#define JUMP_START_VELOCITY_RUNNING         12.0f

#define DOWNWARDS_VELOCITY_TARGET           -10.0f
#define Y_VELOCITY_TARGET_TOLERANCE         1

#define Y_ACCELERATION_RATE                 0.4f

// How much of the Y velocity is preserved when the ceiling is hit
// and the trajectory vector is inverted (from upwards to downwards)
#define CEILING_VELOCITY_FACTOR             0.4f

// How many seconds before landing on the ground the jump command
// still works 
#define JUMP_BUFFER_BACKWARDS_SIZE          0.08f  

// JUMP_BUFFER_BACKWARDS_SIZE for when the player is gliding.
// The player is falling slower, and it feels jarring to have  
// a smaller Y window where the backwards jump buffer works.
#define JUMP_BUFFER_BACKWARDS_SIZE_GLIDING  0.20f

// How many seconds after having left the ground the jump command
// still works
#define JUMP_BUFFER_FORWARDS_SIZE           0.15f

#define Y_VELOCITY_GLIDING                  -1.5f

#define INITIAL_CHECKPOINTS_NUMBER          1;


LevelEntity *PLAYER_ENTITY = 0;

PlayerState *PLAYER_STATE = 0;


static void initializePlayerState() {

    MemFree(PLAYER_STATE);
    PLAYER_STATE = MemAlloc(sizeof(PlayerState));

    PLAYER_STATE->isAscending = false;
    PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;
    PLAYER_STATE->mode = PLAYER_MODE_DEFAULT;
    PLAYER_STATE->lastPressedJump = -1;
    PLAYER_STATE->lastGroundBeneath = -1;
    PLAYER_STATE->checkpointsLeft = INITIAL_CHECKPOINTS_NUMBER;

    TraceLog(LOG_DEBUG, "Player state initialized.");
}

// The vertical velocity that works as the initial
// propulsion of a jump
static float jumpStartVelocity() {

    if (PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING)
        return JUMP_START_VELOCITY_RUNNING;
    else
        return JUMP_START_VELOCITY_DEFAULT;
}

// The size of the backwards jump buffer, that varies in function
// of its vertical velocity
static float jumpBufferBackwardsSize() {

    if (PLAYER_STATE->isGliding)
        return JUMP_BUFFER_BACKWARDS_SIZE_GLIDING;
    else
        return JUMP_BUFFER_BACKWARDS_SIZE;
}

// Syncs the player state's hitbox with the player entity's data 
static void syncPlayersHitboxes() {

    PLAYER_STATE->upperbody = (Rectangle){
        PLAYER_ENTITY->hitbox.x + 1,
        PLAYER_ENTITY->hitbox.y - 1,
        PLAYER_ENTITY->hitbox.width + 2,
        PLAYER_ENTITY->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    PLAYER_STATE->lowerbody = (Rectangle){
        PLAYER_ENTITY->hitbox.x - 1,
        PLAYER_ENTITY->hitbox.y + PLAYER_STATE->upperbody.height,
        PLAYER_ENTITY->hitbox.width + 2,
        PLAYER_ENTITY->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

static void die() {

    PLAYER_ENTITY->isDead = true;
    GAME_STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isAscending=%d",
                PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y, PLAYER_STATE->isAscending);
}


void PlayerInitialize(Vector2 origin) {

    LevelEntity *newPlayer = MemAlloc(sizeof(LevelEntity));
    PLAYER_ENTITY = newPlayer;
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

void PlayerCheckAndSetOrigin(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(PlayerDefaultSprite, pos);
    
    if (LevelCheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player's origin couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        RenderPrintSysMessage("Origem iria colidir.");
        return;
    }

    PLAYER_ENTITY->origin = (Vector2){ hitbox.x, hitbox.y };

    TraceLog(LOG_DEBUG, "Player's origin set to x=%.1f, y=%.1f.", PLAYER_ENTITY->origin.x, PLAYER_ENTITY->origin.y);
}

void PlayerCheckAndSetPos(Vector2 pos) {

    Rectangle hitbox = SpriteHitboxFromMiddle(PLAYER_ENTITY->sprite, pos);
    
    if (LevelCheckCollisionWithAnyEntity(hitbox)) {
        TraceLog(LOG_DEBUG,
            "Player couldn't be set at pos x=%.1f, y=%.1f; would collide with a different entity.", pos.x, pos.y);
        RenderPrintSysMessage("Jogador iria colidir.");
        return;
    }
    
    PLAYER_ENTITY->hitbox = hitbox;
    syncPlayersHitboxes();
    TraceLog(LOG_DEBUG, "Player set to pos x=%.1f, y=%.1f.", PLAYER_ENTITY->hitbox.x, PLAYER_ENTITY->hitbox.y);
}

void PlayerSetMode(PlayerMode mode) {

    PLAYER_STATE->mode = mode;

    TraceLog(LOG_DEBUG, "Player set mode to %d.", mode);
}

void PlayerMoveHorizontal(PlayerHorizontalMovementType direction) {

    if (levelConcludedAgo >= 0) return;
    

    float amount = PLAYER_SPEED_DEFAULT;
    if (PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING)
        amount = PLAYER_SPEED_FAST;

    if (direction == PLAYER_MOVEMENT_LEFT) {
        PLAYER_ENTITY->isFacingRight = false;
        PLAYER_STATE->xVelocity = -amount;
    }
    else if (direction == PLAYER_MOVEMENT_RIGHT) {
        PLAYER_ENTITY->isFacingRight = true;
        PLAYER_STATE->xVelocity = amount;
    }
    else {
        PLAYER_STATE->xVelocity = 0;
    }

    syncPlayersHitboxes();
}

void PlayerStartRunning() {

    PLAYER_STATE->speed = PLAYER_MOVEMENT_RUNNING;
}

void PlayerStopRunning() {

    PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;
}

void PlayerJump() {

    PLAYER_STATE->lastPressedJump = GetTime();
}

void PlayerTick() {

    PlayerState *pState = PLAYER_STATE;


    if (levelConcludedAgo >= 0) return;


    pState->groundBeneath = LevelGetGroundBeneath(PLAYER_ENTITY);


    if (pState->groundBeneath) {

        PLAYER_STATE->lastGroundBeneath = GetTime();

        if (!pState->isAscending) {
            // Is on the ground
            PLAYER_ENTITY->hitbox.y = pState->groundBeneath->hitbox.y - PLAYER_ENTITY->hitbox.height;
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
        (now - PLAYER_STATE->lastPressedJump < jumpBufferBackwardsSize()) &&
        (now - PLAYER_STATE->lastGroundBeneath < JUMP_BUFFER_FORWARDS_SIZE)) {

        // Starts jump
        pState->isAscending = true;
        pState->yVelocity = jumpStartVelocity();
        pState->yVelocityTarget = 0.0f;

        // Player hit enemy
        // -- this check is for the case the player jumps off the enemy,
        // and only in the case the enemy wasn't already destroyed in the previous frame.
        if (pState->groundBeneath &&
            pState->groundBeneath->components & LEVEL_IS_ENEMY) {

            PLAYER_STATE->lastGroundBeneath = GetTime();
            EnemyKill(pState->groundBeneath);
            pState->groundBeneath = 0;
        }
    }

    float oldX = PLAYER_ENTITY->hitbox.x;
    float oldY = PLAYER_ENTITY->hitbox.y;
    PLAYER_ENTITY->hitbox.x += pState->xVelocity;
    PLAYER_ENTITY->hitbox.y -= pState->yVelocity;
    syncPlayersHitboxes();

    // Collision checking
    {
        if (PLAYER_ENTITY->hitbox.y + PLAYER_ENTITY->hitbox.height > FLOOR_DEATH_HEIGHT) {
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
                    PLAYER_STATE->lastGroundBeneath = GetTime();
                    EnemyKill(entity);
                    goto next_entity;
                }
            }

            else if (entity->components & LEVEL_IS_SCENARIO) {

                // Check for collision with level geometry

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, PLAYER_ENTITY->hitbox);

                if (entity->components & LEVEL_IS_DANGER &&
                    (collisionRec.width > 0 || collisionRec.height > 0 || pState->groundBeneath == entity)) {
                    
                    // Player hit dangerous level element
                    die();
                    break;
                }

                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < PLAYER_ENTITY->hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < PLAYER_ENTITY->hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    const bool isPlayerMovingToTheLeft = PLAYER_ENTITY->hitbox.x < oldX;
                    const bool isPlayerMovingToTheRight = PLAYER_ENTITY->hitbox.x > oldX;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft && isPlayerMovingToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight && isPlayerMovingToTheRight)) {


                        // if (GAME_STATE->showDebugHUD) RenderPrintSysMessage("Hit wall");

                        PLAYER_ENTITY->hitbox.x = oldX;

                    }
                }


                if (isACeiling && pState->isAscending) {

                    // if (GAME_STATE->showDebugHUD) RenderPrintSysMessage("Hit ceiling");

                    pState->isAscending = false;
                    PLAYER_ENTITY->hitbox.y = oldY;
                    pState->yVelocity = (pState->yVelocity * -1) * CEILING_VELOCITY_FACTOR;
                    pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
                }

                // TODO maybe ground check should be here as well
            }

            else if (entity->components & LEVEL_IS_EXIT &&
                        CheckCollisionRecs(entity->hitbox, PLAYER_ENTITY->hitbox)) {

                // Player exit level

                LevelGoToOverworld();
            }

            else if (entity->components & LEVEL_IS_GLIDE &&
                        CheckCollisionRecs(entity->hitbox, PLAYER_ENTITY->hitbox) &&
                        PLAYER_STATE->mode != PLAYER_MODE_GLIDE) {
                PlayerSetMode(PLAYER_MODE_GLIDE);
                return;
            }

next_entity:
            node = node->next;
        }
    }


    PLAYER_STATE->isGliding = false;
    if (pState->yVelocity > pState->yVelocityTarget) {

        if (PLAYER_STATE->mode == PLAYER_MODE_GLIDE &&
                !PLAYER_STATE->isAscending &&
                PLAYER_STATE->speed == PLAYER_MOVEMENT_RUNNING) {

            // Is gliding
            pState->yVelocity = Y_VELOCITY_GLIDING;
            PLAYER_STATE->isGliding = true;

        } else {

            // Accelerates jump's vertical movement
            pState->yVelocity -= Y_ACCELERATION_RATE;
        }
    }


    // "Animation"
    Sprite currentSprite;

    if (PLAYER_STATE->mode == PLAYER_MODE_GLIDE) {
        if (PLAYER_STATE->isGliding)
            currentSprite =     PlayerGlideFallingSprite;
        else
            currentSprite =     PlayerGlideOnSprite;
    }
    else currentSprite =        PlayerDefaultSprite;

    PLAYER_ENTITY->sprite.sprite = currentSprite.sprite;
}

void PlayerContinue() {

    GAME_STATE->isPaused = false;

    // Reset all the entities to their origins
    ListNode *node = LEVEL_LIST_HEAD;
    while (node) {

        LevelEntity *entity = (LevelEntity *) node->item;
        entity->isDead = false;
        entity->hitbox.x = entity->origin.x;
        entity->hitbox.y = entity->origin.y;

        node = node->next;
    }

    if (PLAYER_STATE->checkpoint) {
        Vector2 pos = RectangleGetPos(PLAYER_STATE->checkpoint->hitbox);
        pos.y -= PLAYER_STATE->checkpoint->hitbox.height;
        RectangleSetPos(&PLAYER_ENTITY->hitbox, pos);
    } else {
        RectangleSetPos(&PLAYER_ENTITY->hitbox, PLAYER_ENTITY->origin);
    }
    PLAYER_STATE->isAscending = false;

    syncPlayersHitboxes();
    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

void PlayerSetCheckpoint() {

    if (!PLAYER_STATE->groundBeneath) {
        TraceLog(LOG_DEBUG, "Player didn't set checkpoint, not on the ground.");
        return;
    }

    if (PLAYER_STATE->checkpointsLeft < 1) {
        RenderPrintSysMessage("Sem checkpoints disponíveis.");
        return;
    }

    if (PLAYER_STATE->checkpoint) {
        LevelEntityDestroy(
            LinkedListGetNode(LEVEL_LIST_HEAD, PLAYER_STATE->checkpoint));
    }

    Vector2 pos = RectangleGetPos(PLAYER_ENTITY->hitbox);
    pos.y += PLAYER_ENTITY->hitbox.height / 2;
    PLAYER_STATE->checkpoint = LevelCheckpointAdd(pos);

    PLAYER_STATE->checkpointsLeft--;
    char checkpointMsg[50];
    sprintf(checkpointMsg, "Checkpoints disponívels: %d", PLAYER_STATE->checkpointsLeft);
    RenderPrintSysMessage(checkpointMsg);

    TraceLog(LOG_DEBUG, "Player set checkpoint at x=%.1f, y=%.1f.", pos.x, pos.y);
}

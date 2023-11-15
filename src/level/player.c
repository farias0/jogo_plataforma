#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "level.h"
#include "../core.h"
#include "../assets.h"
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
#define JUMP_BUFFER_BACKWARDS_SIZE      0.10f         

// How many seconds after having left the ground the jump command
// still works
#define JUMP_BUFFER_FORWARDS_SIZE       0.10f


LevelEntity *LEVEL_PLAYER = 0;

PlayerState *LEVEL_PLAYER_STATE = 0;


// for jump buffers
static double lastPressedJumpTimestamp = -1;
static double lastGroundBeneathTimestamp = -1;


static void resetPlayerState() {

    if (LEVEL_PLAYER_STATE) {
        MemFree(LEVEL_PLAYER_STATE);
        TraceLog(LOG_TRACE, "Player state destroyed.");
    }

    LEVEL_PLAYER_STATE = MemAlloc(sizeof(PlayerState));

    LEVEL_PLAYER_STATE->isJumping = false;
    LEVEL_PLAYER_STATE->isDead = false;
    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;

    TraceLog(LOG_DEBUG, "Player state reset.");
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

    LEVEL_PLAYER_STATE->isDead = true;
    STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isJumping=%d",
                LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y, LEVEL_PLAYER_STATE->isJumping);
}

void LevelPlayerInitialize(Vector2 pos) {

    LevelEntity *newPlayer = MemAlloc(sizeof(LevelEntity));
    LEVEL_PLAYER = newPlayer;
    LinkedListAdd(&LEVEL_LIST_HEAD, newPlayer);
    
    newPlayer->components = LEVEL_IS_PLAYER;
    newPlayer->hitbox = SpriteHitboxFromEdge(PlayerSprite, pos);
    newPlayer->sprite = PlayerSprite;
    newPlayer->isFacingRight = true;

    resetPlayerState();
    
    syncPlayersHitboxes();

    LevelPlayerSetStartingPos(pos); // TODO replace it with entity origin

    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
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

    if (!LEVEL_PLAYER_STATE->groundBeneath) return;

    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_RUNNING;
}

void LevelPlayerStopRunning() {

    LEVEL_PLAYER_STATE->speed = PLAYER_MOVEMENT_DEFAULT;
}

void LevelPlayerJump() {

    lastPressedJumpTimestamp = GetTime();
}

void LevelPlayerTick() {

    PlayerState *pState = LEVEL_PLAYER_STATE;


    if (levelConcludedAgo >= 0) return;


    pState->groundBeneath = LevelGetGroundBeneath(LEVEL_PLAYER);


    if (pState->groundBeneath) {

        lastGroundBeneathTimestamp = GetTime();

        if (!pState->isJumping) {
            // Is on the ground
            LEVEL_PLAYER->hitbox.y = pState->groundBeneath->hitbox.y - LEVEL_PLAYER->hitbox.height;
            pState->yVelocity = 0;
            pState->yVelocityTarget = 0;
        }
    }


    bool yVelocityWithinTarget =
        abs((int) (pState->yVelocity - pState->yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
    
    if (yVelocityWithinTarget) {
        pState->isJumping = false;

        if (!pState->groundBeneath) {
            // Starts falling down
            pState->yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }


    LEVEL_PLAYER->hitbox.y -= pState->yVelocity;
    LEVEL_PLAYER->hitbox.x += pState->xVelocity;
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

            if (entity->components & LEVEL_IS_ENEMY) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, pState->upperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, pState->lowerbody)) {
                    lastGroundBeneathTimestamp = GetTime();
                    ListNode *enemyNode = node;
                    node = node->next;
                    LevelEntityDestroy(enemyNode);
                    TraceLog(LOG_TRACE, "Player murdered an enemy in cold blood.");
                    continue;
                }
            }

            else if (entity->components & LEVEL_IS_SCENARIO) {
                

                Rectangle collisionRec = GetCollisionRec(entity->hitbox, LEVEL_PLAYER->hitbox);
                if (collisionRec.width == 0 || collisionRec.height == 0) goto next_entity;
            
                const bool isAWall = collisionRec.width <= collisionRec.height;
                const bool isACeiling = (collisionRec.width >= collisionRec.height) &&
                                    (entity->hitbox.y < LEVEL_PLAYER->hitbox.y);


                if (isAWall && collisionRec.width > 0) {

                    const bool isEntitysRightWall = collisionRec.x > entity->hitbox.x; 
                    const bool isEntitysLeftWall = !isEntitysRightWall;

                    const bool isEntityToTheLeft = entity->hitbox.x < LEVEL_PLAYER->hitbox.x;
                    const bool isEntityToTheRight = !isEntityToTheLeft;

                    // So when the player hits the entity to its left,
                    // he can only collide with its left wall.
                    if ((isEntitysRightWall && isEntityToTheLeft) ||
                        (isEntitysLeftWall && isEntityToTheRight)) {


                        if (STATE->showDebugHUD) RenderPrintSysMessage("Hit wall");

                        LEVEL_PLAYER->hitbox.x -= pState->xVelocity;

                    }
                }


                if (isACeiling && pState->isJumping) {

                    if (STATE->showDebugHUD) RenderPrintSysMessage("Hit ceiling");

                    pState->isJumping = false;
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

    const double now = GetTime();
    if (!pState->isJumping &&
        (now - lastPressedJumpTimestamp < JUMP_BUFFER_BACKWARDS_SIZE) &&
        (now - lastGroundBeneathTimestamp < JUMP_BUFFER_FORWARDS_SIZE)) {

        // Starts jump
        pState->isJumping = true;
        pState->yVelocity = jumpStartVelocity();
        pState->yVelocityTarget = 0.0f;
    }

    // Accelerates jump's vertical movement

    if (pState->yVelocity > pState->yVelocityTarget)
            pState->yVelocity -= Y_ACCELERATION_RATE; // Upwards

    else if (pState->yVelocity < pState->yVelocityTarget)
            pState->yVelocity += Y_ACCELERATION_RATE; // Downwards
}

void LevelPlayerContinue() {

    STATE->isPaused = false;
    resetPlayerState();

    Vector2 pos = LevelGetPlayerStartingPosition();
    LEVEL_PLAYER->hitbox.x = pos.x;
    LEVEL_PLAYER->hitbox.y = pos.y;
    syncPlayersHitboxes();
    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

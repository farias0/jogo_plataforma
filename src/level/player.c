#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "level.h"
#include "../core.h"
#include "../assets.h"
#include "../camera.h"


#define PLAYERS_UPPERBODY_PROPORTION    0.90f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT            5.5f
#define PLAYER_SPEED_FAST               9.0f

#define JUMP_START_VELOCITY_DEFAULT     10.0f
#define JUMP_START_VELOCITY_RUNNING     12.0f

#define DOWNWARDS_VELOCITY_TARGET       -10.0f
#define Y_VELOCITY_TARGET_TOLERANCE     1

#define Y_ACCELERATION_RATE             0.4f


LevelEntity *LEVEL_PLAYER = 0;

// The ground beneath the player, updated every frame
static LevelEntity* groundBeneath;


// TODO PlayerState
/*
    TODO: Currently only the player has more than one hitbox,
        but eventually every entity should support multiple hitboxes.
*/

static Rectangle playersUpperbody, playersLowerbody;
// If the player is on the ascension phase of the jump
static bool isJumping = false;
static float yVelocity = 0;
static float yVelocityTarget = 0;
static float xVelocity = 0;


// The vertical velocity that works as the initial
// propulsion of a jump
inline static float jumpStartVelocity() {

    if (STATE->playerMovementSpeed == PLAYER_MOVEMENT_RUNNING)
        return JUMP_START_VELOCITY_RUNNING;
    else
        return JUMP_START_VELOCITY_DEFAULT;
}

static void calculatePlayersHitboxes() {
    playersUpperbody = (Rectangle){
        LEVEL_PLAYER->hitbox.x + 1,       LEVEL_PLAYER->hitbox.y - 1,
        LEVEL_PLAYER->hitbox.width + 2,   LEVEL_PLAYER->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowerbody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    playersLowerbody = (Rectangle){
        LEVEL_PLAYER->hitbox.x - 1,       LEVEL_PLAYER->hitbox.y + playersUpperbody.height,
        LEVEL_PLAYER->hitbox.width + 2,   LEVEL_PLAYER->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

static void die() {
    STATE->isPlayerDead = true;
    STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isJumping=%d",
                LEVEL_PLAYER->hitbox.x, LEVEL_PLAYER->hitbox.y, isJumping);
}

void LevelPlayerInitialize(Vector2 pos) {

    LevelEntity *newPlayer = MemAlloc(sizeof(LevelEntity));
    LEVEL_PLAYER = newPlayer;

    newPlayer->components = LEVEL_IS_PLAYER;
    newPlayer->hitbox = SpriteHitboxFromEdge(PlayerSprite, pos);
    newPlayer->sprite = PlayerSprite;
    newPlayer->isFacingRight = true;

    LevelPlayerSetStartingPos(pos); // TODO replace it with entity origin

    calculatePlayersHitboxes();

    LinkedListAdd(&LEVEL_LIST_HEAD, newPlayer);

    TraceLog(LOG_TRACE, "Added player to level (x=%.1f, y=%.1f)",
                newPlayer->hitbox.x, newPlayer->hitbox.y);
}

void LevelPlayerMoveHorizontal(PlayerHorizontalMovementType direction) {

    float amount = PLAYER_SPEED_DEFAULT;
    if (STATE->playerMovementSpeed == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    if (direction == PLAYER_MOVEMENT_LEFT) {
        LEVEL_PLAYER->isFacingRight = false;

        xVelocity = -amount;
    }
    else if (direction == PLAYER_MOVEMENT_RIGHT) {
        LEVEL_PLAYER->isFacingRight = true;

        xVelocity = amount;
    }
    else {
        xVelocity = 0;
    }

    calculatePlayersHitboxes();
}

void LevelPlayStartRunning() {

    if (!groundBeneath) return;

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_RUNNING;
}

void LevelPlayerStopRunning() {

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;
}

void LevelPlayerJump() {

    /*
        TODO: Instead of checking if there is ground beneath, check if the last time
        there was is within a LAST_GROUND_BENEATH_TOLERANCE.
        
        It will need to check and update a "lastGroundBeneath" timestamp every frame.
        
        This will allow the player to jump even if the jump button was
        pressed a few milliseconds before.
    */

    if (groundBeneath) {

        // Starts jumping
        isJumping = true;
        yVelocity = jumpStartVelocity();
        yVelocityTarget = 0.0f;
    }
}

void LevelPlayerTick() {

    if (STATE->showDebugHUD) {
        char ySpeedTxt[100];
        sprintf(ySpeedTxt, "yVelocity: %f   y: %f", yVelocity, LEVEL_PLAYER->hitbox.y);
        DrawText(ySpeedTxt, 10, 40, 20, WHITE);
    }

    if (levelConcludedAgo >= 0) return;

    
    groundBeneath = LevelGetGroundBeneath(LEVEL_PLAYER); 
    if (groundBeneath) {

        if (STATE->showDebugHUD) DrawText("On the ground!", 10, 60, 20, WHITE);

        if (!isJumping) {
            // Is on the ground
            LEVEL_PLAYER->hitbox.y = groundBeneath->hitbox.y - LEVEL_PLAYER->hitbox.height;
            yVelocity = 0;
            yVelocityTarget = 0;
        }
    }

    bool yVelocityWithinTarget =
        abs((int) (yVelocity - yVelocityTarget)) < Y_VELOCITY_TARGET_TOLERANCE;
        
    if (yVelocityWithinTarget) {
        isJumping = false;

        if (!groundBeneath) {
            // Is falling down
            yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;
        }
    }

    LEVEL_PLAYER->hitbox.y -= yVelocity;
    LEVEL_PLAYER->hitbox.x += xVelocity;

    calculatePlayersHitboxes();


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
                if (CheckCollisionRecs(entity->hitbox, playersUpperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, playersLowerbody)) {
                    ListNode *enemyNode = node;
                    node = node->next;
                    LinkedListRemove(&LEVEL_LIST_HEAD, enemyNode);
                    TraceLog(LOG_TRACE, "Player murdered an enemy in cold blood.");
                    continue;
                }
            }

            else if (entity->components & LEVEL_IS_SCENARIO &&
                        CheckCollisionRecs(entity->hitbox, LEVEL_PLAYER->hitbox)) {

                // Player hit wall
                if ((abs((int) (LEVEL_PLAYER->hitbox.x - entity->hitbox.x - entity->hitbox.width)) < 10.0f) ||
                        (abs((int) (LEVEL_PLAYER->hitbox.x + LEVEL_PLAYER->hitbox.width - entity->hitbox.x)) < 10.0f)) {

                    if (STATE->showDebugHUD) DrawText("Hit wall", 10, 80, 20, WHITE);

                    LEVEL_PLAYER->hitbox.x -= xVelocity;

                    goto next_entity;
                }

                // Player hit ceiling
                if ((abs((int) (LEVEL_PLAYER->hitbox.y - (entity->hitbox.y + entity->hitbox.height))) < 15.0f) && isJumping) {

                    if (STATE->showDebugHUD) DrawText("Hit ceiling", 10, 100, 20, WHITE);

                    isJumping = false;
                    yVelocity = -(1/yVelocity);
                    yVelocityTarget = DOWNWARDS_VELOCITY_TARGET;

                    goto next_entity;
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


    // Accelerates jump's vertical movement
    if (yVelocity > yVelocityTarget) {
        yVelocity -= Y_ACCELERATION_RATE; // Upwards
    } else if (yVelocity < yVelocityTarget) {
        yVelocity += Y_ACCELERATION_RATE; // Downwards
    }
}

void LevelPlayerContinue() {

    STATE->isPaused = false;
    STATE->isPlayerDead = false;

    STATE->playerMovementSpeed = PLAYER_MOVEMENT_DEFAULT;

    Vector2 pos = LevelGetPlayerStartingPosition();
    LEVEL_PLAYER->hitbox.x = pos.x;
    LEVEL_PLAYER->hitbox.y = pos.y;

    CameraLevelCentralizeOnPlayer();

    TraceLog(LOG_DEBUG, "Player continue.");
}

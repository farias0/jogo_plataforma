#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"
#include "entity.h"
#include "../global.h"
#include "../assets.h"


#define PLAYERS_UPPERBODY_PROPORTION 0.90f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT 5.5f
#define PLAYER_SPEED_FAST 9.0f

#define PLAYER_JUMP_DURATION 0.75f // In seconds
#define PLAYER_JUMP_HEIGHT 64.0f
#define PLAYER_END_JUMP_DISTANCE_GROUND 1 // Distance from the ground to end the jump when descending 

#define JUMP_START_VELOCITY 10.0f
#define JUMP_ACCELERATION 0.4f
#define Y_VELOCITY_TARGET_TOLERANCE 1


Rectangle playersUpperbody, playersLowebody;

// TODO PlayerState
// If the player is on the ascension phase of the jump
bool isJumping = false;
float yVelocity = 0;
float yVelocityTarget = 0;
float xVelocity = 0;


void calculatePlayersHitboxes(Entity *player) {
    playersUpperbody = (Rectangle){
        player->hitbox.x + 1,       player->hitbox.y - 1,
        player->hitbox.width + 2,   player->hitbox.height * PLAYERS_UPPERBODY_PROPORTION + 1
    };

    /*
        playersLowebody has one extra pixel in its sides and below it.
        This has game feel implications, but it was included so the collision check would work more consistently
        (this seems to be related with the use of raylib's CheckCollisionRecs for player-enemy collision).
        This is something that should not be needed in a more robust implementation.
    */
    playersLowebody = (Rectangle){
        player->hitbox.x - 1,       player->hitbox.y + playersUpperbody.height,
        player->hitbox.width + 2,   player->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION) + 1
    };
}

void die() {
    STATE->isPlayerDead = true;
    STATE->isPaused = true;

    TraceLog(LOG_DEBUG, "You Died.\n\tx=%f, y=%f, isJumping=%d",
                PLAYER->hitbox.x, PLAYER->hitbox.y, isJumping);
}

Entity *InitializePlayer(Entity *head, Entity **newPlayer) {
    *newPlayer = MemAlloc(sizeof(Entity));

    (*newPlayer)->components = HasPosition +
                            IsPlayer +
                            HasSprite +
                            DoesTick;
    (*newPlayer)->hitbox = (Rectangle){ 0.0f,
                                        0.0f,
                                        PlayerSprite.sprite.width * PlayerSprite.scale,
                                        PlayerSprite.sprite.height * PlayerSprite.scale};
    (*newPlayer)->sprite = PlayerSprite;
    (*newPlayer)->isFacingRight = true;

    calculatePlayersHitboxes(*newPlayer);

    return AddToEntityList(head, *newPlayer);
}

void UpdatePlayerHorizontalMovement(PlayerHorizontalMovementType direction) {
    float amount = PLAYER_SPEED_DEFAULT;
    if (STATE->playerMovementSpeed == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    if (direction == PLAYER_MOVEMENT_LEFT) {
        PLAYER->isFacingRight = false;

        xVelocity = -amount;
    }
    else if (direction == PLAYER_MOVEMENT_RIGHT) {
        PLAYER->isFacingRight = true;

        xVelocity = amount;
    }
    else {
        xVelocity = 0;
    }

    calculatePlayersHitboxes(PLAYER);
}

void PlayerStartJump(Entity *player) {

    /*
        TODO: Instead of checking if there is ground beneath, check if the last time
        there was is within a LAST_GROUND_BENEATH_TOLERANCE.
        
        It will need to check and update a "lastGroundBeneath" timestamp every frame.
        
        This will allow the player to jump even if the jump button was
        pressed a few milliseconds before.
    */

    if (GetGroundBeneath(player)) {
        isJumping = true;
        yVelocity = JUMP_START_VELOCITY;
        yVelocityTarget = 0.0f;
    }
}

void PlayerTick(Entity *player) {

    // debug
    char ySpeedTxt[100];
    sprintf(ySpeedTxt, "yVelocity: %f   y: %f", yVelocity, player->hitbox.y);
    DrawText(ySpeedTxt, 10, 40, 20, WHITE);

    Entity *groundBeneath = GetGroundBeneath(player); 
    if (groundBeneath) {

        // debug
        DrawText("On the ground!", 10, 60, 20, WHITE);

        if (!isJumping) {
            // Landing on the ground
            player->hitbox.y = groundBeneath->hitbox.y - player->hitbox.height;
            yVelocity = 0;
            yVelocityTarget = 0;
        }
    }

    bool yVelocityWithinTarget = abs(yVelocity - yVelocityTarget) < Y_VELOCITY_TARGET_TOLERANCE;
    if (yVelocityWithinTarget) {
        isJumping = false;

        if (!groundBeneath) {
            // Starts falling down
            yVelocityTarget = -JUMP_START_VELOCITY;
        }
    }

    player->hitbox.y -= yVelocity;
    player->hitbox.x += xVelocity;

    calculatePlayersHitboxes(player);


    // Collision checking
    {
        if (player->hitbox.y > FLOOR_DEATH_HEIGHT) {
            die();
            return;
        }

        Entity *entity = ENTITIES_HEAD;
        while (entity != 0) {

            if (entity->components & IsEnemy) {

                // Enemy hit player
                if (CheckCollisionRecs(entity->hitbox, playersUpperbody)) {
                    die();
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(entity->hitbox, playersLowebody)) {
                    ENTITIES_HEAD = DestroyEntity(entity); // TODO: How does this break the loop?
                    break;
                }
            }

            else if (entity->components & IsLevelElement &&
                        CheckCollisionRecs(entity->hitbox, player->hitbox)) {

                // Player hit wall
                if ((abs(player->hitbox.x - entity->hitbox.x - entity->hitbox.width) < 10.0f) ||
                        (abs(player->hitbox.x + player->hitbox.width - entity->hitbox.x) < 10.0f)) {

                    // debug
                    DrawText("Hit wall", 10, 80, 20, WHITE);

                    player->hitbox.x -= xVelocity;

                    goto next_entity;
                }

                // Player hit ceiling
                if ((abs(player->hitbox.y - (entity->hitbox.y + entity->hitbox.height)) < 15.0f) && isJumping) {

                    // debug
                    DrawText("Hit ceiling", 10, 100, 20, WHITE);

                    isJumping = false;
                    yVelocity = -(1/yVelocity);
                    yVelocityTarget = -JUMP_START_VELOCITY;

                    goto next_entity;
                }

                // TODO maybe ground check should be here as well
            }

next_entity:
            entity = entity->next;
        }
    }


    // Accelerates jump's vertical movement
    if (yVelocity > yVelocityTarget) {
        yVelocity -= JUMP_ACCELERATION; // Upwards
    } else if (yVelocity < yVelocityTarget) {
        yVelocity += JUMP_ACCELERATION; // Downwards
    }
}

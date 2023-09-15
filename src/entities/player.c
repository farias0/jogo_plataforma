#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"
#include "entity.h"
#include "../global.h"


#define PLAYER_SPRITE_SCALE 2

// sprite is 32x64 pixels
#define PLAYER_WIDTH (float)(PLAYER_SPRITE_SCALE * 32)
#define PLAYER_HEIGHT (float)(PLAYER_SPRITE_SCALE * 64)

#define PLAYERS_UPPERBODY_PROPORTION 0.90f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT 5.5f
#define PLAYER_SPEED_FAST 8.0f

#define PLAYER_JUMP_DURATION 0.75f // In seconds
#define PLAYER_JUMP_HEIGHT PLAYER_HEIGHT * 1.0
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

Entity *InitializePlayer(Entity *listItem) {
    Entity *newPlayer = MemAlloc(sizeof(Entity));

    newPlayer->components = HasPosition +
                            IsPlayer +
                            HasSprite +
                            DoesTick;
    newPlayer->hitbox = (Rectangle){ 0.0f, 0.0f, PLAYER_WIDTH, PLAYER_HEIGHT };
    newPlayer->sprite = LoadTexture("../assets/player_default_1.png");
    newPlayer->spriteScale = PLAYER_SPRITE_SCALE;
    newPlayer->isFacingRight = true;

    calculatePlayersHitboxes(newPlayer);

    AddToEntityList(listItem, newPlayer);

    return newPlayer;
}

void MovePlayer(PlayerMovementDirection direction) {
    float amount = PLAYER_SPEED_DEFAULT;
    if (STATE->playerMovementType == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    if (direction == PLAYER_MOVEMENT_LEFT) {
        PLAYER->isFacingRight = false;

        PLAYER->hitbox.x -= amount;

        // TODO move camera code to camera.c
        if (CAMERA->hitbox.x > amount && ((PLAYER->hitbox.x - CAMERA->hitbox.x) < SCREEN_WIDTH/3))
            CAMERA->hitbox.x -= amount;

    } else {
        PLAYER->isFacingRight = true;

        PLAYER->hitbox.x += amount;

        // TODO move camera code to camera.c
        if (PLAYER->hitbox.x > amount + SCREEN_WIDTH/2)
            CAMERA->hitbox.x += amount;
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

    if (GetEntitiesGroundBeneath(player) >= 0) {
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

    float yGroundBeneath = GetEntitiesGroundBeneath(player); 
    if (yGroundBeneath >= 0) {

        // debug
        DrawText("On the ground!", 10, 60, 20, WHITE);

        if (!isJumping) {
            // Landing on the ground
            player->hitbox.y = yGroundBeneath - player->hitbox.height;
            yVelocity = 0;
            yVelocityTarget = 0;
        }
    }

    bool yVelocityWithinTarget = abs(yVelocity - yVelocityTarget) < Y_VELOCITY_TARGET_TOLERANCE;
    if (yVelocityWithinTarget) {
        isJumping = false;

        if (yGroundBeneath == -1) {
            // Starts falling down
            yVelocityTarget = -JUMP_START_VELOCITY;
        }
    }

    player->hitbox.y -= yVelocity;

    // Accelerates jump's vertical movement
    if (yVelocity > yVelocityTarget) {
        yVelocity -= JUMP_ACCELERATION; // Upwards
    } else if (yVelocity < yVelocityTarget) {
        yVelocity += JUMP_ACCELERATION; // Downwards
    }

    calculatePlayersHitboxes(player);


    // Collision checking
    {
        if (PLAYER->hitbox.y > FLOOR_DEATH_HEIGHT) {
            STATE->isPlayerDead = true;
            STATE->isPaused = true;
            return;
        }

        Entity *enemy = ENTITIES;
        do {

            if (enemy->components & IsEnemy) {

                // Enemy hit player
                if (CheckCollisionRecs(enemy->hitbox, playersUpperbody)) {
                    STATE->isPlayerDead = true;
                    STATE->isPaused = true;
                    break;
                }

                // Player hit enemy
                if (CheckCollisionRecs(enemy->hitbox, playersLowebody)) {
                    ENTITIES = DestroyEntity(enemy); // TODO: How does this break the loop?
                    break;
                }
            }

            enemy = enemy->next;
        } while (enemy != ENTITIES);
    }
}

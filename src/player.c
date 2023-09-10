#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"
#include "entity.h"
#include "global.h"


#define PLAYER_SPRITE_SCALE 2

// sprite is 32x64 pixels
#define PLAYER_WIDTH (float)(PLAYER_SPRITE_SCALE * 32)
#define PLAYER_HEIGHT (float)(PLAYER_SPRITE_SCALE * 64)

#define PLAYERS_UPPERBODY_PROPORTION 0.75f // What % of the player's height is upperbody, for hitboxes

#define PLAYER_SPEED_DEFAULT 4.0f
#define PLAYER_SPEED_FAST 8.0f

#define PLAYER_JUMP_DURATION 0.75f // In seconds
#define PLAYER_JUMP_HEIGHT PLAYER_HEIGHT * 1.0
#define PLAYER_END_JUMP_DISTANCE_GROUND 1 // Distance from the ground to end the jump when descending 

#define JUMP_START_VELOCITY 10.0f
#define JUMP_ACCELERATION 0.4f
#define Y_VELOCITY_TARGET_TOLERANCE 1


Rectangle playersUpperbody, playersLowebody;

float yVelocity = 0;
float yVelocityTarget = 0;


void calculatePlayersHitboxes(Entity *player) {
    playersUpperbody = (Rectangle){
        player->hitbox.x,       player->hitbox.y,
        player->hitbox.width,   player->hitbox.height * PLAYERS_UPPERBODY_PROPORTION
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

    calculatePlayersHitboxes(newPlayer);

    AddToEntityList(listItem, newPlayer);

    return newPlayer;
}

void MovePlayer(Entity *player, PlayerMovementType type, PlayerMovementDirection direction) {
    float amount = PLAYER_SPEED_DEFAULT;
    if (type == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    switch (direction) {
        case PLAYER_MOVEMENT_LEFT:
            player->hitbox.x -= amount;

            if (CAMERA->hitbox.x > amount && ((player->hitbox.x - CAMERA->hitbox.x) < SCREEN_WIDTH/3))
                CAMERA->hitbox.x -= amount;

            break;
        case PLAYER_MOVEMENT_RIGHT:
            player->hitbox.x += amount;

            if (player->hitbox.x > amount + SCREEN_WIDTH/2)
                CAMERA->hitbox.x += amount;

            break;
    }

    calculatePlayersHitboxes(player);
}

void PlayerStartJump(Entity *player) {

    if (IsOnTheGround(player) >= 0) {
        yVelocity = JUMP_START_VELOCITY;
        yVelocityTarget = 0.0f;

        player->hitbox.y -= 1 * yVelocity; // Take player off ground. TODO shouldn't need to happen; jarring
    }
}

void PlayerTick(Entity *player) {

    // debug
    char ySpeedTxt[100];
    sprintf(ySpeedTxt, "yVelocity: %f   y: %f", yVelocity, player->hitbox.y);
    DrawText(ySpeedTxt, 10, 40, 20, WHITE);

    bool yVelocityWithinTarget = abs(yVelocity - yVelocityTarget) < Y_VELOCITY_TARGET_TOLERANCE;

    float groundY = IsOnTheGround(player); 
    if (groundY >= 0) {

        // debug
        DrawText("On the ground!", 10, 60, 20, WHITE);

        if (yVelocityWithinTarget) {
            player->hitbox.y = groundY - player->hitbox.height;
            yVelocity = 0;
            yVelocityTarget = 0;
        }
    } else {

        player->hitbox.y -= yVelocity;

        if (yVelocityWithinTarget) {
            // Starts falling down
            yVelocityTarget = -JUMP_START_VELOCITY;
        }
    }

    // Accelerates jump's vertical movement
    if (yVelocity > yVelocityTarget) {
        yVelocity -= JUMP_ACCELERATION; // Upwards
    } else if (yVelocity < yVelocityTarget) {
        yVelocity += JUMP_ACCELERATION; // Downwards
    }

    calculatePlayersHitboxes(player);
}

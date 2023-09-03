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

#define PLAYER_FALL_SPEED 8.0f

Rectangle playersUpperbody, playersLowebody;

double jumpStartTimestamp = -1; // Second in which current jump started; -1 means not jumping
float jumpStartPlayerY = -1; // Player's Y when the current jump started
bool isPlayerDescending = false; // If the player is currently descending from a jump


void calculatePlayersHitboxes(Entity *player) {
    playersUpperbody = (Rectangle){
        player->hitbox.x,       player->hitbox.y,
        player->hitbox.width,   player->hitbox.height * PLAYERS_UPPERBODY_PROPORTION
    };

    playersLowebody = (Rectangle){
        player->hitbox.x,       player->hitbox.y + playersUpperbody.height,
        player->hitbox.width,   player->hitbox.height * (1 - PLAYERS_UPPERBODY_PROPORTION)
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
        case PLAYER_MOVEMENT_LEFT: player->hitbox.x -= amount; break;
        case PLAYER_MOVEMENT_RIGHT: player->hitbox.x += amount; break;
    }

    calculatePlayersHitboxes(player);
}

void PlayerStartJump(Entity *player) {
    if (jumpStartTimestamp == -1 && IsOnTheGround(player)) {
        jumpStartTimestamp = GetTime();
        jumpStartPlayerY = player->hitbox.y;
    }
}

void PlayerTick(Entity *player) {

    if (jumpStartTimestamp != -1) {     // Player jumping

        double jumpTimeDelta = GetTime() - jumpStartTimestamp;
        double jumpCurrentFactor = (jumpTimeDelta/PLAYER_JUMP_DURATION) * M_PI;
        double jumpCurrentDelta = sin(jumpCurrentFactor) * PLAYER_JUMP_HEIGHT;
        float newPlayerY = jumpStartPlayerY - jumpCurrentDelta;
        if (newPlayerY > player->hitbox.y) isPlayerDescending = true;
        player->hitbox.y = newPlayerY;

        if (newPlayerY > jumpStartPlayerY) {
        //if (isPlayerDescending && abs((player->hitbox.y + player->hitbox.height) - FLOOR_HEIGHT) < PLAYER_END_JUMP_DISTANCE_GROUND) { 
            // End jump
            jumpStartTimestamp = -1;
            jumpStartPlayerY = -1;
            isPlayerDescending = false;
            player->hitbox.y = FLOOR_HEIGHT - player->hitbox.height;
            return;
        }
    }

    else if (!IsOnTheGround(player)) {
        player->hitbox.y += PLAYER_FALL_SPEED;
    }

    calculatePlayersHitboxes(player);
}

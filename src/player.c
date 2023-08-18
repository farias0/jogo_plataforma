#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "player.h"
#include "global.h"


#define PLAYER_SPRITE_SCALE 2

// sprite is 32x64 pixels
#define PLAYER_WIDTH (float)(PLAYER_SPRITE_SCALE * 32)
#define PLAYER_HEIGHT (float)(PLAYER_SPRITE_SCALE * 64)

#define PLAYER_SPEED_DEFAULT 4.0f
#define PLAYER_SPEED_FAST 8.0f

#define PLAYER_JUMP_DURATION 1.5f // In seconds
#define PLAYER_JUMP_HEIGHT PLAYER_HEIGHT * 1.5
#define PLAYER_END_JUMP_DISTANCE_GROUND 1 // Distance from the ground to end the jump when descending 


static Texture2D sprite;

Rectangle playerHitbox = { 0.0f, 0.0f, PLAYER_WIDTH, PLAYER_HEIGHT };

double jumpStartTimestamp = -1; // Second in which current jump started; -1 means not jumping
float jumpStartPlayerY = -1; // Player's Y when the current jump started
bool isPlayerDescending = false; // If the player is currently descending from a jump


void InitializePlayer() {
    sprite = LoadTexture("../assets/player_default_1.png");
}

void SetPlayerPosition(Vector2 pos) {
    playerHitbox.x = pos.x;
    playerHitbox.y = pos.y;
}

void MovePlayer(PlayerMovementType type, PlayerMovementDirection direction) {
    float amount = PLAYER_SPEED_DEFAULT;
    if (type == PLAYER_MOVEMENT_RUNNING) amount = PLAYER_SPEED_FAST;

    switch (direction) {
        case PLAYER_MOVEMENT_LEFT: playerHitbox.x -= amount; break;
        case PLAYER_MOVEMENT_RIGHT: playerHitbox.x += amount; break;
    }
}

void PlayerStartJump() {
    if (jumpStartTimestamp == -1) {
        jumpStartTimestamp = GetTime();
        jumpStartPlayerY = playerHitbox.y;
    }
}

void PlayerTick() {

    if (jumpStartTimestamp != -1) {     // Player jumping

        double jumpTimeDelta = GetTime() - jumpStartTimestamp;
        double jumpCurrentFactor = (jumpTimeDelta/PLAYER_JUMP_DURATION) * M_PI;
        double jumpCurrentDelta = sin(jumpCurrentFactor) * PLAYER_JUMP_HEIGHT;
        float newPlayerY = jumpStartPlayerY - jumpCurrentDelta;
        if (newPlayerY > playerHitbox.y) isPlayerDescending = true;
        playerHitbox.y = newPlayerY;

        if (isPlayerDescending && abs((playerHitbox.y + playerHitbox.height) - FLOOR_HEIGHT) < PLAYER_END_JUMP_DISTANCE_GROUND) { 
            // End jump
            jumpStartTimestamp = -1;
            jumpStartPlayerY = -1;
            isPlayerDescending = false;
            playerHitbox.y = FLOOR_HEIGHT - playerHitbox.height;
            return;
        }
    }
}

void DrawPlayer() {
    DrawTextureEx(sprite, (Vector2){playerHitbox.x, playerHitbox.y}, 0, PLAYER_SPRITE_SCALE, WHITE);
}